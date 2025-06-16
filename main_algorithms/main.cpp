#include <iostream>
#include <vector>
#include <string>

#include "block.h"
#include "transportUnit.h"
#include "client.h"
#include "input.h"
#include "timeSlot.h"
#include "dispatch.h"

#include "SAGAOptimizer.h"
#include "GRASPOptimizer.h"

#include "globalExecutionTracker.h"
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
    string algorithmToRun = "grasp"; // "grasp" | "saga"
    bool debug = true;
   
    // SAGA Params
    int unloadingTime = 20;
    int timeSlotInterval = 30;
    
    // GRASP Params
    
    
    Input input;
    input.loadFromFile("../input/input_test_small.txt");
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
                        // 1. Instanciar el optimizador GRASP
                        GRASPOptimizer optimizer(
                            finalDeliveries,
                            blocksForThisBatch,
                            availableVehicles,
                            const_cast<Route*>(&route),
                            ts,
                            dueDate,
                            /*graspIterations=*/100, 
                            /*Kpercent=*/20.0, 
                            /*alphaSet =*/{ 0.7, 1.0 }
                        );

                        // 2. Ejecutar GRASP y obtener patrones de vehículo
                        auto patterns = optimizer.run();

                        // 3. Registrar cada patrón y construir los dispatches
                        for (auto& pat : patterns) {
                            // Marca entregas y bloques en el tracker
                            tracker.recordSolutionPattern(pat, ts, dueDate);

                            // Convierte el VehiclePattern en Dispatches
                            auto dispatches = DispatchUtils::buildFromPattern(
                                pat,
                                finalDeliveries,
                                availableVehicles,
                                const_cast<Route*>(&route),
                                ts,
                                dueDate
                            );
                            finalDispatches.insert(
                                finalDispatches.end(),
                                dispatches.begin(),
                                dispatches.end()
                            );
                        }
                        printLog(
                            "           GRASP complete, generated " +
                            std::to_string(patterns.size()) +
                            " vehicle patterns",
                            debug
                        );
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

