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

int main(int argc, char** argv) {
    int unloadingTime = 20;
    int timeSlotInterval = 30;
    
    Input input;
    input.loadFromFile("datasets/input1.txt");
    input.printInputData();
    
    //cout << endl << "=========MAIN PROGRAM =========" << endl << endl;
    
    GlobalExecutionTracker tracker;
    vector<Dispatch> finalDispatches;
    vector<string> uniqueDueDates = input.getUniqueDueDates();
    vector<Delivery*> deliveries = input.getAllDeliveriesFromOrders();
    
    for(const string& dueDate : uniqueDueDates) {
        for(const Route& route : input.getRoutes()) {
            vector<Delivery*> deliveriesForRoute = DeliveryUtils::filterByDateAndRoute(deliveries, dueDate, route);
            
            vector<string> shifts = {"morning", "afternoon"};
            for (const string& shift : shifts) {                
                vector<Delivery*> deliveriesByShift = DeliveryUtils::filterByShift(deliveriesForRoute, shift);

                if (deliveriesByShift.empty()) continue;

                vector<TimeSlot> timeSlots;
                if (shift == "morning") {
                    timeSlots = TimeSlotUtils::generateForWindow(480, 720, route, unloadingTime, timeSlotInterval); // 08:00 – 12:00
                } else if (shift == "afternoon") {
                    timeSlots = TimeSlotUtils::generateForWindow(840, 1080, route, unloadingTime, timeSlotInterval); // 14:00 – 18:00
                }

                for (const TimeSlot& ts : timeSlots) {
                    //vector<Delivery> elegibles = DeliveryUtils::filterByWindow(deliveriesByShift, ts, route); //this will be included if we deliver "null" shift delveries early (TODO)
                    //if (elegibles.empty()) continue;

                    /*
                    cout << endl << "→ Ejecutando SA-GA para fecha: " << dueDate
                         << ", ruta: " << route.getId()
                         << ", shift: " << shift
                         << ", time slot: " << ts.getStartAsString() << " - " << ts.getEndAsString()
                         << ", pedidos: " << deliveriesByShift.size() << endl;
                    */
                    
                    // 1. Filter deliveries by tracker
                    vector<Delivery*> deliveryPtrs;
                    for (Delivery* d : deliveriesByShift) {
                        if (!tracker.isDeliveryFulfilled(d->getId())) {
                            deliveryPtrs.push_back(d);
                        }
                    }
                    if (deliveryPtrs.empty()) continue;

                    // 2. Filter blocks (only those not already used)
                    vector<Block*> blocksForThisBatch;
                    for (Delivery* d : deliveryPtrs) {
                        const vector<Block*>& bs = d->getBlocksToDeliver();
                        for (Block* b : bs) {
                            if (!tracker.isBlockUsed(b->getId())) {
                                blocksForThisBatch.push_back(b);
                            }
                        }
                    }
                    if (blocksForThisBatch.empty()) continue;

                    // 3. Filter available vehicles for this time slot
                    vector<TransportUnit*> allVehicles;
                    for (TransportUnit& unit : input.getTransportUnits()) {
                        allVehicles.push_back(&unit);
                    }
                    vector<TransportUnit*> availableVehicles = tracker.getAvailableVehicles(allVehicles, ts);
                    if (availableVehicles.empty()) continue;

                    // 4. Create and run optimizer
                    SAGAOptimizer optimizer(
                        deliveryPtrs,
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
                    tracker.recordSolution(best, deliveryPtrs, blocksForThisBatch, availableVehicles, ts);
                    
                    //cout << " Best solution found with fitness: " << best.getFitness() << endl;

                    vector<Dispatch> dispatches = DispatchUtils::buildFromChromosome(
                        best, 
                        deliveryPtrs, 
                        availableVehicles, 
                        const_cast<Route*>(&route), 
                        ts, 
                        dueDate
                    );
                    finalDispatches.insert(finalDispatches.end(), dispatches.begin(), dispatches.end());
                }
            }
        }
    }

    cout << endl << "=========== FINAL DISPATCH PLAN ===========" << endl;
    for (const Dispatch& d : finalDispatches) {
        d.printSummary();
    }
    return 0;
}

