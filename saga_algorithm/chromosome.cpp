#include "chromosome.h"

Chromosome::Chromosome() {
    this->fitness = 0;
}

Chromosome::Chromosome(vector<int> deliveryAssignments, vector<int> boxOrientations) {
    this->deliveryAssignments = deliveryAssignments;
    this->boxOrientations = boxOrientations;
    this->fitness = 0;
}

const vector<int>& Chromosome::getDeliveryAssignments() const {
    return this->deliveryAssignments;
}

const vector<int>& Chromosome::getBoxOrientations() const {
    return this->boxOrientations;
}

vector<int>& Chromosome::getDeliveryAssignments() {
    return this->deliveryAssignments;
}

vector<int>& Chromosome::getBoxOrientations() {
    return this->boxOrientations;
}

double Chromosome::getFitness() const {
    return this->fitness;
}

void Chromosome::setFitness(double f) {
    this->fitness = f;
}


vector<int> Chromosome::getAssignedBoxOrientations(int vehicleIdx, const vector<Delivery*>& deliveries) const {
    vector<int> result;
    int orientationIndex = 0;

    for (size_t i = 0; i < deliveryAssignments.size(); ++i) {
        const int assigned = deliveryAssignments[i];
        const vector<Block*>& blocks = deliveries[i]->getBlocksToDeliver();
        
        if (assigned == vehicleIdx) {
            for (size_t j = 0; j < blocks.size(); ++j) {
                result.push_back(boxOrientations[orientationIndex + j]);
            }
        }

        orientationIndex += blocks.size();  // Always advance
    }

    return result;
}