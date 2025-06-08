#include "dispatch.h"
#include <iostream>

Dispatch::Dispatch(
    TransportUnit* truck,
    Route* route,
    TimeSlot slot,
    const string& date,
    const vector<Delivery*>& deliveries,
    const vector<Block*>& blocks,
    const vector<int>& blockOrientations,
    const vector<BlockPlacement>& blockPlacements
) {
    this->truck = truck;
    this->route = route;
    this->slot = slot;
    this->date = date;
    this->deliveries = deliveries;
    this->blocks = blocks;
    this->blockOrientations = blockOrientations;
    this->blockPlacements = blockPlacements;
}

void Dispatch::printSummary() const {
    double truckVolume = truck->getVolume();
    double blocksVolume = 0.0;
    
    for(const Block* b: this->blocks) {
        blocksVolume = blocksVolume + b->getVolume();
    }
    
    cout << "Truck ID: " << truck->getId() << endl;
    cout << "Truck occupation rate: " << blocksVolume * 100 / truckVolume << "%" << endl;
    cout << "Date: " << date << endl;
    cout << "Time Slot: " << slot.getStartAsString() << " - " << slot.getEndAsString() << endl;
    cout << "Route ID: " << route->getId() << endl;
    
    cout << "Deliveries (" << deliveries.size() << "): ";
    for (const Delivery* d : deliveries) {
        cout << d->getId() << " ";
    }
    
    cout << "\nBlocks (" << blocks.size() << "): ";
    for (const Block* b : blocks) cout << b->getId() << " ";
    
    cout << "\nOrientations (" << blockOrientations.size() << "): ";
    for (const int b : blockOrientations) cout << b << " ";
    
    cout << "\nBlock Placements:" << endl;
    for (const auto& bp : blockPlacements) {
        cout << "Block " << bp.blockId
             << " → Pos(" << bp.x << ", " << bp.y << ", " << bp.z << ")"
             << " | Size(" << bp.lx << ", " << bp.ly << ", " << bp.lz << ")"
             << " | Ori: " << bp.orientation << endl;
    }

    cout << endl << "==========================" << endl;
}

TransportUnit* Dispatch::getTruck() const { return truck; }
Route* Dispatch::getRoute() const { return route; }
TimeSlot Dispatch::getTimeSlot() const { return slot; }
string Dispatch::getDate() const { return date; }
const vector<Delivery*>& Dispatch::getDeliveries() const { return deliveries; }
const vector<Block*>& Dispatch::getBlocks() const { return blocks; }
const vector<BlockPlacement>& Dispatch::getBlockPlacements() const { return blockPlacements; }
