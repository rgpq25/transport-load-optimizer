/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   chromosome.h
 * Author: renzo
 *
 * Created on 8 de mayo de 2025, 11:36
 */

#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include <vector>

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
};

#endif /* CHROMOSOME_H */

