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

void GlobalExecutionTracker::recordDispatchSolution(const vector<Dispatch>& dispatches) {
    for (const auto& d : dispatches) {
        for (Delivery* dd : d.getDeliveries()) {
            if (!isDeliveryFulfilled(dd->getId())) {
                markDeliveryFulfilled(dd->getId());
                for (Block* b : d.getBlocks()) {
                    markBlockUsed(b->getId());
                }
                reserveVehicle(
                    d.getTruck()->getId(),
                    d.getTimeSlot(),
                    d.getDate()
                );
            }
        }
    }
}

vector<int> GlobalExecutionTracker::getUnfulfilledDeliveryIds(const vector<Delivery*>& allDeliveries) const {
    vector<int> result;
    for (const Delivery* d : allDeliveries) {
        if (!isDeliveryFulfilled(d->getId())) {
            result.push_back(d->getId());
        }
    }
    return result;
}
