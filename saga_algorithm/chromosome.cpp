#include "chromosome.h"

Chromosome::Chromosome() {
    this->fitness = 0;
}

Chromosome::Chromosome(vector<int> deliveryAssignments, vector<int> boxOrientations) {
    this->deliveryAssignments = deliveryAssignments;
    this->boxOrientations = boxOrientations;
    this->fitness = 0;
}

vector<int>& Chromosome::getDeliveryAssignments() {
    return deliveryAssignments;
}

vector<int>& Chromosome::getBoxOrientations() {
    return boxOrientations;
}

double Chromosome::getFitness() const {
    return fitness;
}

void Chromosome::setFitness(double f) {
    fitness = f;
}