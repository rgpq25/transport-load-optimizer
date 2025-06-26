#ifndef DISPATCHUTILS_H
#define DISPATCHUTILS_H

#include <vector>
#include <string>

#include "chromosome.h"
#include "vehiclePattern.h"

#include "dispatch.h"
#include "delivery.h"
#include "transportUnit.h"
#include "route.h"
#include "timeSlot.h"

namespace DispatchUtils {
    double getObjectiveFunction(
        double dispatchesCount,
        double avgUtilization,
        double fulfillmentRatio,
        double priorityCoverage,
        double overcapacityPenalty
    );
    
    vector<Dispatch> buildFromChromosome(
        const Chromosome& chromosome,
        const vector<Delivery*>& deliveries,
        const vector<TransportUnit*>& vehicles,
        Route* route,
        const TimeSlot& slot,
        const string& date
    );
    
    vector<Dispatch> buildFromPattern(
        const VehiclePattern& pattern,
        const vector<Delivery*>& allDeliveries,
        const vector<TransportUnit*>& allVehicles,
        Route* route,
        const TimeSlot& slot,
        const string& date
    );
    
    void exportDispatchesToCSV(const vector<Dispatch>& dispatches, const string& filename);
    void exportResultMetadata(double fitness, double duration, const string& filename);
    
    double evaluateDispatchesFitness(
        const vector<Dispatch>& dispatches,
        const vector<Delivery*>& allDeliveries,
        const vector<TransportUnit*>& allTransportUnits
    );
}

#endif /* DISPATCHUTILS_H */

