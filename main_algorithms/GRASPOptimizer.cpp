#include "GRASPOptimizer.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <cstdlib>

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

vector<VehiclePattern> GRASPOptimizer::run() {
    vector<VehiclePattern> LoadingTruck;
    
    // Copias de LP y V para no modificar los orígenes
    vector<Delivery*> LPveh = LP;
    vector<TransportUnit*> Vveh = V;

    // Mientras haya entregas y vehículos disponibles
    while (!LPveh.empty() && !Vveh.empty()) {
        
        // 1. Selección de vehículo óptimo por cercanía de volumen
        // Calcular volumen pendiente (suma de volúmenes de todos los bloques de LPveh)
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
                // --- Selección aleatoria de espacio (en lugar de front) ---
                int eIdx = std::rand() % static_cast<int>(E.size());
                //std::cout << "[DEBUG] E.size=" << E.size()
                //    << "  -> selecting space index eIdx=" << eIdx << std::endl;
                MaximalSpace e = E[eIdx];
                E.erase(E.begin() + eIdx);

                // Generar capas candidatas para cada delivery pendiente
                vector<LayerCandidate> layers;
                for (auto* d : LPcopy) {
                    auto cand = generateLayers(e, d);
                    layers.insert(layers.end(), cand.begin(), cand.end());
                }
                //std::cout << "[DEBUG] Generated layers.size=" << layers.size() << std::endl;
                if (layers.empty()) break;
                
                // --- Aleatorizar orden de capas antes de RCL ---
                std::random_shuffle(layers.begin(), layers.end());
                //std::cout << "[DEBUG] Shuffled layers." << std::endl;

                // Selección de la capa a colocar
                LayerCandidate sp;
                if (!localsearch) {
                    double alpha = alphaTuner.pick();
                    //std::cout << "[DEBUG] pick() -> alpha=" << alpha << std::endl;
                    auto rcl = buildRCL(layers, alpha);
                    //std::cout << "[DEBUG] RCL.size=" << rcl.size() << std::endl;
                    
                    // Antes de escoger, filtrar en la RCL aquellas que superarían el peso
                    rcl.erase(std::remove_if(rcl.begin(), rcl.end(),
                        [&](const LayerCandidate& cand) {
                            // suma de pesos de los bloques de la capa
                            double layerWeight = 0.0;
                            for (auto* b : cand.blocks) layerWeight += b->getWeight();
                            return (CurrentPattern.currentWeight + layerWeight > ve->getMaxWeight() &&
                                    CurrentPattern.currentWeight + layerWeight < ve->getMinWeight()
                            );
                        }),
                        rcl.end());
                    if (rcl.empty()) break;    // no hay capa viable por peso
                    
                    int idx = rand() % static_cast<int>(rcl.size());
                    //std::cout << "[DEBUG] selected idx=" << idx << " from RCL" << std::endl;
                    sp = rcl[idx];
                } else {
                    
                    // similar: quitamos candidatas que violen peso antes de bestFit
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
            }

            // 4.4 Actualizar mejor patrón si CurrentPattern es mejor
            if (CurrentPattern.usedVolume() > BestVehiclePattern.usedVolume()) {
                BestVehiclePattern = CurrentPattern;
            }

            // Alternar entre fase constructiva y mejora local
            localsearch = !localsearch;
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
    vector<LayerCandidate> candidates;
    const auto& blocks = d->getBlocksToDeliver();
    for (Block* b : blocks) {
        double h = b->getHeight();
        double l = b->getLength();
        double w = b->getWidth();
        vector<pair<double,double>> orients = {{l,w}, {w,l}};
        for (int ori = 0; ori < 2; ++ori) {
            double fpLen = orients[ori].first;
            double fpWid = orients[ori].second;
            if (fpLen <= e.length && fpWid <= e.width) {
                LayerCandidate lc(d, {b}, {ori}, fpLen, fpWid, h);
                candidates.push_back(lc);
            }
        }
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

