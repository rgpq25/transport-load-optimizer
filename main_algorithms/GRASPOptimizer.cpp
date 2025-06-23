#include "GRASPOptimizer.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include "bin3D.h"
#include <fstream>
#include <chrono>

GRASPOptimizer::GRASPOptimizer(
    const vector<Delivery*>& deliveries,
    const vector<Block*>& blocks,
    const vector<TransportUnit*>& vehicles,
    Route* r,
    const TimeSlot& s,
    const string& dt,
    int graspIterations,
    double Kpercent,
    const vector<double>& alphaSet
) : LP(deliveries), allBlocks(blocks), V(vehicles), route(r), slot(s), date(dt),
    maxIter(graspIterations * 2), K(Kpercent), alphaTuner(alphaSet) {}

vector<VehiclePattern> GRASPOptimizer::run(bool debug) {
    vector<VehiclePattern> LoadingTruck;
    
    // Copias de LP y V para no modificar los orígenes
    vector<Delivery*> LPveh = LP;
    vector<TransportUnit*> Vveh = V;
    
    // Para impresion en debug
    ofstream debugFile;
    if (debug) {
        debugFile.open("../output/grasp_debug_log.csv");
        debugFile << "Iteración,Unidad de Transporte,Fitness patrón generado,Duracion (s),Mejor fitness global\n";
    }
    double bestFitnessGlobal = -numeric_limits<double>::infinity();
    int iterationCount = 1;

    // Mientras haya entregas y vehículos disponibles
    while (!LPveh.empty() && !Vveh.empty()) {
        
        // 1. Selección de vehículo óptimo por cercanía de volumen
        double targetVolume = 0.0;
        for (Delivery* d : LPveh) {
            for (Block* b : d->getBlocksToDeliver()) {
                targetVolume += b->getVolume();
            }
        }

        int veIndex = 0;
        double bestDiff = numeric_limits<double>::max();
        for (size_t i = 0; i < Vveh.size(); ++i) {
            // Volumen interno del vehículo
            double vehicleVolume = Vveh[i]->getVolume();
            double diff = abs(vehicleVolume - targetVolume);
            if (diff < bestDiff) {
                bestDiff = diff;
                veIndex = static_cast<int>(i);
            }
        }
        TransportUnit* ve = Vveh[veIndex];

        // 2. Inicialización de BestVehiclePattern y flag de fase
        VehiclePattern BestVehiclePattern;
        BestVehiclePattern.vehicle = ve;
        bool localsearch = false;

        // 3. Generar el espacio inicial único para este vehículo
        vector<MaximalSpace> E = generateInitialSpaces(ve);

        // 4. Bucle GRASP (constructiva + mejora local)
        for (int iter = 0; iter < maxIter; ++iter) {
            auto start = chrono::high_resolution_clock::now();  // Tiempo inicio
            
            // Si estamos en la fase constructiva, reiniciamos los espacios
            if (!localsearch) {
                E = generateInitialSpaces(ve);
            }

            // Copia de las entregas pendientes para este vehículo
            vector<Delivery*> LPcopy = LPveh;
            VehiclePattern CurrentPattern;
            CurrentPattern.vehicle = ve;

            // 4.2 Construcción iterativa de capas
            while (!LPcopy.empty() && !E.empty()) {
                int eIdx = std::rand() % static_cast<int>(E.size());
                MaximalSpace e = E[eIdx];
                E.erase(E.begin() + eIdx);

                // Generar capas candidatas para cada delivery pendiente
                vector<LayerCandidate> layers;
                for (auto* d : LPcopy) {
                    auto cand = generateLayers(e, d);
                    layers.insert(layers.end(), cand.begin(), cand.end());
                }
                if (layers.empty()) break;
                
                std::random_shuffle(layers.begin(), layers.end());

                // Selección de la capa a colocar
                LayerCandidate sp;
                if (!localsearch) {
                    double alpha = alphaTuner.pick();
                    auto rcl = buildRCL(layers, alpha);
                    
                    // Antes de escoger, filtrar en la RCL aquellas que superarían el peso
                    rcl.erase(std::remove_if(rcl.begin(), rcl.end(),
                        [&](const LayerCandidate& cand) {
                            double layerWeight = 0.0;
                            for (auto* b : cand.blocks) layerWeight += b->getWeight();
                            return (CurrentPattern.currentWeight + layerWeight > ve->getMaxWeight() &&
                                    CurrentPattern.currentWeight + layerWeight < ve->getMinWeight()
                            );
                        }),
                        rcl.end());
                    if (rcl.empty()) break;
                        
                    int idx = rand() % static_cast<int>(rcl.size());
                    sp = rcl[idx];
                } else {
                    // Quitamos candidatas que violen peso antes de bestFit
                    layers.erase(std::remove_if(layers.begin(), layers.end(),
                        [&](const LayerCandidate& cand) {
                            double layerWeight = 0.0;
                            for (auto* b : cand.blocks) layerWeight += b->getWeight();
                            return (CurrentPattern.currentWeight + layerWeight > ve->getMaxWeight() &&
                                    CurrentPattern.currentWeight + layerWeight < ve->getMinWeight()
                            );
                        }),
                        layers.end());
                    if (layers.empty()) break;
                        
                    sp = selectBestFit(layers);
                }
                
                // Al confirmar sp, actualizamos el peso
                double spWeight = 0.0;
                for (auto* b : sp.blocks) spWeight += b->getWeight();
                CurrentPattern.currentWeight += spWeight;

                // Ubicar la capa y subdividir el espacio
                CurrentPattern.layers.push_back(sp);
                vector<MaximalSpace> newSpaces;
                locateLayer(sp, e, newSpaces);
                E.insert(E.end(), newSpaces.begin(), newSpaces.end());

                // Eliminar el delivery completo de LPcopy
                LPcopy.erase(
                    remove(LPcopy.begin(), LPcopy.end(), sp.delivery),
                    LPcopy.end()
                );
            }

            // 4.3 Fase de mejora local: remover K% de bloques
            if (localsearch && !CurrentPattern.layers.empty()) {
                // Eliminar K% y obtener las capas quitadas
                auto removed = removeKPercent(CurrentPattern);
                
                // Reintegrar sus deliveries a LPcopy
                for (auto& layer : removed) {
                    LPcopy.push_back(layer.delivery);
                }
                
                // Regenerar espacios para reintentarlo
                E = generateInitialSpaces(ve);
                
                // Reinsertar las capas con las entregas eliminadas
                while (!LPcopy.empty() && !E.empty()) {
                    int eIdx = std::rand() % static_cast<int>(E.size());
                    MaximalSpace e = E[eIdx];
                    E.erase(E.begin() + eIdx);

                    vector<LayerCandidate> layers;
                    for (auto* d : LPcopy) {
                        auto cand = generateLayers(e, d);
                        layers.insert(layers.end(), cand.begin(), cand.end());
                    }
                    if (layers.empty()) break;

                    // Filtro por peso
                    layers.erase(std::remove_if(layers.begin(), layers.end(),
                        [&](const LayerCandidate& cand) {
                            double w = 0.0;
                            for (auto* b : cand.blocks) w += b->getWeight();
                            return (CurrentPattern.currentWeight + w > ve->getMaxWeight() &&
                                    CurrentPattern.currentWeight + w < ve->getMinWeight());
                        }),
                        layers.end());
                    if (layers.empty()) break;

                    LayerCandidate sp = selectBestFit(layers);

                    double spWeight = 0.0;
                    for (auto* b : sp.blocks) spWeight += b->getWeight();
                    CurrentPattern.currentWeight += spWeight;
                    CurrentPattern.layers.push_back(sp);

                    vector<MaximalSpace> newSpaces;
                    locateLayer(sp, e, newSpaces);
                    E.insert(E.end(), newSpaces.begin(), newSpaces.end());

                    LPcopy.erase(remove(LPcopy.begin(), LPcopy.end(), sp.delivery), LPcopy.end());
                }
            }

            // 4.4 Actualizar mejor patrón si CurrentPattern es mejor
            double fitCurr = evaluateSolutionFitness({CurrentPattern});
            double fitBest = evaluateSolutionFitness({BestVehiclePattern});
            if (fitCurr > fitBest) {
                BestVehiclePattern = CurrentPattern;
            }

            // Alternar entre fase constructiva y mejora local
            localsearch = !localsearch;
            
            auto end = chrono::high_resolution_clock::now();  // Tiempo fin
            chrono::duration<double> duration = end - start;
            
            // === Registro de datos
            if (debug == true) {
                if (fitCurr > bestFitnessGlobal) {
                    bestFitnessGlobal = fitCurr;
                }
                
                debugFile 
                    << iterationCount << ","
                    << ve->getId() << ","
                    << fitCurr << ","
                    << duration.count() << ","
                    << bestFitnessGlobal << "\n";
            }
            
            iterationCount += 1;
        }

        // 5. Finalización para el vehículo ve
        LoadingTruck.push_back(BestVehiclePattern);

        // Marcar entregas atendidas y eliminarlas de LPveh
        for (auto& layer : BestVehiclePattern.layers) {
            LPveh.erase(
                remove(LPveh.begin(), LPveh.end(), layer.delivery),
                LPveh.end()
            );
        }
        
        // Retirar el vehículo de la lista
        Vveh.erase(Vveh.begin() + veIndex);
    }
    
    if (debug) {
        debugFile.close();
    }

    return LoadingTruck;
}

vector<MaximalSpace> GRASPOptimizer::generateInitialSpaces(TransportUnit* truck) {
    // Inicializa el espacio libre con las dimensiones completas del vehículo
    MaximalSpace space (
        0.0,
        0.0,
        0.0,
        truck->getLength(),
        truck->getWidth(),
        truck->getHeight()
    );

    return vector<MaximalSpace>{ space };
}

vector<LayerCandidate> GRASPOptimizer::generateLayers(const MaximalSpace& e, Delivery* d) {
    vector<Block*>& blocks = const_cast<vector<Block*>&>(d->getBlocksToDeliver());
    vector<LayerCandidate> candidates;
    if (blocks.empty()) return candidates;

    // 1) Orientación 0: longitud en X = suma de lengths, ancho en Y = max widths
    double sumLen = 0, maxWid = 0, maxHei = 0;
    for (Block* b : blocks) {
        sumLen += b->getLength();
        maxWid = max(maxWid, b->getWidth());
        maxHei = max(maxHei, b->getHeight());
    }
    if (sumLen <= e.length && maxWid <= e.width) {
        vector<int> orients(blocks.size(), 0);
        candidates.emplace_back(d, blocks, orients, sumLen, maxWid, maxHei);
    }

    // 2) Orientación 1: longitud en X = suma de widths, ancho en Y = max lengths
    double sumLen1 = 0, maxWid1 = 0;
    for (Block* b : blocks) {
        sumLen1 += b->getWidth();
        maxWid1 = max(maxWid1, b->getLength());
    }
    if (sumLen1 <= e.length && maxWid1 <= e.width) {
        vector<int> orients(blocks.size(), 1);
        candidates.emplace_back(d, blocks, orients, sumLen1, maxWid1, maxHei);
    }

    return candidates;
}

vector<LayerCandidate> GRASPOptimizer::buildRCL(const vector<LayerCandidate>& layers, double alpha) {
    if (layers.empty()) return {};
    
    vector<LayerCandidate> sorted = layers;
    sort(sorted.begin(), sorted.end(), [](const LayerCandidate& a, const LayerCandidate& b) {
        return a.volume > b.volume;
    });
    size_t rclSize = static_cast<size_t>(ceil(alpha * sorted.size()));
    rclSize = max<size_t>(1, rclSize);
    rclSize = min(rclSize, sorted.size());

    vector<LayerCandidate> rcl;
    rcl.reserve(rclSize);
    for (size_t i = 0; i < rclSize; ++i) {
        rcl.push_back(sorted[i]);
    }
    return rcl;
}

LayerCandidate GRASPOptimizer::selectBestFit(const vector<LayerCandidate>& layers) {
    if (layers.empty()) {
        throw runtime_error("selectBestFit called with empty layer list");
    }

    // Encontrar el candidato con el mayor volumen
    const LayerCandidate* best = &layers[0];
    for (const auto& candidate : layers) {
        if (candidate.volume > best->volume) {
            best = &candidate;
        }
    }
    
    return *best;
}

void GRASPOptimizer::locateLayer(const LayerCandidate& sp, MaximalSpace& e, vector<MaximalSpace>& spacesOut) {
    // Asumimos que la capa se coloca en la esquina inferior (e.originX, e.originY, e.originZ)
    double ox = e.originX;
    double oy = e.originY;
    double oz = e.originZ;
    double L = e.length;
    double W = e.width;
    double H = e.height;

    double layerL = sp.footprintLength;
    double layerW = sp.footprintWidth;
    double layerH = sp.layerHeight;

    // Espacio residual a la derecha de la capa (eje X)
    double remLx = L - layerL;
    if (remLx > 1e-6) {
        spacesOut.emplace_back(
            ox + layerL,  // nueva originX
            oy,           // misma originY
            oz,           // misma originZ
            remLx,        // length restante
            layerW,       // width = ancho de la capa
            layerH        // height = grosor de la capa
        );
    }

    // Espacio residual al frente de la capa (eje Y)
    double remWy = W - layerW;
    if (remWy > 1e-6) {
        spacesOut.emplace_back(
            ox,           // misma originX
            oy + layerW,  // nueva originY
            oz,           // misma originZ
            L,            // length = longitud total del espacio original
            remWy,        // width restante
            layerH        // height = grosor de la capa
        );
    }
}

vector<LayerCandidate> GRASPOptimizer::removeKPercent(VehiclePattern& pattern) {
    // 1. Contar bloques
    int totalBlocks = 0;
    for (auto& layer : pattern.layers) {
        totalBlocks += (int)layer.blocks.size();
    }

    int toRemove = (int)std::ceil(totalBlocks * (K / 100.0));
    std::vector<LayerCandidate> removedLayers;
    if (toRemove <= 0 || pattern.layers.empty()) {
        return removedLayers;
    }

    // 2. Determinar cuántas capas quitar
    int removedCount = 0;
    int layersToRemove = 0;
    for (int idx = (int)pattern.layers.size() - 1; idx >= 0; --idx) {
        removedCount += (int)pattern.layers[idx].blocks.size();
        ++layersToRemove;
        if (removedCount >= toRemove) break;
    }

    // 3. Extraer y devolver esas capas
    int startIdx = (int)pattern.layers.size() - layersToRemove;
    removedLayers.assign(
        pattern.layers.begin() + startIdx,
        pattern.layers.end()
    );

    // 4. Borrar del pattern
    pattern.layers.erase(
        pattern.layers.begin() + startIdx,
        pattern.layers.end()
    );

    return removedLayers;
}

double GRASPOptimizer::evaluateSolutionFitness(const vector<VehiclePattern>& patterns) const {
    // 1) Acumular prioridad total
    int totalPriority = 0;
    for (Delivery* d : LP) {
        totalPriority += d->getPriority();
    }

    // 2) Mapear vehículo → bloques, volúmenes y pesos usados
    unordered_map<int, vector<Block*>> vehicleBlocks;
    unordered_map<int, double> vehicleVolumeUsed;
    unordered_map<int, double> vehicleWeightUsed;
    unordered_set<int> usedVehicles;

    // 3) Contar deliveries atendidos (únicos) y prioridad cubierta
    unordered_set<int> seenDeliveries;
    int numDeliveriesAssigned = 0;
    int attendedPriority    = 0;

    for (const auto& pat : patterns) {
        // Encontrar índice del vehículo en V
        int vIdx = int(find(V.begin(), V.end(), pat.vehicle) - V.begin());
        usedVehicles.insert(vIdx);

        for (const auto& layer : pat.layers) {
            Delivery* d = layer.delivery;
            int did = d->getId();
            // Sólo contar cada delivery una vez
            if (seenDeliveries.insert(did).second) {
                numDeliveriesAssigned++;
                attendedPriority += d->getPriority();
            }
            // Registrar bloques, volumen y peso
            for (Block* b : layer.blocks) {
                vehicleBlocks[vIdx].push_back(b);
                double vol = b->getHeight() * b->getWidth() * b->getLength();
                vehicleVolumeUsed[vIdx] += vol;
                vehicleWeightUsed[vIdx] += b->getWeight();
            }
        }
    }

    // 4) Comprobación de factibilidad 3D (igual que en SAGA)
    for (int vIdx : usedVehicles) {
        TransportUnit* vehicle = V[vIdx];
        Bin3D bin(vehicle->getLength(), vehicle->getWidth(), vehicle->getHeight());
        // Asumimos que las orientaciones usadas están guardadas en pat.layers
        for (Block* b : vehicleBlocks[vIdx]) {
            // Aquí deberías recuperar la orientación correspondiente;
            // si no la almacenas, asume una orientación fija o extiéndelo.
            int ori = 0;  
            if (!bin.tryPlaceBlock(b, ori)) {
                return 0.0;  // Empaque inviable
            }
        }
    }

    // 5) Calcular score y penalizaciones
    double totalUtilizationScore = 0.0;
    double overcapacityPenalty   = 0.0;

    for (int vIdx : usedVehicles) {
        TransportUnit* vehicle = V[vIdx];
        double maxVol = vehicle->getLength() * vehicle->getWidth() * vehicle->getHeight();
        double usedVol = vehicleVolumeUsed[vIdx];
        double usedW   = vehicleWeightUsed[vIdx];
        double util   = usedVol / maxVol;
        totalUtilizationScore += util;
        if (usedW > vehicle->getMaxWeight()) {
            overcapacityPenalty += (usedW - vehicle->getMaxWeight()) * 10.0;
        }
    }

    int numVehiclesUsed = (int)usedVehicles.size();
    double avgUtilization   = numVehiclesUsed ? totalUtilizationScore / numVehiclesUsed : 0.0;
    double fulfillmentRatio = LP.empty() ? 0.0 : (double)numDeliveriesAssigned / LP.size();
    double priorityCoverage = totalPriority ? (double)attendedPriority / totalPriority : 0.0;

    // 6) Combinar según los mismos pesos A…E de SAGA
    const double A = 0.5; // penaliza nº camiones
    const double B = 0.5; // recompensa utilización
    const double C = 1.0; // penaliza sobrepeso
    const double D = 1.0; // recompensa ratio de entregas
    const double E = 1.0; // recompensa coverage de prioridad

    double truckScore = V.empty() ? 0.0 : 1.0 - double(numVehiclesUsed) / V.size();
    double fitness =
        A * truckScore +
        B * avgUtilization -
        C * overcapacityPenalty +
        D * fulfillmentRatio +
        E * priorityCoverage;

    return fitness;
}