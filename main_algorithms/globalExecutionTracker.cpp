#include "globalExecutionTracker.h"

bool GlobalExecutionTracker::isDeliveryFulfilled(int deliveryId) const {
    return fulfilledDeliveryIds.count(deliveryId) > 0;
}

bool GlobalExecutionTracker::isBlockUsed(int blockId) const {
    return usedBlockIds.count(blockId) > 0;
}

bool GlobalExecutionTracker::isVehicleAvailable(int vehicleId, const TimeSlot& slot, const string& date) const {
    auto dayIt = vehicleUsageByDate.find(date);
    if (dayIt == vehicleUsageByDate.end()) return true;

    auto vehIt = dayIt->second.find(vehicleId);
    if (vehIt == dayIt->second.end()) return true;

    for (const TimeSlot& ts : vehIt->second) {
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

void GlobalExecutionTracker::reserveVehicle(int vehicleId, const TimeSlot& slot, const string& date) {
    vehicleUsageByDate[date][vehicleId].push_back(slot);
}

vector<TransportUnit*> GlobalExecutionTracker::getAvailableVehicles(const vector<TransportUnit*>& all, const TimeSlot& slot, const string& date) {
    vector<TransportUnit*> result;
    for (TransportUnit* v : all) {
        if (isVehicleAvailable(v->getId(), slot, date)) {
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
    const TimeSlot& slot, 
    const string& date
) {
    const vector<int>& deliveryAssignments = solution.getDeliveryAssignments();

    // Register deliveries and their vehicle usage
    for (int i = 0; i < deliveries.size(); ++i) {
        int vehicleIndex = deliveryAssignments[i];
        if (vehicleIndex < 0 || vehicleIndex >= vehicles.size()) continue;

        markDeliveryFulfilled(deliveries[i]->getId());
        reserveVehicle(vehicles[vehicleIndex]->getId(), slot, date);
        
        const std::vector<Block*>& bs = deliveries[i]->getBlocksToDeliver();
        for (Block* b : bs) {
            markBlockUsed(b->getId());
        }
    }
}

void GlobalExecutionTracker::recordSolutionPattern(
    const VehiclePattern& pattern,
    const TimeSlot& slot,
    const string& date
) {
    // 1. Reservar el vehículo
    int vehicleId = pattern.vehicle->getId();
    reserveVehicle(vehicleId, slot, date);

    // 2. Marcar entregas y bloques como usados
    for (const auto& layer : pattern.layers) {
        // Delivery
        int deliveryId = layer.delivery->getId();
        if (!isDeliveryFulfilled(deliveryId)) {
            markDeliveryFulfilled(deliveryId);
        }
        // Bloques de esa entrega
        for (Block* b : layer.blocks) {
            int blockId = b->getId();
            if (!isBlockUsed(blockId)) {
                markBlockUsed(blockId);
            }
        }
    }
};

vector<int> GlobalExecutionTracker::getUnfulfilledDeliveryIds(const vector<Delivery*>& allDeliveries) const {
    vector<int> result;
    for (const Delivery* d : allDeliveries) {
        if (!isDeliveryFulfilled(d->getId())) {
            result.push_back(d->getId());
        }
    }
    return result;
}
