#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

#include "block.h"
#include "transportUnit.h"
#include "client.h"
#include "input.h"
#include "timeSlot.h"
#include "dispatch.h"

#include "SAGAOptimizer.h"
#include "globalExecutionTracker.h"

#include "GRASPOptimizer.h"
#include "graspPackingState.h"

#include "deliveryUtils.h"
#include "timeSlotUtils.h"
#include "dispatchUtils.h"

/*
 * Currently the algorithm can only asign the deliveries with "null" shifts
 * to the same day as its dueDate. To attend them earlier we can change the 
 * iteration to start from today and iterate to the max delivery date
 * in the input. And in the filterByDateAndRoute, we always return those
 * with "null" shifts. That way we can start attending deliveries with no
 * time window form today.
 */

/*
 TODOS:
 * - Actually check for block availability from the inputs when generating a new solution
 * - Confirm if null time window deliveries can be attended before the due date.
 * - Get a better and bigger randomized dataset
 * - Consider a max containers per day
 * - !!! When reassigning trucks, the random number COULD BE 0 (UNDEFINED TRUCK)
 */

using namespace std;

void printLog(const string& log, bool debug) {
    if (debug == true) cout << log << endl;
}

int main(int argc, char** argv) {
    string algorithmToRun = "saga"; // "grasp" | "saga"
    bool debug = true;
   
    // SAGA Params
    int unloadingTime = 20;
    int timeSlotInterval = 30;
    
    // GRASP Params
    
    
    Input input;
    input.loadFromFile("../input/input_test.txt");
    input.printInputData();
        
    cout << endl << "=========MAIN PROGRAM =========" << endl << endl;
    
    GlobalExecutionTracker tracker;
    vector<Dispatch> finalDispatches;
    vector<string> uniqueDueDates = input.getUniqueDueDates();
    vector<Delivery*> deliveries = input.getAllDeliveriesFromOrders();
    
    // Filter 1: By date
    for(const string& dueDate : uniqueDueDates) {
        printLog("DUE DATE = " + dueDate, debug);
        
        // Filter 2: By route
        for(const Route& route : input.getRoutes()) {
            printLog("    ROUTE ID = " + to_string(route.getId()), debug);
            vector<Delivery*> deliveriesForRoute = DeliveryUtils::filterByDateAndRoute(
                deliveries, 
                dueDate, 
                route
            );
            
            // Filter 3: By shift
            vector<string> shifts = {"morning", "afternoon"};
            for (const string& shift : shifts) {
                printLog("       SHIFT = " + shift, debug);
                vector<Delivery*> deliveriesByShift = DeliveryUtils::filterByShift(
                    deliveriesForRoute, 
                    shift
                );
                
                if (deliveriesByShift.empty()) {
                    printLog("            There were no deliveries for this shift.", debug);
                    continue;
                }

                vector<TimeSlot> timeSlots;
                if (shift == "morning") {
                    timeSlots = TimeSlotUtils::generateForWindow(
                        480, 
                        720, 
                        route, 
                        unloadingTime, 
                        timeSlotInterval
                    ); // 08:00 – 12:00
                } else if (shift == "afternoon") {
                    timeSlots = TimeSlotUtils::generateForWindow(
                        840, 
                        1080, 
                        route, 
                        unloadingTime, 
                        timeSlotInterval
                    ); // 14:00 – 18:00
                }
                
                // Filter 4: By timeslot
                for (const TimeSlot& ts : timeSlots) {
                    // 1. Filter deliveries by tracker
                    vector<Delivery*> deliveryPtrs;
                    for (Delivery* d : deliveriesByShift) {
                        if (!tracker.isDeliveryFulfilled(d->getId())) {
                            deliveryPtrs.push_back(d);
                        }
                    }
                    if (deliveryPtrs.empty()) continue;

                    
                    // 2. Filter blocks (only those not already used)
                    vector<Delivery*> finalDeliveries;
                    vector<Block*> blocksForThisBatch;

                    for (Delivery* d : deliveryPtrs) {
                        const vector<Block*>& bs = d->getBlocksToDeliver();
                        bool allBlocksAvailable = true;
                        int idOfAlreadyUsedBlock = -1;
                        
                        for (Block* b : bs) {
                            if (tracker.isBlockUsed(b->getId())) {
                                allBlocksAvailable = false;
                                idOfAlreadyUsedBlock = b->getId();
                                break;
                            }
                        }

                        if (allBlocksAvailable) {
                            finalDeliveries.push_back(d);
                            blocksForThisBatch.insert(blocksForThisBatch.end(), bs.begin(), bs.end());
                        }
                    }
                    if (finalDeliveries.empty() || blocksForThisBatch.empty()) continue;

                    // 3. Filter available vehicles for this time slot
                    vector<TransportUnit*> allVehicles;
                    for (TransportUnit& unit : input.getTransportUnits()) {
                        allVehicles.push_back(&unit);
                    }
                    vector<TransportUnit*> availableVehicles = tracker.getAvailableVehicles(allVehicles, ts, dueDate);
                    if (availableVehicles.empty()) continue;
                    
                    
                    // SELECTING AN ITERATION FOR DEBUG
                    bool matchesSelected = 
                                1 == 2 &&
                                dueDate == "2024-09-28" && 
                                route.getId() == 4 && 
                                shift == "morning";
                    
                    if (matchesSelected == true) {
                        cout << "[DEBUG] - Check deliveries" << endl;
                        for (Delivery* del : finalDeliveries) {
                            cout << del->getId() << endl;
                        }
                        
                        cout << "Check available trucks" << endl;
                        for (TransportUnit* tu : availableVehicles) {
                            cout << tu->getId() << endl;
                        }
                        
                        cout << "FINISHED PRINT ===================" << endl;
                    }
                    
                    if (algorithmToRun == "saga") {
                        SAGAOptimizer optimizer(
                            finalDeliveries,
                            blocksForThisBatch,
                            availableVehicles,
                            const_cast<Route*>(&route),
                            ts,
                            /*T_init=*/1000, 
                            /*T_min=*/1, 
                            /*alpha=*/0.95, 
                            /*populationSize=*/30
                        );

                        Chromosome best = optimizer.run(matchesSelected);
                        tracker.recordSolution(
                            best, 
                            finalDeliveries, 
                            blocksForThisBatch, 
                            availableVehicles, 
                            ts, 
                            dueDate
                        );

                        vector<Dispatch> dispatches = DispatchUtils::buildFromChromosome(
                            best, 
                            finalDeliveries, 
                            availableVehicles, 
                            const_cast<Route*>(&route), 
                            ts, 
                            dueDate
                        );
                        finalDispatches.insert(finalDispatches.end(), dispatches.begin(), dispatches.end());
                        printLog("           SAGA RESULT = " + to_string(best.getFitness()), debug);
                    } 
                    else if (algorithmToRun == "grasp") {
                        // Filtrar deliveries ya atendidos
                        vector<Delivery*> filteredDeliveries;
                        for (size_t i = 0; i < finalDeliveries.size(); ++i) {
                            if (!tracker.isDeliveryFulfilled(finalDeliveries[i]->getId())) {
                                filteredDeliveries.push_back(finalDeliveries[i]);
                            }
                        }
                        if (filteredDeliveries.empty()) continue;

                        // Estimar volumen total de los bloques
                        double totalBlockVolume = 0.0;
                        vector<Block*> blocksForFilteredDeliveries;
                        for (size_t i = 0; i < filteredDeliveries.size(); ++i) {
                            const vector<Block*>& bs = filteredDeliveries[i]->getBlocksToDeliver();
                            for (size_t j = 0; j < bs.size(); ++j) {
                                Block* b = bs[j];
                                totalBlockVolume += b->getLength() * b->getWidth() * b->getHeight();
                                blocksForFilteredDeliveries.push_back(b);
                            }
                        }

                        // Ordenar camiones por cercanía de volumen al requerido
                        vector<pair<TransportUnit*, double>> sortedVehicles;
                        for (size_t i = 0; i < availableVehicles.size(); ++i) {
                            TransportUnit* truck = availableVehicles[i];
                            double truckVol = truck->getLength() * truck->getWidth() * truck->getHeight();
                            double diff = fabs(truckVol - totalBlockVolume);
                            sortedVehicles.push_back(make_pair(truck, diff));
                        }

                        std::sort(sortedVehicles.begin(), sortedVehicles.end(),
                            [](const pair<TransportUnit*, double>& a, const pair<TransportUnit*, double>& b) {
                                return a.second < b.second;
                            });

                        // Ejecutar GRASP por camión
                        for (size_t v = 0; v < sortedVehicles.size(); ++v) {
                            TransportUnit* truck = sortedVehicles[v].first;

                            GRASPOptimizer optimizer;
                            vector<const Block*> constBlocks;
                            for (size_t i = 0; i < blocksForFilteredDeliveries.size(); ++i) {
                                constBlocks.push_back(static_cast<const Block*>(blocksForFilteredDeliveries[i]));
                            }

                            GraspPackingState packing = optimizer.constructPacking(
                                constBlocks,
                                truck,
                                0.3
                            );

                            if (packing.getPlacedLayers().empty()) {
                                continue; // nada empacado, siguiente camión
                            }

                            // Construir solución GRASP
                            GraspSolution graspSol;
                            std::set<int> usedBlockIds;
                            const vector<LayerCandidate>& layers = packing.getPlacedLayers();
                            for (size_t i = 0; i < layers.size(); ++i) {
                                const vector<const Block*>& bs = layers[i].getBlocks();
                                for (size_t j = 0; j < bs.size(); ++j) {
                                    usedBlockIds.insert(bs[j]->getId());
                                }
                            }

                            // Asignar entregas completamente empacadas
                            vector<Delivery*> attendedDeliveries;
                            for (size_t i = 0; i < filteredDeliveries.size(); ++i) {
                                Delivery* d = filteredDeliveries[i];
                                const vector<Block*>& bs = d->getBlocksToDeliver();

                                bool allIncluded = true;
                                
                                for (size_t j = 0; j < bs.size(); ++j) {
                                    if (usedBlockIds.count(bs[j]->getId()) == 0) {
                                        allIncluded = false;
                                        break;
                                    }
                                }
                                
                                if (allIncluded) {
                                    graspSol.assignDelivery(d, truck);
                                    attendedDeliveries.push_back(d);
                                }
                            }

                            // Registrar bloques empacados
                            double accumulatedZ = 0.0;
                            for (size_t i = 0; i < layers.size(); ++i) {
                                const LayerCandidate& layer = layers[i];
                                const vector<const Block*>& bs = layer.getBlocks();
                                
                                // --- DEBUG INICIO ---
                                cout << "Layer " << i << " | height = " << layer.getHeight() << " | Blocks: " << bs.size() << endl;
                                for (size_t j = 0; j < bs.size(); ++j) {
                                    const Block* block = bs[j];
                                    pair<double, double> pos = layer.getPosition(block->getId());
                                    int ori = layer.getOrientation(block->getId());
                                    double lx = (ori == 0) ? block->getLength() : block->getWidth();
                                    double ly = (ori == 0) ? block->getWidth() : block->getLength();
                                    double lz = block->getHeight();

                                    cout << " - Block " << block->getId()
                                         << " → Pos(" << pos.first << ", " << pos.second << ", " << accumulatedZ << ")"
                                         << " | Size(" << lx << ", " << ly << ", " << lz << ") | Ori: " << ori << endl;
                                }
                                // --- DEBUG FIN ---

                                for (size_t j = 0; j < bs.size(); ++j) {
                                    const Block* block = bs[j];
                                    int orientation = layer.getOrientation(block->getId());
                                    pair<double, double> pos = layer.getPosition(block->getId());

                                    double lx = (orientation == 0) ? block->getLength() : block->getWidth();
                                    double ly = (orientation == 0) ? block->getWidth() : block->getLength();
                                    double lz = block->getHeight();

                                    BlockPlacement bp;
                                    bp.blockId = block->getId();
                                    bp.orientation = orientation;
                                    bp.x = pos.first;
                                    bp.y = pos.second;
                                    bp.z = accumulatedZ;
                                    bp.lx = lx;
                                    bp.ly = ly;
                                    bp.lz = lz;

                                    graspSol.addPackedBlock(const_cast<Block*>(block), orientation, bp);
                                }

                                accumulatedZ += layer.getHeight();
                            }

                            // Registrar solución y cortar iteración
                            if (!attendedDeliveries.empty()) {
                                tracker.recordGraspSolution(
                                    graspSol,
                                    attendedDeliveries,
                                    availableVehicles,
                                    ts,
                                    dueDate
                                );

                                vector<Dispatch> dispatches = DispatchUtils::buildFromGrasp(
                                    graspSol,
                                    attendedDeliveries,
                                    availableVehicles,
                                    const_cast<Route*>(&route),
                                    ts,
                                    dueDate
                                );

                                finalDispatches.insert(finalDispatches.end(), dispatches.begin(), dispatches.end());
                                printLog("           GRASP result for truck ID " + to_string(truck->getId()) + ": " + to_string(dispatches.size()) + " dispatch(es)", debug);
                                break; // <== Muy importante para evitar múltiples asignaciones del mismo delivery
                            }
                        }
                    }
                }
            }
        }
    }
    
    
    cout << endl << "=========== FINAL DISPATCH PLAN (" 
            << finalDispatches.size() 
            << ") ===========" << endl;
    int disCounter = 0;
    for (const Dispatch& d : finalDispatches) {
        bool matchesSelected = d.getDate() == "2024-09-28" && 
                                d.getRoute()->getId() == 4 && 
                                d.getTimeSlot().getStartAsString() == "08:00" &&
                                d.getTimeSlot().getEndAsString() == "10:17";
        
        disCounter = disCounter + 1;
        
        //if (matchesSelected == false) continue;
        
        cout << "Dispatch ID: " << disCounter << endl;
        d.printSummary();
    }
    
    
    DispatchUtils::exportDispatchesToCSV(
        finalDispatches, 
        "../output/output_dispatches.csv"
    );
    
    
    cout << endl << "=========== DELIVERIES NOT ATTENDED ===========" 
            << endl;
    vector<int> unfulfilled = tracker.getUnfulfilledDeliveryIds(deliveries);
    if (unfulfilled.empty()) {
        cout << " All deliveries were dispatched." << endl;
    } else {
        for (int id : unfulfilled) {
            cout << "Delivery not dispatched → ID: " << id << endl;
        }
    }
    
    return 0;
}

