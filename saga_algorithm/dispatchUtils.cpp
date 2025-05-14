#include "dispatchUtils.h"
#include <map>

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
            //VERIFY THE ABOVE LINE, IS IT WORKING CORRECTLY?
            
            Dispatch dispatch(
                truck,
                route,
                slot,
                date,
                deliveriesByVehicle[vIdx],
                blocksByVehicle[vIdx],
                blockOrientations
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
}