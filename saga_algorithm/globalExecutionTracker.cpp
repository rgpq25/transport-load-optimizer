#include "globalExecutionTracker.h"

bool GlobalExecutionTracker::isDeliveryFulfilled(int deliveryId) const {
    return fulfilledDeliveryIds.count(deliveryId) > 0;
}

bool GlobalExecutionTracker::isBlockUsed(int blockId) const {
    return usedBlockIds.count(blockId) > 0;
}

bool GlobalExecutionTracker::isVehicleAvailable(int vehicleId, const TimeSlot& slot) const {
    auto it = vehicleUsage.find(vehicleId);
    if (it == vehicleUsage.end()) return true;

    for (const TimeSlot& ts : it->second) {
        // Check for overlap: if current slot overlaps with any assigned one
        if (!(slot.getEnd() <= ts.getStart() || slot.getStart() >= ts.getEnd())) {
            return false;
        }
    }
    return true;
}

void GlobalExecutionTracker::markDeliveryFulfilled(int deliveryId) {
    fulfilledDeliveryIds.insert(deliveryId);
}

void GlobalExecutionTracker::markBlockUsed(int blockId) {
    usedBlockIds.insert(blockId);
}

void GlobalExecutionTracker::reserveVehicle(int vehicleId, const TimeSlot& slot) {
    vehicleUsage[vehicleId].push_back(slot);
}

vector<TransportUnit*> GlobalExecutionTracker::getAvailableVehicles(const vector<TransportUnit*>& all, const TimeSlot& slot) {
    vector<TransportUnit*> result;
    for (TransportUnit* v : all) {
        if (isVehicleAvailable(v->getId(), slot)) {
            result.push_back(v);
        }
    }
    return result;
}

void GlobalExecutionTracker::recordSolution(
    const Chromosome& solution,
    const vector<Delivery*>& deliveries,
    const vector<Block*>& blocks,
    const vector<TransportUnit*>& vehicles,
    const TimeSlot& slot
) {
    const vector<int>& deliveryAssignments = solution.getDeliveryAssignments();
    const vector<int>& boxOrientations = solution.getBoxOrientations();

    // Register deliveries and their vehicle usage
    for (int i = 0; i < deliveries.size(); ++i) {
        int vehicleIndex = deliveryAssignments[i];
        if (vehicleIndex < 0 || vehicleIndex >= vehicles.size()) continue;

        markDeliveryFulfilled(deliveries[i]->getId());
        reserveVehicle(vehicles[vehicleIndex]->getId(), slot);
        
        const std::vector<Block*>& bs = deliveries[i]->getBlocksToDeliver();
        for (Block* b : bs) {
            markBlockUsed(b->getId());
        }
    }
}
