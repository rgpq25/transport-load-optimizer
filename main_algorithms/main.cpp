#include <iostream>
#include <vector>
#include <string>
#include <chrono>

#include "block.h"
#include "transportUnit.h"
#include "client.h"
#include "input.h"
#include "timeSlot.h"
#include "dispatch.h"

#include "SAGAOptimizer.h"
#include "GRASPOptimizer.h"

#include "globalExecutionTracker.h"
#include "deliveryUtils.h"
#include "timeSlotUtils.h"
#include "dispatchUtils.h"

/*
 * Currently the algorithm can only asign the deliveries with "null" shifts
 * to the same day as its dueDate. To attend them earlier we can change the 
 * iteration to start from today and iterate to the max delivery date
 * in the input. And in the filterByDateAndRoute, we always return those
 * with "null" shifts. That way we can start attending deliveries with no
 * time window form today.
 */

/*
 TODOS:
 * - Actually check for block availability from the inputs when generating a new solution
 * - Confirm if null time window deliveries can be attended before the due date.
 * - Get a better and bigger randomized dataset
 * - Consider a max containers per day
 * - !!! When reassigning trucks, the random number COULD BE 0 (UNDEFINED TRUCK)
 */

using namespace std;

void printLog(const string& log, bool debug) {
    if (debug == true) cout << log << endl;
}


bool isDebugMatch(const string& dueDate, int routeId, const string& shift) {
    return false;
    bool isMatch =
        dueDate == "2024-09-28" && 
        routeId == 4 && 
        shift == "morning";
    
    return isMatch;
}

unordered_map<string, string> parseArgs(int argc, char** argv) {
    unordered_map<string, string> args;
    for (int i = 1; i < argc - 1; ++i) {
        if (string(argv[i]).rfind("--", 0) == 0) {
            string key = argv[i];
            string value = argv[i + 1];
            args[key.substr(2)] = value;
            ++i;  // skip value
        }
    }
    return args;
}

vector<double> parseAlphaSet(const string& input) {
    vector<double> result;
    stringstream ss(input);
    string token;
    while (getline(ss, token, ',')) {
        result.push_back(stod(token));
    }
    return result;
}


int main(int argc, char** argv) {
    std::srand(std::time(nullptr));

    auto args = parseArgs(argc, argv);
    bool debug = false;
    
    string algorithmToRun = "GRASP";
    
    string repositoryPath = "C:/main/university/semesters/CICLO XII/TESIS2/transport-load-optimizer";
    string inputPath = repositoryPath + "/input/input_large.txt";
    string outputDispatchesPath = repositoryPath + "/output/output_dispatches.csv";
    string outputMetadataPath = repositoryPath + "/output/output_result_metadata.csv";
   
    // SAGA Params
    int populationSize = 70;
    double mutationRate = 0.1;
    int T_init = 50;
    int T_min = 1;
    double alpha = 0.95;
    
    // GRASP Params
    int graspIterations = 100;
    double Kpercent = 70;
    vector<double> alphaSet = {0.1, 0.5, 0.9};
    
    int unloadingTime = 20;
    int timeSlotInterval = 30;
    
    // Parameter configuration with execution arguments
    ///*
    algorithmToRun = args["algo"];
    inputPath = args["input"];
    if (algorithmToRun == "SA-GA") {
        if (
            args.count("population") +
            args.count("mutation_rate") +
            args.count("t_init") + 
            args.count("t_min") + 
            args.count("alpha") != 5
        ) throw runtime_error("Missing parameters for SA-GA algorithm.");
        
        populationSize = stoi(args["population"]);
        mutationRate = stod(args["mutation_rate"]);
        T_init = stoi(args["t_init"]);
        T_min= stoi(args["t_min"]);
        alpha = stod(args["alpha"]);
        
        cout << "[INIT] - Params set for SA-GA: " << 
                "populationSize = " << populationSize << " | " <<
                "mutationRate = " << mutationRate << " | " <<
                "T_init = " << T_init << " | " <<
                "T_min = " << T_min << " | " <<
                "alpha = " << alpha << endl;
    } else if (algorithmToRun == "GRASP") {
        if (
            args.count("iterations") + 
            args.count("k_percent") + 
            args.count("alphas") != 3
        ) throw runtime_error("Missing parameters for GRASP algorithm.");
        
        graspIterations = stoi(args["iterations"]);
        Kpercent = stod(args["k_percent"]);
        alphaSet = parseAlphaSet(args["alphas"]);
        
        cout << "[INIT] - Params set for GRASP: " << 
                "graspIterations = " << graspIterations << " | " <<
                "Kpercent = " << Kpercent << " | " <<
                "alphaSet = " << args["alphas"] << endl;
    } else {
        throw runtime_error("Missing --algo parameter.");
    }
    //*/
    
    Input input;
    input.loadFromFile(inputPath);
    input.printInputData();
        
    cout << endl << "=========MAIN PROGRAM =========" << endl << endl;
    
    GlobalExecutionTracker tracker;
    vector<Dispatch> finalDispatches;
    vector<string> uniqueDueDates = input.getUniqueDueDates();
    vector<Delivery*> deliveries = input.getAllDeliveriesFromOrders();
    vector<TransportUnit*> allVehicles;
    for (TransportUnit& unit : input.getTransportUnits()) {
        allVehicles.push_back(&unit);
    }
    
    // Filter 1: By date
    auto start = chrono::high_resolution_clock::now();
    for(const string& dueDate : uniqueDueDates) {
        printLog("DUE DATE = " + dueDate, debug);
        
        // Filter 2: By route
        for(const Route& route : input.getRoutes()) {
            printLog("    ROUTE ID = " + to_string(route.getId()), debug);
            vector<Delivery*> deliveriesForRoute = DeliveryUtils::filterByDateAndRoute(
                deliveries, 
                dueDate, 
                route
            );
            
            // Filter 3: By shift
            vector<string> shifts = {"morning", "afternoon"};
            for (const string& shift : shifts) {
                printLog("       SHIFT = " + shift, debug);
                vector<Delivery*> deliveriesByShift = DeliveryUtils::filterByShift(
                    deliveriesForRoute, 
                    shift
                );
                
                if (deliveriesByShift.empty()) {
                    printLog("            There were no deliveries for this shift.", debug);
                    continue;
                }

                vector<TimeSlot> timeSlots;
                if (shift == "morning") {
                    timeSlots = TimeSlotUtils::generateForWindow(
                        480, 
                        720, 
                        route, 
                        unloadingTime, 
                        timeSlotInterval
                    ); // 08:00 – 12:00
                } else if (shift == "afternoon") {
                    timeSlots = TimeSlotUtils::generateForWindow(
                        840, 
                        1080, 
                        route, 
                        unloadingTime, 
                        timeSlotInterval
                    ); // 14:00 – 18:00
                }
                
                // Filter 4: By timeslot
                for (const TimeSlot& ts : timeSlots) {
                    // 1. Filter deliveries by tracker
                    vector<Delivery*> deliveryPtrs;
                    for (Delivery* d : deliveriesByShift) {
                        if (!tracker.isDeliveryFulfilled(d->getId())) {
                            deliveryPtrs.push_back(d);
                        }
                    }
                    if (deliveryPtrs.empty()) continue;

                    
                    // 2. Filter blocks (only those not already used)
                    vector<Delivery*> finalDeliveries;
                    vector<Block*> blocksForThisBatch;

                    for (Delivery* d : deliveryPtrs) {
                        const vector<Block*>& bs = d->getBlocksToDeliver();
                        bool allBlocksAvailable = true;
                        int idOfAlreadyUsedBlock = -1;
                        
                        for (Block* b : bs) {
                            if (tracker.isBlockUsed(b->getId())) {
                                allBlocksAvailable = false;
                                idOfAlreadyUsedBlock = b->getId();
                                break;
                            }
                        }

                        if (allBlocksAvailable) {
                            finalDeliveries.push_back(d);
                            blocksForThisBatch.insert(blocksForThisBatch.end(), bs.begin(), bs.end());
                        }
                    }
                    if (finalDeliveries.empty() || blocksForThisBatch.empty()) continue;

                    // 3. Filter available vehicles for this time slot
                    vector<TransportUnit*> availableVehicles = tracker.getAvailableVehicles(allVehicles, ts, dueDate);
                    if (availableVehicles.empty()) continue;
                    
                    
                    // [DEBUG]
                    bool matchesSelected = isDebugMatch(dueDate, route.getId(), shift);
                    
                    if (matchesSelected == true) {
                        cout << "[DEBUG] - Check deliveries" << endl;
                        for (Delivery* del : finalDeliveries) {
                            cout << del->getId() << endl;
                        }
                        
                        cout << "Check available trucks" << endl;
                        for (TransportUnit* tu : availableVehicles) {
                            cout << tu->getId() << endl;
                        }
                        
                        cout << "FINISHED PRINT ===================" << endl;
                    }
                    
                    if (algorithmToRun == "SA-GA") {
                        SAGAOptimizer optimizer(
                            finalDeliveries,
                            blocksForThisBatch,
                            availableVehicles,
                            const_cast<Route*>(&route),
                            ts,
                            populationSize,
                            mutationRate,
                            T_init, 
                            T_min, 
                            alpha
                        );

                        Chromosome best = optimizer.run(matchesSelected);
                        
                        vector<Dispatch> dispatches = DispatchUtils::buildFromChromosome(
                            best, 
                            finalDeliveries, 
                            availableVehicles, 
                            const_cast<Route*>(&route), 
                            ts, 
                            dueDate
                        );
                        tracker.recordDispatchSolution(dispatches);
                        finalDispatches.insert(finalDispatches.end(), dispatches.begin(), dispatches.end());
                    } 
                    else if (algorithmToRun == "GRASP") {
                        GRASPOptimizer optimizer(
                            finalDeliveries,
                            blocksForThisBatch,
                            availableVehicles,
                            const_cast<Route*>(&route),
                            ts,
                            dueDate,
                            graspIterations, 
                            Kpercent, 
                            alphaSet
                        );

                        auto patterns = optimizer.run(matchesSelected);

                        for (const auto& pat : patterns) {
                            auto dispatches = DispatchUtils::buildFromPattern(
                                pat,
                                finalDeliveries,
                                availableVehicles,
                                const_cast<Route*>(&route),
                                ts,
                                dueDate
                            );

                            tracker.recordDispatchSolution(dispatches);
                            finalDispatches.insert(
                                finalDispatches.end(),
                                dispatches.begin(),
                                dispatches.end()
                            );
                        }
                    }
                }
            }
        }
    }
    auto end = chrono::high_resolution_clock::now();  // Fin de medición de tiempo
    chrono::duration<double> algorithmTotalDuration = end - start;
    
    
    cout << endl << "=========== FINAL DISPATCH PLAN (" 
            << finalDispatches.size() 
            << ") ===========" << endl;
    int disCounter = 0;
    for (const Dispatch& d : finalDispatches) {
        disCounter = disCounter + 1;
        
        bool matchesSelected = d.getDate() == "2024-09-28" && 
                               d.getRoute()->getId() == 4 &&
                               d.getTimeSlot().getStart() < 840;        
        //if (matchesSelected == false) continue;
        
        cout << "Dispatch ID: " << disCounter << endl;
        d.printSummary();
    }
    
    cout << endl << "======== FITNESS OF SOLUTION ==========" << endl;
    double solutionFitness = DispatchUtils::evaluateDispatchesFitness(finalDispatches, deliveries, allVehicles);
    cout << solutionFitness << endl;
    
    DispatchUtils::exportDispatchesToCSV(
        finalDispatches, 
        outputDispatchesPath
    );
    
    DispatchUtils::exportResultMetadata(
        solutionFitness,
        algorithmTotalDuration.count(),
        outputMetadataPath
    );
    
    
    cout << endl << "=========== DELIVERIES NOT ATTENDED ===========" 
            << endl;
    vector<int> unfulfilled = tracker.getUnfulfilledDeliveryIds(deliveries);
    if (unfulfilled.empty()) {
        cout << " All deliveries were dispatched." << endl;
    } else {
        for (int id : unfulfilled) {
            cout << "Delivery not dispatched → ID: " << id << endl;
        }
    }
    
    return 0;
}

