#include "layerCandidate.h"

LayerCandidate::LayerCandidate(){};

LayerCandidate::LayerCandidate(
    Delivery* d,
    const vector<Block*>& blks,
    const vector<int>& orients,
    double fpLen,
    double fpWid,
    double h
) : delivery(d), blocks(blks), orientations(orients),
    footprintLength(fpLen), footprintWidth(fpWid),
    layerHeight(h), volume(fpLen * fpWid * h) {}
