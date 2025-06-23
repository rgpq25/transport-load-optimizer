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


bool isDebugMatch(const string& dueDate, int routeId, const string& shift) {
    return false;
    bool isMatch =
        dueDate == "2024-09-28" && 
        routeId == 4 && 
        shift == "morning";
    
    return isMatch;
}


int main(int argc, char** argv) {
    string algorithmToRun = "saga"; // "grasp" | "saga"
    bool debug = false;
    
    int unloadingTime = 20;
    int timeSlotInterval = 30;
   
    // SAGA Params
    int T_init = 50;
    int T_min= 1;
    double alpha = 0.95;
    int populationSize= 30;
    
    // GRASP Params
    int graspIterations = 100;
    double Kpercent = 30;
    vector<double> alphaSet = { 0.1, 0.5 };
    
    
    Input input;
    input.loadFromFile("../input/input_test_small.txt");
    input.printInputData();
        
    cout << endl << "=========MAIN PROGRAM =========" << endl << endl;
    
    GlobalExecutionTracker tracker;
    vector<Dispatch> finalDispatches;
    vector<string> uniqueDueDates = input.getUniqueDueDates();
    vector<Delivery*> deliveries = input.getAllDeliveriesFromOrders();
    vector<TransportUnit*> allVehicles;
    for (TransportUnit& unit : input.getTransportUnits()) {
        allVehicles.push_back(&unit);
    }
    
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
                    vector<TransportUnit*> availableVehicles = tracker.getAvailableVehicles(allVehicles, ts, dueDate);
                    if (availableVehicles.empty()) continue;
                    
                    
                    // [DEBUG]
                    bool matchesSelected = isDebugMatch(dueDate, route.getId(), shift);
                    
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
                            T_init, 
                            T_min, 
                            alpha, 
                            populationSize
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
                            graspIterations, 
                            Kpercent, 
                            alphaSet
                        );

                        // 1) Ejecutar GRASP y obtener patrones de vehículo
                        auto patterns = optimizer.run(matchesSelected);

                        // 2) Para cada patrón, generar Dispatch(es)
                        for (const auto& pat : patterns) {
                            auto dispatches = DispatchUtils::buildFromPattern(
                                pat,
                                finalDeliveries,
                                availableVehicles,
                                const_cast<Route*>(&route),
                                ts,
                                dueDate
                            );

                            // 3) SOLO si buildFromPattern devolvió dispatches válidos:
                            if (!dispatches.empty()) {
                                // 3.a) Marcar en el tracker SOLO los deliveries que realmente se despacharon
                                for (const auto& d : dispatches) {
                                    for (Delivery* dd : d.getDeliveries()) {
                                        if (!tracker.isDeliveryFulfilled(dd->getId())) {
                                            tracker.markDeliveryFulfilled(dd->getId());
                                            for (Block* b : d.getBlocks()) {
                                                tracker.markBlockUsed(b->getId());
                                            }
                                            tracker.reserveVehicle(
                                                d.getTruck()->getId(),
                                                d.getTimeSlot(),
                                                d.getDate()
                                            );
                                        }
                                    }
                                }
                                // 3.b) Añadir esos dispatches al plan final
                                finalDispatches.insert(
                                    finalDispatches.end(),
                                    dispatches.begin(),
                                    dispatches.end()
                                );
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
        disCounter = disCounter + 1;
        
        bool matchesSelected = d.getDate() == "2024-09-28" && 
                               d.getRoute()->getId() == 4 &&
                               d.getTimeSlot().getStart() < 840;        
        //if (matchesSelected == false) continue;
        
        cout << "Dispatch ID: " << disCounter << endl;
        d.printSummary();
    }
    
    cout << endl << "======== FITNESS OF SOLUTION ==========" << endl;
    cout << DispatchUtils::evaluateDispatchesFitness(finalDispatches, deliveries, allVehicles) << endl;
    
    
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

