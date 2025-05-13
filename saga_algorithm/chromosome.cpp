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