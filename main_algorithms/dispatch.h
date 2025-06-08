#ifndef DISPATCH_H
#define DISPATCH_H

#include <vector>
#include <string>

#include "transportUnit.h"
#include "route.h"
#include "timeSlot.h"
#include "delivery.h"
#include "block.h"

struct BlockPlacement {
    int blockId;
    int orientation; // 0 or 1
    double x, y, z;
    double lx, ly, lz; // rotated dimensions
};

class Dispatch {
private:
    TransportUnit* truck;
    Route* route;
    TimeSlot slot;
    string date;
    vector<Delivery*> deliveries;
    vector<Block*> blocks;
    vector<int> blockOrientations;
    vector<BlockPlacement> blockPlacements;

public:
    Dispatch(
        TransportUnit* truck, 
        Route* route, 
        TimeSlot slot, 
        const string& date,
        const vector<Delivery*>& deliveries, 
        const vector<Block*>& blocks,
        const vector<int>& blockOrientations,
        const vector<BlockPlacement>& blockPlacements
    );

    void printSummary() const;
    
    TransportUnit* getTruck() const;
    Route* getRoute() const;
    TimeSlot getTimeSlot() const;
    string getDate() const;
    const vector<Delivery*>& getDeliveries() const;
    const vector<Block*>& getBlocks() const;
    const vector<BlockPlacement>& getBlockPlacements() const;
};

#endif /* DISPATCH_H */

