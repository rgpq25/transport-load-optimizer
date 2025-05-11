#include "block.h"
#include "transportUnit.h"
#include "client.h"
#include "input.h"
#include "timeSlot.h"
#include <iostream>
#include <vector>
#include <string>

#include "deliveryUtils.h"
#include "timeSlotUtils.h"

/*
 * Currently the algorithm can only asign the deliveries with "null" shifts
 * to the same day as its dueDate. To attend them earlier we can change the 
 * iteration to start from today and iterate to the max delivery date
 * in the input. And in the filterByDateAndRoute, we always return those
 * with "null" shifts. That way we can start attending deliveries with no
 * time window form today.
 */

//Pass by value: Will not modify the original value passed
//Pass by reference: Will modify the original value passed

using namespace std;

int main(int argc, char** argv) {
    int unloadingTime = 20;
    int timeSlotInterval = 30;
    
    Input input;
    input.loadFromFile("data.txt");
    //input.printInputData();
    
    cout << endl << "=========MAIN PROGRAM =========" << endl << endl;
    
    vector<string> uniqueDueDates = input.getUniqueDueDates();
    vector<Delivery> deliveries = input.getAllDeliveriesFromOrders();
    
    for(const string& dueDate : uniqueDueDates) {
        for(const Route& route : input.getRoutes()) {
            vector<Delivery> deliveriesForRoute = DeliveryUtils::filterByDateAndRoute(deliveries, dueDate, route);
            
            vector<string> shifts = {"morning", "afternoon", "null"};
            for (const string& shift : shifts) {                
                vector<Delivery> deliveriesByShift = DeliveryUtils::filterByShift(deliveriesForRoute, shift);

                if (deliveriesByShift.empty()) continue;

                vector<TimeSlot> timeSlots;
                if (shift == "morning") {
                    timeSlots = TimeSlotUtils::generateForWindow(480, 720, route, unloadingTime, timeSlotInterval); // 08:00 – 12:00
                } else if (shift == "afternoon") {
                    timeSlots = TimeSlotUtils::generateForWindow(840, 1080, route, unloadingTime, timeSlotInterval); // 14:00 – 18:00
                } else {
                    timeSlots = TimeSlotUtils::generateForWindow(480, 1080, route, unloadingTime, timeSlotInterval); // full day (null shift)
                }

                for (const TimeSlot& ts : timeSlots) {
                    //vector<Delivery> elegibles = DeliveryUtils::filterByWindow(deliveriesByShift, ts, route); //this will be included if we deliver "null" shift delveries early (TODO)
                    //if (elegibles.empty()) continue;

                    cout << "→ Ejecutando SA-GA para fecha: " << dueDate
                         << ", ruta: " << route.getId()
                         << ", shift: " << shift
                         << ", time slot: " << ts.getStartAsString() << " - " << ts.getEndAsString()
                         << ", pedidos: " << deliveriesByShift.size() << endl;

                    // Empezamos con optimizacion SA-GA
                }
            }

            
        }
    }

    
    return 0;
}

