#include "chromosome.h"

Chromosome::Chromosome() {
    this->fitness = 0;
}

Chromosome::Chromosome(vector<int> orderAssignments, vector<int> boxOrientations) {
    this->orderAssignments = orderAssignments;
    this->boxOrientations = boxOrientations;
    this->fitness = 0;
}

vector<int>& Chromosome::getOrderAssignments() {
    return orderAssignments;
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