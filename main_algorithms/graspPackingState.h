#ifndef GRASPPACKINGSTATE_H
#define GRASPPACKINGSTATE_H

#include <vector>
#include <map>
#include "layerCandidate.h"
#include "block.h"

using namespace std;

class GraspPackingState {
private:
    double maxLength;
    double maxWidth;
    double maxHeight;
    double currentHeight;

    vector<LayerCandidate> placedLayers;
    vector<const Block*> remainingBlocks;

public:
    GraspPackingState(double length, double width, double height, const vector<const Block*>& blocks);

    double getCurrentHeight() const;
    double getRemainingHeight() const;

    const vector<const Block*>& getRemainingBlocks() const;
    const vector<LayerCandidate>& getPlacedLayers() const;
    
    bool hasStructuralSupport(const LayerCandidate& newLayer) const;

    bool canPlaceLayer(const LayerCandidate& layer) const;
    void placeLayer(const LayerCandidate& layer);
};

#endif /* GRASPPACKINGSTATE_H */

