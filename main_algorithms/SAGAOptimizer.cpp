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
    int populationSize,
    double mutationRate,
    int T_init,
    int T_min,
    double alpha
) {
    this->deliveries = deliveries;
    this->blocks = blocks;
    this->vehicles = vehicles;
    this->route = route;
    this->timeSlot = timeSlot;
    this->populationSize = populationSize;
    this->mutationRate = mutationRate;
    this->T_init = T_init;
    this->T_min = T_min;
    this->alpha = alpha;
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

    // [TRACE] Initial population (generation 0)
    bool trace = !traceDir.empty();
    ofstream traceCrossovers, traceGenerations;
    if (trace) {
        ofstream tracePopulation;
        TraceUtils::openTraceFile(tracePopulation, traceDir, "saga_population.csv",
            "run_id,chromosome,assignments,orientations,fitness," + TraceUtils::breakdownHeader());
        double sumInitial = 0.0;
        for (size_t idx = 0; idx < population.size(); ++idx) {
            Chromosome copy = population[idx];
            FitnessBreakdown bd;
            evaluateFitness(copy, &bd);
            sumInitial += population[idx].getFitness();
            tracePopulation << runId << "," << idx << ","
                            << assignmentsToString(population[idx]) << ","
                            << orientationsToString(population[idx]) << ","
                            << population[idx].getFitness() << ","
                            << TraceUtils::breakdownToCSV(bd) << "\n";
        }
        tracePopulation.close();

        TraceUtils::openTraceFile(traceGenerations, traceDir, "saga_generations.csv",
            "run_id,generation,temperature,avg_fitness,best_fitness_population,best_fitness_global,duration_s");
        traceGenerations << runId << ",0,," << sumInitial / population.size() << ","
                         << best.getFitness() << "," << best.getFitness() << ",\n";

        TraceUtils::openTraceFile(traceCrossovers, traceDir, "saga_crossovers.csv",
            "run_id,generation,temperature,crossover,parent1_index,parent1_fitness,parent2_index,parent2_fitness,"
            "child_assignments_before_mutation,child_orientations_before_mutation,"
            "child_assignments,child_orientations,mutated_genes,child_fitness," + TraceUtils::breakdownHeader() + ","
            "delta,acceptance_probability,random_value,decision,fitness_in_new_population");
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
            int parent1Index = -1, parent2Index = -1;   // [TRACE]
            Chromosome parent1 = selectParent(population, trace ? &parent1Index : nullptr);
            Chromosome parent2 = selectParent(population, trace ? &parent2Index : nullptr);

            Chromosome child = crossover(parent1, parent2);
            Chromosome childBeforeMutation;             // [TRACE]
            if (trace) childBeforeMutation = child;
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
            double traceProb = -1.0, traceRand = -1.0;  // [TRACE]
            string traceDecision;                       // [TRACE]

            if (delta < 0) {
                newPopulation.push_back(child);
                sumFitness += child.getFitness();
                traceDecision = "child_improves";
            } else {
                double prob = exp(-delta / T);
                double randVal = (double)rand() / RAND_MAX;
                traceProb = prob;
                traceRand = randVal;

                if (randVal < prob) {
                    newPopulation.push_back(child);
                    sumFitness += child.getFitness();
                    traceDecision = "child_accepted_by_sa";
                } else {
                    newPopulation.push_back(parent1);
                    sumFitness += parent1.getFitness();
                    traceDecision = "parent1_kept";
                }
            }

            // [TRACE] Detail of this crossover
            if (trace) {
                int mutatedGenes = 0;
                const vector<int>& aBefore = childBeforeMutation.getDeliveryAssignments();
                const vector<int>& aAfter = child.getDeliveryAssignments();
                const vector<int>& oBefore = childBeforeMutation.getBoxOrientations();
                const vector<int>& oAfter = child.getBoxOrientations();
                for (size_t g = 0; g < aAfter.size(); ++g) if (aBefore[g] != aAfter[g]) mutatedGenes++;
                for (size_t g = 0; g < oAfter.size(); ++g) if (oBefore[g] != oAfter[g]) mutatedGenes++;

                Chromosome copy = child;
                FitnessBreakdown bd;
                evaluateFitness(copy, &bd);

                traceCrossovers << runId << "," << iterationIndex + 1 << "," << T << "," << i << ","
                                << parent1Index << "," << parent1.getFitness() << ","
                                << parent2Index << "," << parent2.getFitness() << ","
                                << assignmentsToString(childBeforeMutation) << ","
                                << orientationsToString(childBeforeMutation) << ","
                                << assignmentsToString(child) << ","
                                << orientationsToString(child) << ","
                                << mutatedGenes << "," << childFitness << ","
                                << TraceUtils::breakdownToCSV(bd) << ","
                                << delta << ",";
                if (traceProb >= 0) traceCrossovers << traceProb << "," << traceRand;
                else traceCrossovers << ",";
                traceCrossovers << "," << traceDecision << "," << newPopulation.back().getFitness() << "\n";
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

        // [TRACE] Summary of every generation
        if (trace) {
            traceGenerations << runId << "," << iterationIndex + 1 << "," << T << ","
                             << sumFitness / populationSize << ","
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

    // [TRACE] Best chromosome of the run
    if (trace) {
        traceGenerations.close();
        traceCrossovers.close();

        ofstream traceBest;
        TraceUtils::openTraceFile(traceBest, traceDir, "saga_best.csv",
            "run_id,assignments,orientations,fitness," + TraceUtils::breakdownHeader());
        Chromosome copy = best;
        FitnessBreakdown bd;
        evaluateFitness(copy, &bd);
        traceBest << runId << "," << assignmentsToString(best) << "," << orientationsToString(best) << ","
                  << best.getFitness() << "," << TraceUtils::breakdownToCSV(bd) << "\n";
        traceBest.close();
    }

    return best;
}


void SAGAOptimizer::setTrace(const string& traceDir, const string& runId) {
    this->traceDir = traceDir;
    this->runId = runId;
}


void SAGAOptimizer::disableReseed() {
    this->reseed = false;
}


// "E<id entrega>:UT<id unidad>" separated by spaces; "-" when the delivery is not assigned
string SAGAOptimizer::assignmentsToString(const Chromosome& c) const {
    const vector<int>& assignments = c.getDeliveryAssignments();
    ostringstream ss;
    for (size_t i = 0; i < assignments.size(); ++i) {
        if (i > 0) ss << " ";
        ss << "E" << deliveries[i]->getId() << ":";
        if (assignments[i] < 0 || assignments[i] >= (int)vehicles.size()) ss << "-";
        else ss << "UT" << vehicles[assignments[i]]->getId();
    }
    return ss.str();
}


// One digit (0/1) per block, in the order of the blocks of the subproblem
string SAGAOptimizer::orientationsToString(const Chromosome& c) const {
    ostringstream ss;
    for (int o : c.getBoxOrientations()) ss << o;
    return ss.str();
}




vector<Chromosome> SAGAOptimizer::generateInitialPopulation() {
    vector<Chromosome> population;
    int numDeliveries = deliveries.size();
    int numBlocks = blocks.size();
    int numVehicles = vehicles.size();

    if (reseed) srand(time(nullptr)); // Seed RNG once (skipped when a fixed seed is used)

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




double SAGAOptimizer::evaluateFitness(Chromosome& chromosome, FitnessBreakdown* breakdown) {
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
                if (breakdown != nullptr) breakdown->feasible3D = false;
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

    double avgUtilization = numDeliveriesAssigned == 0 ? 0.0 : totalUtilizationScore * 100 / numDeliveriesAssigned;
    double fulfillmentRatio = deliveries.empty() ? 0.0 : (double)numDeliveriesAssigned * 100 / deliveries.size();
    double priorityCoverage = totalPriority == 0 ? 0.0 : (double)attendedPriority * 100 / totalPriority;

    if (breakdown != nullptr) {
        breakdown->dispatchesCount = dispatchesCount;
        breakdown->avgUtilization = avgUtilization;
        breakdown->fulfillmentRatio = fulfillmentRatio;
        breakdown->priorityCoverage = priorityCoverage;
        breakdown->overcapacityPenalty = overcapacityPenalty;
        ostringstream ss;
        for (size_t v = 0; v < vehicles.size(); ++v) {
            if (!usedVehicles.count(v)) continue;
            if (!ss.str().empty()) ss << ";";
            ss << "UT" << vehicles[v]->getId() << "=" << vehicleWeightUsed[v] << "/" << vehicles[v]->getMaxWeight();
        }
        breakdown->weightByVehicle = ss.str();
    }

    return DispatchUtils::getObjectiveFunction(
        dispatchesCount, 
        avgUtilization, 
        fulfillmentRatio, 
        priorityCoverage, 
        overcapacityPenalty
    );
}



Chromosome SAGAOptimizer::selectParent(const vector<Chromosome>& population, int* chosenIndex) {
    int tournamentSize = 3;
    int bestIndex = rand() % population.size();
    Chromosome best = population[bestIndex];

    for (int i = 1; i < tournamentSize; ++i) {
        int challengerIndex = rand() % population.size();
        Chromosome challenger = population[challengerIndex];
        if (challenger.getFitness() > best.getFitness()) {
            best = challenger;
            bestIndex = challengerIndex;
        }
    }

    if (chosenIndex != nullptr) *chosenIndex = bestIndex;
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

    // Mutate delivery assignments
    for (size_t i = 0; i < deliveries.size(); ++i) {
        if ((rand() / double(RAND_MAX)) < this->mutationRate) {
            int newVehicle = (rand() % (vehicles.size() + 1)) - 1;
            deliveries[i] = newVehicle;
        }
    }

    // Mutate block orientations
    for (size_t i = 0; i < orientations.size(); ++i) {
        if ((rand() / double(RAND_MAX)) < this->mutationRate) {
            orientations[i] = 1 - orientations[i]; // Flip 0 ↔ 1
        }
    }
}