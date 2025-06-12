#ifndef GRASPUTILS_H
#define GRASPUTILS_H

#include <vector>
#include "block.h"
#include "layerCandidate.h"

namespace GraspUtils {
    std::vector<LayerCandidate> generateCandidateLayers(
        const std::vector<const Block*>& blocks,
        double maxLength,
        double maxWidth
    );

    std::vector<LayerCandidate> selectFromRCL(
        const std::vector<LayerCandidate>& candidates,
        double alpha
    );
}

#endif /* GRASPUTILS_H */

