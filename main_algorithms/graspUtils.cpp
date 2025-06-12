#include "graspUtils.h"
#include <algorithm>
#include <cstdlib>

using namespace std;

namespace GraspUtils {
    std::vector<LayerCandidate> generateCandidateLayers(
        const std::vector<const Block*>& blocks,
        double maxLength,
        double maxWidth
    ) {
        std::vector<LayerCandidate> candidates;

        for (size_t i = 0; i < blocks.size(); ++i) {
            const Block* b1 = blocks[i];

            for (int o1 = 0; o1 <= 1; ++o1) {
                double l1 = (o1 == 0) ? b1->getLength() : b1->getWidth();
                double w1 = (o1 == 0) ? b1->getWidth() : b1->getLength();

                if (l1 > maxLength || w1 > maxWidth) continue;

                // Capa de un solo bloque
                LayerCandidate single;
                single.addBlock(b1, o1, 0.0, 0.0);
                single.setScore(l1 * w1);  // volumen base
                candidates.push_back(single);

                // Combinaciones con otros bloques
                for (size_t j = i + 1; j < blocks.size(); ++j) {
                    const Block* b2 = blocks[j];

                    for (int o2 = 0; o2 <= 1; ++o2) {
                        double l2 = (o2 == 0) ? b2->getLength() : b2->getWidth();
                        double w2 = (o2 == 0) ? b2->getWidth() : b2->getLength();

                        // Intentamos colocarlos uno al lado del otro
                        double totalL = l1 + l2;
                        double maxW = std::max(w1, w2);

                        if (totalL <= maxLength && maxW <= maxWidth) {
                            LayerCandidate layer;
                            layer.addBlock(b1, o1, 0.0, 0.0);
                            layer.addBlock(b2, o2, l1, 0.0);

                            double score = (l1 * w1) + (l2 * w2);
                            layer.setScore(score);
                            candidates.push_back(layer);
                        }
                    }
                }
            }
        }

        return candidates;
    }

    std::vector<LayerCandidate> selectFromRCL(
        const std::vector<LayerCandidate>& candidates,
        double alpha
    ) {
        std::vector<LayerCandidate> rcl;

        if (candidates.empty()) return rcl;

        double minScore = candidates[0].getScore();
        double maxScore = candidates[0].getScore();

        for (size_t i = 1; i < candidates.size(); ++i) {
            double s = candidates[i].getScore();
            if (s < minScore) minScore = s;
            if (s > maxScore) maxScore = s;
        }

        double threshold = maxScore - alpha * (maxScore - minScore);

        for (size_t i = 0; i < candidates.size(); ++i) {
            if (candidates[i].getScore() >= threshold) {
                rcl.push_back(candidates[i]);
            }
        }

        return rcl;
    }

}
