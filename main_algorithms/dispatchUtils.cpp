#include "dispatchUtils.h"
#include <map>
#include "bin3D.h"
#include <fstream>
#include <iostream>

namespace DispatchUtils {
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

            if (!feasible) continue;  // Safety guard

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
        file << "dispatch_id,truck_id,date,slot_start,slot_end,"
            << "truck_l,truck_w,truck_h,"
            << "block_id,orientation,x,y,z,lx,ly,lz\n";

        int counter = 1;
        for (int i = 0; i < dispatches.size(); ++i) {
            const Dispatch& d = dispatches[i];
            int truckId = d.getTruck()->getId();
            string date = d.getDate();
            string start = d.getTimeSlot().getStartAsString();
            string end = d.getTimeSlot().getEndAsString();

            for (const auto& bp : d.getBlockPlacements()) {
                file << counter << "," << truckId << "," << date << ","
                    << start << "," << end << ","
                    << d.getTruck()->getLength() << "," << d.getTruck()->getWidth() << "," << d.getTruck()->getHeight() << ","
                    << bp.blockId << "," << bp.orientation << ","
                    << bp.x << "," << bp.y << "," << bp.z << ","
                    << bp.lx << "," << bp.ly << "," << bp.lz << "\n";
            }
            counter += 1;
        }

        file.close();
        cout << "[CSV] Exported " << dispatches.size() << " dispatches to " << filename << endl;
    }
}