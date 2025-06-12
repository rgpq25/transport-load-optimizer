#ifndef GLOBALEXECUTIONTRACKER_H
#define GLOBALEXECUTIONTRACKER_H

#include <set>
#include <unordered_map>
#include <map>
#include <vector>

#include "delivery.h"
#include "block.h"
#include "transportUnit.h"
#include "timeSlot.h"
#include "chromosome.h"
#include "graspSolution.h"

using namespace std;

class GlobalExecutionTracker {
private:
    set<int> fulfilledDeliveryIds;
    set<int> usedBlockIds;
    unordered_map<string, unordered_map<int, vector<TimeSlot>>> vehicleUsageByDate;

public:
    bool isDeliveryFulfilled(int deliveryId) const;
    bool isBlockUsed(int blockId) const;
    bool isVehicleAvailable(int vehicleId, const TimeSlot& slot, const string& date) const;

    void markDeliveryFulfilled(int deliveryId);
    void markBlockUsed(int blockId);
    void reserveVehicle(int vehicleId, const TimeSlot& slot, const string& date);

    vector<TransportUnit*> getAvailableVehicles(
        const vector<TransportUnit*>& all, 
        const TimeSlot& slot, 
        const string& date
    );

    void recordSolution(
        const Chromosome& solution,
        const vector<Delivery*>& deliveries,
        const vector<Block*>& blocks,
        const vector<TransportUnit*>& vehicles,
        const TimeSlot& slot, 
        const string& date
    );
    
    void recordGraspSolution(
        const GraspSolution& solution,
        const vector<Delivery*>& allDeliveries,
        const vector<TransportUnit*>& allVehicles,
        const TimeSlot& slot,
        const string& date
    );
    
    vector<int> getUnfulfilledDeliveryIds(const vector<Delivery*>& allDeliveries) const;
};

#endif /* GLOBALEXECUTIONTRACKER_H */

