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

        // Iterar por cada bloque para iniciar la construcción de una capa
        for (size_t i = 0; i < blocks.size(); ++i) {
            const Block* b1 = blocks[i];

            for (int o1 = 0; o1 <= 1; ++o1) {
                double l1 = (o1 == 0) ? b1->getLength() : b1->getWidth();
                double w1 = (o1 == 0) ? b1->getWidth() : b1->getLength();
                double h1 = b1->getHeight();

                if (l1 > maxLength || w1 > maxWidth) continue;

                LayerCandidate layer;
                layer.addBlock(b1, o1, 0.0, 0.0);
                double currentX = l1;
                double maxY = w1;
                double layerHeight = h1;
                bool valid = true;

                // Intentar agregar más bloques consecutivos a la derecha (eje X)
                for (size_t j = 0; j < blocks.size(); ++j) {
                    if (j == i) continue;
                    const Block* b2 = blocks[j];

                    for (int o2 = 0; o2 <= 1; ++o2) {
                        double l2 = (o2 == 0) ? b2->getLength() : b2->getWidth();
                        double w2 = (o2 == 0) ? b2->getWidth() : b2->getLength();
                        double h2 = b2->getHeight();

                        // Restricción de espacio
                        if (currentX + l2 > maxLength || w2 > maxWidth) continue;

                        // Fragilidad: evitar poner más frágiles debajo
                        if (b2->getFragility() > b1->getFragility()) continue;

                        // Añadir bloque a la capa
                        layer.addBlock(b2, o2, currentX, 0.0);
                        currentX += l2;
                        maxY = std::max(maxY, w2);
                        layerHeight = std::max(layerHeight, h2);

                        break;  // Solo intentamos una orientación por bloque válido
                    }
                }

                layer.setHeight(layerHeight);
                layer.setScore(currentX * maxY); // Volumen base como score
                candidates.push_back(layer);
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
