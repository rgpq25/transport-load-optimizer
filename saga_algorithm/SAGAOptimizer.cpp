/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   SAGAOptimizer.cpp
 * Author: renzo
 * 
 * Created on 12 de mayo de 2025, 15:59
 */

#include "SAGAOptimizer.h"

SAGAOptimizer::SAGAOptimizer(
    vector<Delivery*>& deliveries,
    vector<Block*>& blocks,
    vector<TransportUnit*>& vehicles,
    Route* route,
    TimeSlot timeSlot,
    int T_init,
    int T_min,
    double alpha,
    int populationSize
) {
    this->deliveries = deliveries;
    this->blocks = blocks;
    this->vehicles = vehicles;
    this->route = route;
    this->timeSlot = timeSlot;
    this->T_init = T_init;
    this->T_min = T_min;
    this->alpha = alpha;
    this->populationSize = populationSize;
}


Chromosome SAGAOptimizer::run() {
    vector<Chromosome> population = generateInitialPopulation();

    // 1. Evaluate initial fitness
    for (Chromosome& chromosome : population) {
        chromosome.setFitness(evaluateFitness(chromosome));
    }

    // 2. Select the best initial chromosome
    Chromosome best = population[0];
    for (const Chromosome& c : population) {
        if (c.getFitness() > best.getFitness()) {
            best = c;
        }
    }

    // 3. Simulated Annealing loop
    int T = T_init;

    while (T > T_min) {
        vector<Chromosome> newPopulation;

        for (int i = 0; i < populationSize; ++i) {
            Chromosome parent1 = selectParent(population);
            Chromosome parent2 = selectParent(population);

            Chromosome child = crossover(parent1, parent2);
            mutate(child);
            child.setFitness(evaluateFitness(child));

            // Apply acceptance criteria (Simulated Annealing)
            double delta = parent1.getFitness() - child.getFitness();

            if (delta < 0) {
                newPopulation.push_back(child);
            } else {
                double prob = exp(-delta / T);
                double randVal = (double)rand() / RAND_MAX;

                if (randVal < prob) {
                    newPopulation.push_back(child);
                } else {
                    newPopulation.push_back(parent1);
                }
            }

            // Update best solution
            if (child.getFitness() > best.getFitness()) {
                best = child;
            }
        }

        population = newPopulation;
        T = T * alpha;
    }

    return best;
}