#include "dispatch.h"
#include <iostream>

Dispatch::Dispatch(
    TransportUnit* truck,
    Route* route,
    TimeSlot slot,
    const string& date,
    const vector<Delivery*>& deliveries,
    const vector<Block*>& blocks
) {
    this->truck = truck;
    this->route = route;
    this->slot = slot;
    this->date = date;
    this->deliveries = deliveries;
    this->blocks = blocks;
}

void Dispatch::printSummary() const {
    cout << "==== Dispatch Summary ====" << endl;
    cout << "Truck ID: " << truck->getId() << endl;
    cout << "Date: " << date << endl;
    cout << "Time Slot: " << slot.getStartAsString() << " - " << slot.getEndAsString() << endl;
    cout << "Route ID: " << route->getId() << endl;
    
    cout << "Deliveries (" << deliveries.size() << "): ";
    for (const Delivery* d : deliveries) {
        cout << d->getId() << " ";
    }
    
    cout << "\nBlocks (" << blocks.size() << "): ";
    for (const Block* b : blocks) cout << b->getId() << " ";

    cout << endl << "==========================" << endl;
}

TransportUnit* Dispatch::getTruck() const { return truck; }
Route* Dispatch::getRoute() const { return route; }
TimeSlot Dispatch::getTimeSlot() const { return slot; }
string Dispatch::getDate() const { return date; }
const vector<Delivery*>& Dispatch::getDeliveries() const { return deliveries; }
const vector<Block*>& Dispatch::getBlocks() const { return blocks; }
