/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   dispatch.h
 * Author: renzo
 *
 * Created on 13 de mayo de 2025, 09:56
 */

#ifndef DISPATCH_H
#define DISPATCH_H

#include <vector>
#include <string>

#include "transportUnit.h"
#include "route.h"
#include "timeSlot.h"
#include "delivery.h"
#include "block.h"

class Dispatch {
private:
    TransportUnit* truck;
    Route* route;
    TimeSlot slot;
    string date;
    vector<Delivery*> deliveries;
    vector<Block*> blocks;

public:
    Dispatch(
        TransportUnit* truck, 
        Route* route, 
        TimeSlot slot, 
        const string& date,
        const vector<Delivery*>& deliveries, 
        const vector<Block*>& blocks
    );

    void printSummary() const;
    
    TransportUnit* getTruck() const;
    Route* getRoute() const;
    TimeSlot getTimeSlot() const;
    string getDate() const;
    const vector<Delivery*>& getDeliveries() const;
    const vector<Block*>& getBlocks() const;
};

#endif /* DISPATCH_H */

