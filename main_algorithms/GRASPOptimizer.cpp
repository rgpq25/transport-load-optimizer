#include "GRASPOptimizer.h"
#include "dispatchUtils.h"
#include "bin3D.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
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

    // [TRACE] Archivos de la traza
    bool trace = !traceDir.empty();
    ofstream traceVehicles, traceIterations, traceLayers, traceLocalSearch;
    int vehicleStep = 0;
    double traceBestGlobal = -numeric_limits<double>::infinity();
    if (trace) {
        TraceUtils::openTraceFile(traceVehicles, traceDir, "grasp_vehicle_selection.csv",
            "run_id,step,pending_deliveries,target_volume,vehicle_id,vehicle_volume,difference,selected");
        TraceUtils::openTraceFile(traceIterations, traceDir, "grasp_iterations.csv",
            "run_id,vehicle_id,iteration,phase,deliveries,layers,pattern_weight,max_weight,fitness," +
            TraceUtils::breakdownHeader() + ",best_vehicle_fitness_before,new_best,best_fitness_global,duration_s");
        TraceUtils::openTraceFile(traceLayers, traceDir, "grasp_layers.csv",
            "run_id,vehicle_id,iteration,phase,step,space_x,space_y,space_z,space_length,space_width,space_height,"
            "spaces_left,candidates,alpha,rcl_size,event,delivery_id,blocks,orientation,layer_length,layer_width,"
            "layer_height,layer_volume,layer_weight,pattern_weight,new_spaces");
        TraceUtils::openTraceFile(traceLocalSearch, traceDir, "grasp_local_search.csv",
            "run_id,vehicle_id,iteration,total_blocks,k_percent,blocks_to_remove,removed_deliveries,removed_blocks");
    }

    // [TRACE] Peso real de un patrón (suma de los bloques de sus capas)
    auto patternWeight = [](const VehiclePattern& p) {
        double w = 0.0;
        for (const auto& layer : p.layers) for (auto* b : layer.blocks) w += b->getWeight();
        return w;
    };

    // [TRACE] Una fila por cada paso de construcción (capa colocada o fin de la construcción)
    auto logLayer = [&](int vehicleId, int iteration, const string& phase, int step, const MaximalSpace& e,
                        size_t spacesLeft, int candidates, double alpha, int rclSize, const string& event,
                        const LayerCandidate* sp, const VehiclePattern& pattern, int newSpaces) {
        traceLayers << runId << "," << vehicleId << "," << iteration << "," << phase << "," << step << ","
                    << e.originX << "," << e.originY << "," << e.originZ << ","
                    << e.length << "," << e.width << "," << e.height << ","
                    << spacesLeft << "," << candidates << ",";
        if (alpha >= 0) traceLayers << alpha;
        traceLayers << ",";
        if (rclSize >= 0) traceLayers << rclSize;
        traceLayers << "," << event << ",";
        if (sp != nullptr) {
            double w = 0.0;
            string blockIds;
            for (auto* b : sp->blocks) {
                w += b->getWeight();
                blockIds += (blockIds.empty() ? "" : " ") + to_string(b->getId());
            }
            traceLayers << sp->delivery->getId() << "," << blockIds << ","
                        << (sp->orientations.empty() ? 0 : sp->orientations[0]) << ","
                        << sp->footprintLength << "," << sp->footprintWidth << "," << sp->layerHeight << ","
                        << sp->volume << "," << w << ",";
        } else {
            traceLayers << ",,,,,,,,";
        }
        traceLayers << patternWeight(pattern) << ",";
        if (newSpaces >= 0) traceLayers << newSpaces;
        traceLayers << "\n";
    };

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

        // [TRACE] Selección de la unidad de transporte
        if (trace) {
            vehicleStep++;
            for (size_t i = 0; i < Vveh.size(); ++i) {
                traceVehicles << runId << "," << vehicleStep << "," << LPveh.size() << "," << targetVolume << ","
                              << Vveh[i]->getId() << "," << Vveh[i]->getVolume() << ","
                              << abs(Vveh[i]->getVolume() - targetVolume) << ","
                              << ((int)i == veIndex ? 1 : 0) << "\n";
            }
        }

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

            string tracePhase = localsearch ? "local_search" : "construction";  // [TRACE]
            int traceStep = 0;                                                  // [TRACE]

            // 4.2 Construcción iterativa de capas
            while (!LPcopy.empty() && !E.empty()) {
                int eIdx = std::rand() % static_cast<int>(E.size());
                MaximalSpace e = E[eIdx];
                E.erase(E.begin() + eIdx);
                traceStep++;

                // Generar capas candidatas para cada delivery pendiente
                vector<LayerCandidate> layers;
                for (auto* d : LPcopy) {
                    auto cand = generateLayers(e, d);
                    layers.insert(layers.end(), cand.begin(), cand.end());
                }
                if (layers.empty()) {
                    if (trace) logLayer(ve->getId(), iterationCount, tracePhase, traceStep, e, E.size(), 0, -1, -1,
                                        "no_candidates_stop", nullptr, CurrentPattern, -1);
                    break;
                }

                std::random_shuffle(layers.begin(), layers.end());

                // Selección de la capa a colocar
                LayerCandidate sp;
                double traceAlpha = -1.0;   // [TRACE]
                int traceRclSize = -1;      // [TRACE]
                if (!localsearch) {
                    double alpha = alphaTuner.pick();
                    auto rcl = buildRCL(layers, alpha);
                    traceAlpha = alpha;
                    
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
                    if (rcl.empty()) {
                        if (trace) logLayer(ve->getId(), iterationCount, tracePhase, traceStep, e, E.size(),
                                            (int)layers.size(), traceAlpha, 0, "empty_rcl_stop", nullptr, CurrentPattern, -1);
                        break;
                    }
                    traceRclSize = (int)rcl.size();

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
                    if (layers.empty()) {
                        if (trace) logLayer(ve->getId(), iterationCount, tracePhase, traceStep, e, E.size(), 0, -1, -1,
                                            "no_candidates_stop", nullptr, CurrentPattern, -1);
                        break;
                    }

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

                if (trace) logLayer(ve->getId(), iterationCount, tracePhase, traceStep, e, E.size() - newSpaces.size(),
                                    (int)layers.size(), traceAlpha, traceRclSize,
                                    localsearch ? "best_fit_layer" : "rcl_layer", &sp, CurrentPattern, (int)newSpaces.size());

                // Eliminar el delivery completo de LPcopy
                LPcopy.erase(
                    remove(LPcopy.begin(), LPcopy.end(), sp.delivery),
                    LPcopy.end()
                );
            }

            // 4.3 Fase de mejora local: remover K% de bloques
            if (localsearch && !CurrentPattern.layers.empty()) {
                // Eliminar K% y obtener las capas quitadas
                int traceTotalBlocks = 0;   // [TRACE]
                for (auto& layer : CurrentPattern.layers) traceTotalBlocks += (int)layer.blocks.size();
                auto removed = removeKPercent(CurrentPattern);

                // [TRACE] Capas removidas por la mejora local
                if (trace) {
                    string removedDeliveries;
                    int removedBlocks = 0;
                    for (auto& layer : removed) {
                        removedDeliveries += (removedDeliveries.empty() ? "" : " ") + to_string(layer.delivery->getId());
                        removedBlocks += (int)layer.blocks.size();
                    }
                    traceLocalSearch << runId << "," << ve->getId() << "," << iterationCount << ","
                                     << traceTotalBlocks << "," << K << ","
                                     << (int)std::ceil(traceTotalBlocks * (K / 100.0)) << ","
                                     << removedDeliveries << "," << removedBlocks << "\n";
                }

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
                    traceStep++;

                    vector<LayerCandidate> layers;
                    for (auto* d : LPcopy) {
                        auto cand = generateLayers(e, d);
                        layers.insert(layers.end(), cand.begin(), cand.end());
                    }
                    if (layers.empty()) {
                        if (trace) logLayer(ve->getId(), iterationCount, "reinsertion", traceStep, e, E.size(), 0, -1, -1,
                                            "no_candidates_stop", nullptr, CurrentPattern, -1);
                        break;
                    }

                    // Filtro por peso
                    layers.erase(std::remove_if(layers.begin(), layers.end(),
                        [&](const LayerCandidate& cand) {
                            double w = 0.0;
                            for (auto* b : cand.blocks) w += b->getWeight();
                            return (CurrentPattern.currentWeight + w > ve->getMaxWeight() &&
                                    CurrentPattern.currentWeight + w < ve->getMinWeight());
                        }),
                        layers.end());
                    if (layers.empty()) {
                        if (trace) logLayer(ve->getId(), iterationCount, "reinsertion", traceStep, e, E.size(), 0, -1, -1,
                                            "no_candidates_stop", nullptr, CurrentPattern, -1);
                        break;
                    }

                    LayerCandidate sp = selectBestFit(layers);

                    double spWeight = 0.0;
                    for (auto* b : sp.blocks) spWeight += b->getWeight();
                    CurrentPattern.currentWeight += spWeight;
                    CurrentPattern.layers.push_back(sp);

                    vector<MaximalSpace> newSpaces;
                    locateLayer(sp, e, newSpaces);
                    E.insert(E.end(), newSpaces.begin(), newSpaces.end());

                    if (trace) logLayer(ve->getId(), iterationCount, "reinsertion", traceStep, e, E.size() - newSpaces.size(),
                                        (int)layers.size(), -1, -1, "best_fit_layer", &sp, CurrentPattern, (int)newSpaces.size());

                    LPcopy.erase(remove(LPcopy.begin(), LPcopy.end(), sp.delivery), LPcopy.end());
                }
            }

            // 4.4 Actualizar mejor patrón si CurrentPattern es mejor
            double fitCurr = evaluateSolutionFitness({CurrentPattern});
            double fitBest = evaluateSolutionFitness({BestVehiclePattern});
            if (fitCurr > fitBest) {
                BestVehiclePattern = CurrentPattern;
            }

            // [TRACE] Resumen de la iteración
            if (trace) {
                FitnessBreakdown bd;
                evaluateSolutionFitness({CurrentPattern}, &bd);
                string deliveryIds;
                for (auto& layer : CurrentPattern.layers) {
                    deliveryIds += (deliveryIds.empty() ? "" : " ") + to_string(layer.delivery->getId());
                }
                traceIterations << runId << "," << ve->getId() << "," << iterationCount << "," << tracePhase << ","
                                << deliveryIds << "," << CurrentPattern.layers.size() << ","
                                << patternWeight(CurrentPattern) << "," << ve->getMaxWeight() << ","
                                << fitCurr << "," << TraceUtils::breakdownToCSV(bd) << ","
                                << fitBest << "," << (fitCurr > fitBest ? 1 : 0) << ",";
                traceBestGlobal = max(traceBestGlobal, fitCurr);
                traceIterations << traceBestGlobal << ",";   // la duración se completa al final de la iteración
            }

            // Alternar entre fase constructiva y mejora local
            localsearch = !localsearch;
            
            auto end = chrono::high_resolution_clock::now();  // Tiempo fin
            chrono::duration<double> duration = end - start;
            if (trace) traceIterations << duration.count() << "\n";  // [TRACE]

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

    if (trace) {
        traceVehicles.close();
        traceIterations.close();
        traceLayers.close();
        traceLocalSearch.close();
    }

    return LoadingTruck;
}

void GRASPOptimizer::setTrace(const string& traceDir, const string& runId) {
    this->traceDir = traceDir;
    this->runId = runId;
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

double GRASPOptimizer::evaluateSolutionFitness(const vector<VehiclePattern>& patterns, FitnessBreakdown* breakdown) const {
    unordered_map<int, vector<Block*>> vehicleBlocks;
    unordered_map<int, double> vehicleVolumeUsed;
    unordered_map<int, double> vehicleWeightUsed;
    unordered_set<int> usedVehicles;

    unordered_set<int> seenDeliveries;
    
    int numDeliveriesAssigned = 0;
    int totalPriority = 0;
    int attendedPriority    = 0;
    
    // 1) Acumular prioridad total
    for (Delivery* d : LP) {
        totalPriority += d->getPriority();
    }

    for (const auto& pat : patterns) {
        int vIdx = int(find(V.begin(), V.end(), pat.vehicle) - V.begin());
        usedVehicles.insert(vIdx);
        
        TransportUnit* vehicle = V[vIdx];
        Bin3D bin(vehicle->getLength(), vehicle->getWidth(), vehicle->getHeight());

        for (const auto& layer : pat.layers) {
            Delivery* d = layer.delivery;
            int did = d->getId();
            
            // Sólo contar cada delivery una vez
            if (seenDeliveries.insert(did).second) {
                numDeliveriesAssigned++;
                attendedPriority += d->getPriority();
            }
            
            // Registrar bloques, volumen y peso ( verificar si es viable en 3D
            for (size_t i = 0; i < layer.blocks.size(); ++i) {
                Block* b = layer.blocks[i];
                
                vehicleBlocks[vIdx].push_back(b);
                vehicleVolumeUsed[vIdx] += b->getVolume();
                vehicleWeightUsed[vIdx] += b->getWeight();
                
                if (!bin.tryPlaceBlock(b, layer.orientations[i])) {
                    if (breakdown != nullptr) breakdown->feasible3D = false;
                    return -9999999;
                }
            }
        }
    }

    // 5) Calcular score y penalizaciones
    double totalUtilizationScore = 0.0;
    double overcapacityPenalty = 0.0;

    for (int vIdx : usedVehicles) {
        TransportUnit* vehicle = V[vIdx];
        
        double maxVolume = vehicle->getVolume();
        double maxWeight = vehicle->getMaxWeight();
        double minWeight = vehicle->getMinWeight();

        double usedVolume = vehicleVolumeUsed[vIdx];
        double usedWeight = vehicleWeightUsed[vIdx];

        totalUtilizationScore += usedVolume / maxVolume;

        if (usedWeight > maxWeight) {
            overcapacityPenalty += (usedWeight - maxWeight);  // Penalize overweight
        } else if (usedWeight < minWeight) {
            overcapacityPenalty += (minWeight - usedWeight);  // Penalize minweight
        }
    }

    double avgUtilization   = numDeliveriesAssigned == 0 ? 0.0 : totalUtilizationScore * 100 / numDeliveriesAssigned;
    double fulfillmentRatio = LP.empty() ? 0.0 : (double)numDeliveriesAssigned * 100 / LP.size();
    double priorityCoverage = totalPriority == 0 ? 0.0 : (double)attendedPriority * 100 / totalPriority;

    if (breakdown != nullptr) {
        breakdown->dispatchesCount = patterns.size();
        breakdown->avgUtilization = avgUtilization;
        breakdown->fulfillmentRatio = fulfillmentRatio;
        breakdown->priorityCoverage = priorityCoverage;
        breakdown->overcapacityPenalty = overcapacityPenalty;
        ostringstream ss;
        for (int vIdx : usedVehicles) {
            if (!ss.str().empty()) ss << ";";
            ss << "UT" << V[vIdx]->getId() << "=" << vehicleWeightUsed.at(vIdx) << "/" << V[vIdx]->getMaxWeight();
        }
        breakdown->weightByVehicle = ss.str();
    }

    return DispatchUtils::getObjectiveFunction(
        patterns.size(), 
        avgUtilization, 
        fulfillmentRatio, 
        priorityCoverage, 
        overcapacityPenalty
    );
}