#ifndef GRASPSOLUTION_H
#define GRASPSOLUTION_H

#include <vector>
#include <map>
#include <string>
#include "dispatch.h"
#include "delivery.h"
#include "transportUnit.h"
#include "block.h"

using namespace std;

class GraspSolution {
private:
    map<int, TransportUnit*> deliveryAssignments;      // deliveryId → truck
    vector<Block*> packedBlocks;
    vector<int> orientations;
    vector<BlockPlacement> placements;

public:
    void assignDelivery(Delivery* delivery, TransportUnit* truck);
    void addPackedBlock(Block* block, int orientation, const BlockPlacement& placement);

    const map<int, TransportUnit*>& getAssignments() const;
    const vector<Block*>& getPackedBlocks() const;
    const vector<int>& getOrientations() const;
    const vector<BlockPlacement>& getPlacements() const;
};

#endif /* GRASPSOLUTION_H */

