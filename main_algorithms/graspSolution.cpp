#include "graspSolution.h"

void GraspSolution::assignDelivery(Delivery* delivery, TransportUnit* truck) {
    deliveryAssignments[delivery->getId()] = truck;
}

void GraspSolution::addPackedBlock(Block* block, int orientation, const BlockPlacement& placement) {
    packedBlocks.push_back(block);
    orientations.push_back(orientation);
    placements.push_back(placement);
}

const map<int, TransportUnit*>& GraspSolution::getAssignments() const {
    return deliveryAssignments;
}

const vector<Block*>& GraspSolution::getPackedBlocks() const {
    return packedBlocks;
}

const vector<int>& GraspSolution::getOrientations() const {
    return orientations;
}

const vector<BlockPlacement>& GraspSolution::getPlacements() const {
    return placements;
}
