#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include <vector>
#include "delivery.h"
#include "transportUnit.h"

using namespace std;

class Chromosome {
private:
    vector<int> deliveryAssignments;
    vector<int> boxOrientations;
    double fitness;
public:
    Chromosome();
    Chromosome(vector<int> deliveryAssignments, vector<int> boxOrientations);

    const vector<int>& getDeliveryAssignments() const;
    const vector<int>& getBoxOrientations() const;
    vector<int>& getDeliveryAssignments();
    vector<int>& getBoxOrientations();
    double getFitness() const;
    void setFitness(double f);
    
    vector<int> getAssignedBoxOrientations(int vehicleIdx, const vector<Delivery*>& deliveries) const;
    
    void printChromosome(const vector<Delivery*> deliveries, const vector<TransportUnit*> transportUnits);

};

#endif /* CHROMOSOME_H */

