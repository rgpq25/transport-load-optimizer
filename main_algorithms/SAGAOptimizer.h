/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   SAGAOptimizer.h
 * Author: renzo
 *
 * Created on 12 de mayo de 2025, 15:59
 */

#ifndef SAGAOPTIMIZER_H
#define SAGAOPTIMIZER_H

#include <vector>

#include "delivery.h"
#include "block.h"
#include "transportUnit.h"
#include "route.h"
#include "timeSlot.h"

#include "chromosome.h"
#include "traceUtils.h"

using namespace std;

class SAGAOptimizer {
private:
    vector<Delivery*> deliveries;
    vector<Block*> blocks;
    vector<TransportUnit*> vehicles;
    Route* route;
    TimeSlot timeSlot;

    int populationSize;
    double mutationRate;
    int T_init;
    int T_min;
    double alpha;

    // Trace mode (disabled by default)
    string traceDir;
    string runId;
    bool reseed = true;

public:
    SAGAOptimizer(
        vector<Delivery*>& deliveries,
        vector<Block*>& blocks,
        vector<TransportUnit*>& vehicles,
        Route* route,
        TimeSlot timeSlot,
        int populationSize,
        double mutationRate,
        int T_init,
        int T_min,
        double alpha        
    );

    Chromosome run(bool printDebug); // runs the SA-GA loop and returns the best solution

    void setTrace(const string& traceDir, const string& runId);
    void disableReseed();   // keeps the global seed instead of reseeding with time(nullptr)

private:
    vector<Chromosome> generateInitialPopulation();
    double evaluateFitness(Chromosome& chromosome, FitnessBreakdown* breakdown = nullptr);
    Chromosome crossover(const Chromosome& p1, const Chromosome& p2);
    void mutate(Chromosome& c);
    Chromosome selectParent(const vector<Chromosome>& population, int* chosenIndex = nullptr);

    string assignmentsToString(const Chromosome& c) const;
    string orientationsToString(const Chromosome& c) const;
};

#endif /* SAGAOPTIMIZER_H */

