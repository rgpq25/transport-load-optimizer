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
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <iostream>

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
        double currFitness = evaluateFitness(chromosome);
        chromosome.setFitness(currFitness);
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




vector<Chromosome> SAGAOptimizer::generateInitialPopulation() {
    vector<Chromosome> population;
    int numDeliveries = deliveries.size();
    int numBlocks = blocks.size();
    int numVehicles = vehicles.size();

    srand(time(nullptr)); // Seed RNG once

    for (int i = 0; i < populationSize; ++i) {
        // --- 1. Random delivery-to-vehicle assignment ---
        vector<int> deliveryAssignments;
        for (int d = 0; d < numDeliveries; ++d) {
            int assignedVehicle = (rand() % (numVehicles + 1)) - 1; // Gives range: [-1, 0, ..., numVehicles - 1]
            deliveryAssignments.push_back(assignedVehicle);
        }

        // --- 2. Random orientation assignment (0 or 1) ---
        vector<int> boxOrientations;
        for (int b = 0; b < numBlocks; ++b) {
            int orientation = rand() % 2;
            boxOrientations.push_back(orientation);
        }

        // --- 3. Add to population ---
        Chromosome chromosome(deliveryAssignments, boxOrientations);
        population.push_back(chromosome);
    }

    return population;
}




double SAGAOptimizer::evaluateFitness(Chromosome& chromosome) {
    const vector<int>& deliveryAssignments = chromosome.getDeliveryAssignments();
    const vector<int>& blockOrientations = chromosome.getBoxOrientations();

    // Map vehicle index → blocks assigned to it
    unordered_map<int, vector<Block*>> vehicleBlocks;
    unordered_map<int, double> vehicleVolumeUsed;
    unordered_map<int, double> vehicleWeightUsed;
    unordered_set<int> usedVehicles;
    
    int numDeliveriesAssigned = 0;

    // Go through all deliveries and assign their blocks to the vehicle assigned
    for (int i = 0; i < deliveries.size(); ++i) {
        int vehicleIndex = deliveryAssignments[i];
        if (vehicleIndex < 0 || vehicleIndex >= vehicles.size()) continue;
        
        numDeliveriesAssigned++;

        Delivery* d = deliveries[i];
        const vector<Block*>& dBlocks = d->getBlocksToDeliver();

        for (Block* b : dBlocks) {
            vehicleBlocks[vehicleIndex].push_back(b);
            double volume = b->getHeight() * b->getWidth() * b->getLength();
            vehicleVolumeUsed[vehicleIndex] += volume;
            vehicleWeightUsed[vehicleIndex] += b->getWeight();
        }

        usedVehicles.insert(vehicleIndex);
    }

    // Evaluate fitness
    double totalUtilizationScore = 0.0;
    double overcapacityPenalty = 0.0;

    for (int vIdx : usedVehicles) {
        TransportUnit* vehicle = vehicles[vIdx];

        double maxVolume = vehicle->getHeight() * vehicle->getWidth() * vehicle->getLength();
        double maxWeight = vehicle->getMaxWeight();

        double usedVolume = vehicleVolumeUsed[vIdx];
        double usedWeight = vehicleWeightUsed[vIdx];

        double utilization = usedVolume / maxVolume;
        totalUtilizationScore += utilization;

        if (usedWeight > maxWeight) {
            overcapacityPenalty += (usedWeight - maxWeight) * 10.0;  // Penalize overweight
        }
    }

    int numVehiclesUsed = usedVehicles.size();
    double avgUtilization = usedVehicles.empty() ? 0.0 : totalUtilizationScore / numVehiclesUsed;
    double fulfillmentRatio = deliveries.empty() ? 0.0 : (double)numDeliveriesAssigned / deliveries.size();

    // Simple fitness function (can be tuned)
    double A = 0.5;  // minimize truck count
    double B = 0.5;  // maximize volume utilization
    double C = 1.0;  // penalty factor
    double D = 1.0;  // reward for fulfilling deliveries
    
    double truckScore = (vehicles.empty()) ? 0.0 : (1.0 - (double(numVehiclesUsed) / vehicles.size()));

    double fitness = A * truckScore +
                     B * avgUtilization -
                     C * overcapacityPenalty +
                     D * fulfillmentRatio;

    return fitness;
}



Chromosome SAGAOptimizer::selectParent(const vector<Chromosome>& population) {
    int tournamentSize = 3;
    Chromosome best = population[rand() % population.size()];

    for (int i = 1; i < tournamentSize; ++i) {
        Chromosome challenger = population[rand() % population.size()];
        if (challenger.getFitness() > best.getFitness()) {
            best = challenger;
        }
    }

    return best;
}



Chromosome SAGAOptimizer::crossover(const Chromosome& p1, const Chromosome& p2) {
    const vector<int>& d1 = p1.getDeliveryAssignments();
    const vector<int>& d2 = p2.getDeliveryAssignments();

    const vector<int>& o1 = p1.getBoxOrientations();
    const vector<int>& o2 = p2.getBoxOrientations();

    vector<int> childDeliveries;
    vector<int> childOrientations;

    // Crossover for delivery assignments
    for (size_t i = 0; i < d1.size(); ++i) {
        int gene = (rand() % 2 == 0) ? d1[i] : d2[i];
        childDeliveries.push_back(gene);
    }

    // Crossover for orientations
    for (size_t i = 0; i < o1.size(); ++i) {
        int gene = (rand() % 2 == 0) ? o1[i] : o2[i];
        childOrientations.push_back(gene);
    }

    return Chromosome(childDeliveries, childOrientations);
}




void SAGAOptimizer::mutate(Chromosome& c) {
    vector<int>& deliveries = c.getDeliveryAssignments();
    vector<int>& orientations = c.getBoxOrientations();

    const double deliveryMutationRate = 0.1;
    const double orientationMutationRate = 0.1;

    // Mutate delivery assignments
    for (size_t i = 0; i < deliveries.size(); ++i) {
        if ((rand() / double(RAND_MAX)) < deliveryMutationRate) {
            int newVehicle = rand() % vehicles.size();
            deliveries[i] = newVehicle;
        }
    }

    // Mutate block orientations
    for (size_t i = 0; i < orientations.size(); ++i) {
        if ((rand() / double(RAND_MAX)) < orientationMutationRate) {
            orientations[i] = 1 - orientations[i]; // Flip 0 ↔ 1
        }
    }
}