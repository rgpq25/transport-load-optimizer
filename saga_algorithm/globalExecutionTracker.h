/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   globalExecutionTracker.h
 * Author: renzo
 *
 * Created on 12 de mayo de 2025, 18:52
 */

#ifndef GLOBALEXECUTIONTRACKER_H
#define GLOBALEXECUTIONTRACKER_H

#include <set>
#include <map>
#include <vector>

#include "delivery.h"
#include "block.h"
#include "transportUnit.h"
#include "timeSlot.h"
#include "chromosome.h"

using namespace std;

class GlobalExecutionTracker {
private:
    set<int> fulfilledDeliveryIds;
    set<int> usedBlockIds;
    map<int, vector<TimeSlot>> vehicleUsage;

public:
    bool isDeliveryFulfilled(int deliveryId) const;
    bool isBlockUsed(int blockId) const;
    bool isVehicleAvailable(int vehicleId, const TimeSlot& slot) const;

    void markDeliveryFulfilled(int deliveryId);
    void markBlockUsed(int blockId);
    void reserveVehicle(int vehicleId, const TimeSlot& slot);

    vector<TransportUnit*> getAvailableVehicles(
        const std::vector<TransportUnit*>& all, 
        const TimeSlot& slot
    );

    void recordSolution(
        const Chromosome& solution,
        const std::vector<Delivery*>& deliveries,
        const std::vector<Block*>& blocks,
        const std::vector<TransportUnit*>& vehicles,
        const TimeSlot& slot
    );
};

#endif /* GLOBALEXECUTIONTRACKER_H */

