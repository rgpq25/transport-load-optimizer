#ifndef GRASPOPTIMIZER_H
#define GRASPOPTIMIZER_H

#include "delivery.h"
#include "block.h"
#include "transportUnit.h"
#include "route.h"
#include "timeSlot.h"
#include <string>
#include <vector>

#include "alphaReactive.h"
#include "vehiclePattern.h"
#include "maximalSpace.h"
#include "layerCandidate.h"

class GRASPOptimizer {
public:
    GRASPOptimizer(
        const vector<Delivery*>& deliveries,
        const vector<Block*>& blocks,
        const vector<TransportUnit*>& vehicles,
        Route* route,
        const TimeSlot& slot,
        const string& date,
        int graspIterations,
        double Kpercent,
        const vector<double>& alphaSet
    );

    // Ejecuta todo el Algorithm 4 y devuelve patrones para cada vehículo
    vector<VehiclePattern> run();

private:
    // Datos de entrada y parámetros
    vector<Delivery*>        LP;
    vector<Block*>           allBlocks;
    vector<TransportUnit*>   V;
    Route*                        route;
    TimeSlot                      slot;
    string                   date;
    int                           maxIter;
    double                        K;
    AlphaReactive                 alphaTuner;

    // Métodos internos
    vector<MaximalSpace>        generateInitialSpaces(TransportUnit* truck);
    vector<LayerCandidate>      generateLayers(const MaximalSpace& e, Delivery* d);
    vector<LayerCandidate>      buildRCL(const vector<LayerCandidate>& layers, double alpha);
    LayerCandidate              selectBestFit(const vector<LayerCandidate>& layers);
    void                        locateLayer(const LayerCandidate& sp, MaximalSpace& e, vector<MaximalSpace>& spacesOut);
    vector<LayerCandidate>      removeKPercent(VehiclePattern& pattern);
};

#endif /* GRASPOPTIMIZER_H */

