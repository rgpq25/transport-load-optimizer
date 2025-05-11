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

#include "gene.h"
#include "dispatchPlan.h"
#include <vector>

class Chromosome {
private:
    vector<Gene> genes;
    vector<DispatchPlan> dispatchPlans;
    double fitness;
public:
    Chromosome();
};

#endif /* CHROMOSOME_H */

