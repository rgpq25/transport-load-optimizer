#ifndef GRASPOPTIMIZER_H
#define GRASPOPTIMIZER_H

#include <vector>
#include "block.h"
#include "transportUnit.h"
#include "layerCandidate.h"
#include "graspPackingState.h"

using namespace std;

class GRASPOptimizer {
public:
    GraspPackingState constructPacking(
        const vector<const Block*>& blocks,
        const TransportUnit* truck,
        double alpha
    );
};

#endif /* GRASPOPTIMIZER_H */

