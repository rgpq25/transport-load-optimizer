#include "SAGAOptimizer.h"
#include "bin3D.h"
#include "dispatchUtils.h"
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>

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




Chromosome SAGAOptimizer::run(bool printDebug) {
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
    double T = T_init;

    ofstream debugFile;
    int iterationIndex = 0;
    int totalIterationsEstimate = static_cast<int>(log(static_cast<double>(T_min) / T_init) / log(alpha)) * -1;
    int summaryStep = max(1, totalIterationsEstimate / 19); // 20 points → 19 gaps
    
    if (printDebug == true) {
        debugFile.open("../output/saga_debug_log.csv");
        debugFile << "Temperatura,Fitness promedio,Mejor fitness poblacion,Mejor fitness global,Duracion (s)\n";
    }
    
    while (T > T_min) {
        auto start = chrono::high_resolution_clock::now();  // Inicio de medición de tiempo
        
        vector<Chromosome> newPopulation;
        double sumFitness = 0.0;
        double bestFitnessInPopulation = population[0].getFitness();

        for (int i = 0; i < populationSize; ++i) {
            Chromosome parent1 = selectParent(population);
            Chromosome parent2 = selectParent(population);

            Chromosome child = crossover(parent1, parent2);
            mutate(child);
            child.setFitness(evaluateFitness(child));
            
            double childFitness = child.getFitness();
            if (childFitness > bestFitnessInPopulation) {
                bestFitnessInPopulation = childFitness;
            }
            if (parent1.getFitness() > bestFitnessInPopulation) {
                bestFitnessInPopulation = parent1.getFitness();
            }
            if (parent2.getFitness() > bestFitnessInPopulation) {
                bestFitnessInPopulation = parent2.getFitness();
            }
            
            // Apply acceptance criteria (Simulated Annealing)
            double delta = parent1.getFitness() - childFitness;

            if (delta < 0) {
                newPopulation.push_back(child);
                sumFitness += child.getFitness();
            } else {
                double prob = exp(-delta / T);
                double randVal = (double)rand() / RAND_MAX;

                if (randVal < prob) {
                    newPopulation.push_back(child);
                    sumFitness += child.getFitness();
                } else {
                    newPopulation.push_back(parent1);
                    sumFitness += parent1.getFitness();
                }
            }
            
            // Update best solution
            if (child.getFitness() > best.getFitness()) {
                best = child;
            }
        }
        
        auto end = chrono::high_resolution_clock::now();  // Fin de medición de tiempo
        chrono::duration<double> duration = end - start;

        if (printDebug && (iterationIndex == 0 || T <= T_min || iterationIndex % summaryStep == 0)) {
            double avgFitness = sumFitness / populationSize;
            debugFile  << T << ","
                 << avgFitness << ","
                 << bestFitnessInPopulation << ","
                 << best.getFitness() << ","
                 << duration.count() << "\n";
        }
        

        population = newPopulation;
        T = T * alpha;
        iterationIndex++;
    }
    
    if (printDebug) {
        debugFile.close();
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

    // Map vehicle index → blocks assigned to it
    unordered_map<int, vector<Block*>> vehicleBlocks;
    unordered_map<int, double> vehicleVolumeUsed;
    unordered_map<int, double> vehicleWeightUsed;
    unordered_set<int> usedVehicles;
    unordered_map<int, int> transportUsageById;
    
    int numDeliveriesAssigned = 0;
    int totalPriority = 0;
    int attendedPriority = 0;

    // Go through all deliveries and assign their blocks to the vehicle assigned
    for (int i = 0; i < deliveries.size(); ++i) {
        int priority = deliveries[i]->getPriority();
        totalPriority += priority;
        
        int vehicleIndex = deliveryAssignments[i];
        if (vehicleIndex < 0 || vehicleIndex >= vehicles.size()) continue;
        
        transportUsageById[vehicleIndex] += 1;
        
        attendedPriority += priority;
        numDeliveriesAssigned++;

        Delivery* d = deliveries[i];
        const vector<Block*>& dBlocks = d->getBlocksToDeliver();

        for (Block* b : dBlocks) {
            vehicleBlocks[vehicleIndex].push_back(b);
            vehicleVolumeUsed[vehicleIndex] += b->getVolume();
            vehicleWeightUsed[vehicleIndex] += b->getWeight();
        }

        usedVehicles.insert(vehicleIndex);
    }
    
    // 3D Feasibility Check
    int boxIndex = 0;
    for (int vIdx : usedVehicles) {
        TransportUnit* vehicle = vehicles[vIdx];
        const vector<Block*>& blocksForVehicle = vehicleBlocks[vIdx];

        Bin3D bin(vehicle->getLength(), vehicle->getWidth(), vehicle->getHeight());
        vector<int> orientationsForVehicle = chromosome.getAssignedBoxOrientations(vIdx, deliveries);
        
        for (size_t i = 0; i < blocksForVehicle.size(); ++i) {
            if (!bin.tryPlaceBlock(blocksForVehicle[i], orientationsForVehicle[i])) {
                return -9999999;
            }
        }
    }

    // Evaluate fitness
    int dispatchesCount = 0;
    double totalUtilizationScore = 0.0;
    double overcapacityPenalty = 0.0;
    
    for (auto it = transportUsageById.begin(); it != transportUsageById.end(); ++it) {
        int key   = it->first;
        int value = it->second;
        dispatchesCount += 1;
    }

    for (int vIdx : usedVehicles) {
        TransportUnit* vehicle = vehicles[vIdx];

        double maxVolume = vehicle->getVolume();
        double maxWeight = vehicle->getMaxWeight();
        double minWeight = vehicle->getMinWeight();

        double usedVolume = vehicleVolumeUsed[vIdx];
        double usedWeight = vehicleWeightUsed[vIdx];

        totalUtilizationScore += usedVolume / maxVolume;

        if (usedWeight > maxWeight) {
            overcapacityPenalty += (usedWeight - maxWeight);  // Penalize overweight
        } else if (usedWeight < minWeight) {
            overcapacityPenalty += (minWeight - usedWeight);  // Penalize minweight
        }
    }

    double avgUtilization = numDeliveriesAssigned == 0 ? 0.0 : totalUtilizationScore / numDeliveriesAssigned;
    double fulfillmentRatio = deliveries.empty() ? 0.0 : (double)numDeliveriesAssigned / deliveries.size();
    double priorityCoverage = totalPriority == 0 ? 0.0 : (double)attendedPriority / totalPriority;
    
    return DispatchUtils::getObjectiveFunction(
        dispatchesCount, 
        avgUtilization, 
        fulfillmentRatio, 
        priorityCoverage, 
        overcapacityPenalty
    );
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

    const double deliveryMutationRate = 0.4;
    const double orientationMutationRate = 0.4;

    // Mutate delivery assignments
    for (size_t i = 0; i < deliveries.size(); ++i) {
        if ((rand() / double(RAND_MAX)) < deliveryMutationRate) {
            int newVehicle = (rand() % (vehicles.size() + 1)) - 1;
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