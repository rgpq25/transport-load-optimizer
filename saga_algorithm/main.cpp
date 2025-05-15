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
 */

using namespace std;

void printLog(const string& log, bool debug) {
    if (debug == true) cout << log << endl;
}

int main(int argc, char** argv) {
    int unloadingTime = 20;
    int timeSlotInterval = 30;
    bool debug = false;
    
    Input input;
    input.loadFromFile("../input/input_large.txt");
    
    
    input.printInputData();
    //return 0;
    
    
    cout << endl << "=========MAIN PROGRAM =========" << endl << endl;
    
    GlobalExecutionTracker tracker;
    vector<Dispatch> finalDispatches;
    vector<string> uniqueDueDates = input.getUniqueDueDates();
    vector<Delivery*> deliveries = input.getAllDeliveriesFromOrders();
    
    for(const string& dueDate : uniqueDueDates) {
        printLog("DUE DATE = " + dueDate, debug);
        
        for(const Route& route : input.getRoutes()) {
            printLog("    ROUTE ID = " + to_string(route.getId()), debug);
            vector<Delivery*> deliveriesForRoute = DeliveryUtils::filterByDateAndRoute(deliveries, dueDate, route);
            
            vector<string> shifts = {"morning", "afternoon"};
            for (const string& shift : shifts) {
                printLog("       SHIFT = " + shift, debug);
                vector<Delivery*> deliveriesByShift = DeliveryUtils::filterByShift(deliveriesForRoute, shift);
                if (deliveriesByShift.empty()) {
                    printLog("            There were no deliveries for this shift.", debug);
                    continue;
                }

                vector<TimeSlot> timeSlots;
                if (shift == "morning") {
                    timeSlots = TimeSlotUtils::generateForWindow(480, 720, route, unloadingTime, timeSlotInterval); // 08:00 – 12:00
                } else if (shift == "afternoon") {
                    timeSlots = TimeSlotUtils::generateForWindow(840, 1080, route, unloadingTime, timeSlotInterval); // 14:00 – 18:00
                }
                

                for (const TimeSlot& ts : timeSlots) {
                    // 1. Filter deliveries by tracker
                    vector<Delivery*> deliveryPtrs;
                    for (Delivery* d : deliveriesByShift) {
                        if (!tracker.isDeliveryFulfilled(d->getId())) {
                            deliveryPtrs.push_back(d);
                        }
                    }
                    if (deliveryPtrs.empty()) continue;

                    
                    // 2. Filter blocks (only those not already used). If block not available, discard said delivery (no partial deliveries allowed)
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
                        } else {
                            cout << "[DEBUG] Delivery " << d->getId() << " cannot be attended: at least one block already used (" << idOfAlreadyUsedBlock << ")" << endl;
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
                    
                    
                    // 4. Create and run optimizer
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

                    Chromosome best = optimizer.run();
                    tracker.recordSolution(best, finalDeliveries, blocksForThisBatch, availableVehicles, ts, dueDate);

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
            }
        }
    }
    
    
    cout << endl << "=========== FINAL DISPATCH PLAN (" << finalDispatches.size() << ") ===========" << endl;
    int disCounter = 1;
    for (const Dispatch& d : finalDispatches) {
        cout << "Dispatch ID: " << disCounter << endl;
        d.printSummary();
        
        disCounter = disCounter + 1;
    }
    DispatchUtils::exportDispatchesToCSV(finalDispatches, "../output/output_dispatches.csv");
    
    
    cout << endl << "=========== DELIVERIES NOT ATTENDED ===========" << endl;
    vector<int> unfulfilled = tracker.getUnfulfilledDeliveryIds(deliveries);

    if (unfulfilled.empty()) {
        cout << " Todos los deliveries fueron atendidos." << endl;
    } else {
        for (int id : unfulfilled) {
            cout << "Delivery no atendido → ID: " << id << endl;
        }
    }
    
    return 0;
}

