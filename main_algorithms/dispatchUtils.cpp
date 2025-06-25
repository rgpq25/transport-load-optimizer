#include "dispatchUtils.h"
#include "bin3D.h"
#include <map>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace DispatchUtils {
    double getObjectiveFunction(
        double dispatchesCount,
        double avgUtilization,
        double fulfillmentRatio,
        double priorityCoverage,
        double overcapacityPenalty
    ) {
        double A = -1.0;      // minimize dispatch count
        double B = 5.0;         // maximize volume utilization
        double C = 1.0;         // maximize fulfilling deliveries
        double D = 1.0;         // maximize priority coverage
        double E = -100.0;      // penalty factor

        return A * dispatchesCount +
            B * avgUtilization + 
            C * fulfillmentRatio +
            D * priorityCoverage +
            E * overcapacityPenalty;
    };
    
    vector<Dispatch> buildFromChromosome(
        const Chromosome& chromosome,
        const vector<Delivery*>& deliveries,
        const vector<TransportUnit*>& vehicles,
        Route* route,
        const TimeSlot& slot,
        const string& date
    ) {
        vector<Dispatch> result;
        const vector<int>& assignments = chromosome.getDeliveryAssignments();

        map<int, vector<Delivery*>> deliveriesByVehicle;
        map<int, vector<Block*>> blocksByVehicle;

        for (int i = 0; i < assignments.size(); ++i) {
            int vehicleIdx = assignments[i];
            if (vehicleIdx < 0 || vehicleIdx >= vehicles.size()) continue;

            Delivery* d = deliveries[i];
            deliveriesByVehicle[vehicleIdx].push_back(d);

            const vector<Block*>& bs = d->getBlocksToDeliver();
            blocksByVehicle[vehicleIdx].insert(blocksByVehicle[vehicleIdx].end(), bs.begin(), bs.end());
        }

        for (const auto& entry : deliveriesByVehicle) {
            int vIdx = entry.first;
            TransportUnit* truck = vehicles[vIdx];
            
            vector<int> blockOrientations = chromosome.getAssignedBoxOrientations(vIdx, deliveries);
            
            const vector<Block*>& vehicleBlocks = blocksByVehicle[vIdx];

            // Run Bin3D placement again to retrieve positions
            Bin3D bin(truck->getLength(), truck->getWidth(), truck->getHeight());
            vector<BlockPlacement> placements;
            bool feasible = true;

            for (size_t i = 0; i < vehicleBlocks.size(); ++i) {
                if (!bin.tryPlaceBlock(vehicleBlocks[i], blockOrientations[i])) {
                    feasible = false;
                    break;
                }
            }

            if (!feasible) continue;

            // Retrieve and convert placed blocks
            const vector<PlacedBox>& placed = bin.getPlacedBoxes();
            for (const PlacedBox& pb : placed) {
                BlockPlacement bp;
                bp.blockId = pb.block->getId();
                bp.orientation = pb.orientationIndex;
                bp.x = pb.position[0];
                bp.y = pb.position[1];
                bp.z = pb.position[2];
                bp.lx = pb.size[0];
                bp.ly = pb.size[1];
                bp.lz = pb.size[2];
                placements.push_back(bp);
            }
            
            Dispatch dispatch(
                truck,
                route,
                slot,
                date,
                deliveriesByVehicle[vIdx],
                blocksByVehicle[vIdx],
                blockOrientations,
                placements
            );
            
            result.push_back(dispatch);
        }
        
        
        for (const auto& entry : deliveriesByVehicle) {
            int vIdx = entry.first;
            const auto& ds = deliveriesByVehicle[vIdx];
            const auto& bs = blocksByVehicle[vIdx];
        }

        return result;
    }
    
    vector<Dispatch> buildFromPattern(
        const VehiclePattern& pattern,
        const vector<Delivery*>& allDeliveries,
        const vector<TransportUnit*>& allVehicles,
        Route* route,
        const TimeSlot& slot,
        const string& date
    ) {
        vector<Dispatch> result;
        TransportUnit* truck = pattern.vehicle;

        // 1. Recolectar deliveries y bloques del pattern
        vector<Delivery*> patDeliveries;
        vector<Block*>     patBlocks;
        vector<int>        patOrientations;
        for (const auto& layer : pattern.layers) {
            patDeliveries.push_back(layer.delivery);
            for (size_t i = 0; i < layer.blocks.size(); ++i) {
                patBlocks.push_back(layer.blocks[i]);
                patOrientations.push_back(layer.orientations[i]);
            }
        }

        // 2. Volver a ejecutar Bin3D para obtener posiciones
        Bin3D bin(truck->getLength(), truck->getWidth(), truck->getHeight());
        bool feasible = true;
        for (size_t i = 0; i < patBlocks.size(); ++i) {
            if (!bin.tryPlaceBlock(patBlocks[i], patOrientations[i])) {
                feasible = false;
                break;
            }
        }
        if (!feasible) {
            // No factible según el packer 3D, devolvemos vacío
            return result;
        }

        // 3. Extraer placements de Bin3D
        std::vector<BlockPlacement> placements;
        for (const auto& pb : bin.getPlacedBoxes()) {
            BlockPlacement bp;
            bp.blockId     = pb.block->getId();
            bp.orientation = pb.orientationIndex;
            bp.x           = pb.position[0];
            bp.y           = pb.position[1];
            bp.z           = pb.position[2];
            bp.lx          = pb.size[0];
            bp.ly          = pb.size[1];
            bp.lz          = pb.size[2];
            placements.push_back(bp);
        }

        // 4. Crear el Dispatch y devolverlo
        Dispatch dispatch(
            truck,
            route,
            slot,
            date,
            patDeliveries,
            patBlocks,
            patOrientations,
            placements
        );
        result.push_back(dispatch);
        return result;
    };
    
    void exportDispatchesToCSV(const vector<Dispatch>& dispatches, const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening output CSV file." << endl;
            return;
        }

        // Header
        file << "dispatch_id,date,slot_start,slot_end,"
            << "route_id,route_string,"
            << "truck_id,truck_l,truck_w,truck_h,"
            << "delivery_id,block_id,fragility,orientation,x,y,z,lx,ly,lz\n";

        int counter = 1;
        for (int i = 0; i < dispatches.size(); ++i) {
            const Dispatch& d = dispatches[i];
            TransportUnit* truck = d.getTruck();
            string date = d.getDate();
            string start = d.getTimeSlot().getStartAsString();
            string end = d.getTimeSlot().getEndAsString();
            
            vector<Delivery*> currentDeliveries = d.getDeliveries();
            
            unordered_map<int, const BlockPlacement*> blockPlacementsById;
            blockPlacementsById.reserve(d.getBlockPlacements().size());
            for (const BlockPlacement& bp : d.getBlockPlacements()) {
                int id = bp.blockId;
                blockPlacementsById[id] = &bp;
            }
            
            for (const Delivery* currDelivery : currentDeliveries) {
                for (const Block* currBlock : currDelivery->getBlocksToDeliver()) {
                    auto it = blockPlacementsById.find(currBlock->getId());
                    if (it != blockPlacementsById.end()) {
                        const BlockPlacement& bp = *it->second;
                        
                        file << counter << "," << date << "," << start << "," << end << ","
                            << d.getRoute()->getId() << "," << d.getRoute()->getRouteAsString() << ","
                            << truck->getId() << "," << truck->getLength() << "," << truck->getWidth() << "," << truck->getHeight() << ","
                            << currDelivery->getId() << "," << bp.blockId << "," << currBlock->getFragility() << "," << bp.orientation << ","
                            << bp.x << "," << bp.y << "," << bp.z << ","
                            << bp.lx << "," << bp.ly << "," << bp.lz << "\n";
                        
                    } else {
                        cout << "[ERROR] - Couldnt find the block placement of a certain delivery";
                    }
                }
            }

            /*
            for (const auto& bp : d.getBlockPlacements()) {
                file << counter << "," << date << "," << start << "," << end << ","
                    << truck->getId() << "," << truck->getLength() << "," << truck->getWidth() << "," << truck->getHeight() << ","
                    << d. << "," << bp.blockId << "," << bp.orientation << ","
                    << bp.x << "," << bp.y << "," << bp.z << ","
                    << bp.lx << "," << bp.ly << "," << bp.lz << "\n";
            }
            */
            
            counter += 1;
        }

        file.close();
        cout << "[CSV] Exported " << dispatches.size() << " dispatches to " << filename << endl;
    }

    double evaluateDispatchesFitness(
        const vector<Dispatch>& dispatches,
        const vector<Delivery*>& allDeliveries,
        const vector<TransportUnit*>& allTransportUnits
    ) {
        // 1) Prioridad total
        int totalPriority = 0;
        for (auto* d : allDeliveries) {
            totalPriority += d->getPriority();
        }

        // 2) Contar deliveries atendidos y prioridad cubierta
        int numDeliveriesAssigned = 0;
        int attendedPriority    = 0;

        for (const auto& disp : dispatches) {
            for (Delivery* d : disp.getDeliveries()) {
                numDeliveriesAssigned++;
                attendedPriority += d->getPriority();
            }
        }

        // 3) Calcular scores y penalizaciones
        double totalUtilizationScore = 0.0;
        double overcapacityPenalty = 0.0;

        for (const auto& dispatch : dispatches) {
            TransportUnit* vehicle = dispatch.getTruck();
            
            double maxVolume = vehicle->getVolume();
            double maxWeight = vehicle->getMaxWeight();
            double minWeight = vehicle->getMinWeight();
            
            double usedVolume = 0;
            double usedWeight = 0;
            for (auto& block: dispatch.getBlocks()) {
                usedVolume += block->getVolume();
                usedWeight += block->getWeight();
            }

            totalUtilizationScore += usedVolume / maxVolume;
            
            if (usedWeight > maxWeight) {
                overcapacityPenalty += (usedWeight - maxWeight);  // Penalize overweight
            } else if (usedWeight < minWeight) {
                overcapacityPenalty += (minWeight - usedWeight);  // Penalize minweight
            }
        }

        // 4) Retornar getObjectiveFunction
        double avgUtilization = numDeliveriesAssigned == 0 ? 0.0 : totalUtilizationScore / numDeliveriesAssigned;
        double fulfillmentRatio = allDeliveries.empty() ? 0.0 : (double)numDeliveriesAssigned / allDeliveries.size();
        double priorityCoverage = totalPriority == 0 ? 0.0 : (double)attendedPriority / totalPriority;
        
        return getObjectiveFunction(
            dispatches.size(),
            avgUtilization,
            fulfillmentRatio,
            priorityCoverage,
            overcapacityPenalty
        );
    }
}