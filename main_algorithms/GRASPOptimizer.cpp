#include "GRASPOptimizer.h"
#include "graspUtils.h"  // Aquí estarán funciones auxiliares

GraspPackingState GRASPOptimizer::constructPacking(
    const vector<const Block*>& blocks,
    const TransportUnit* truck,
    double alpha
) {
    double maxL = truck->getLength();
    double maxW = truck->getWidth();
    double maxH = truck->getHeight();

    GraspPackingState state(maxL, maxW, maxH, blocks);

    while (true) {
        // 1. Generar capas candidatas válidas con los bloques restantes
        vector<LayerCandidate> candidates = GraspUtils::generateCandidateLayers(
            state.getRemainingBlocks(),
            maxL,
            maxW
        );

        if (candidates.empty()) break;

        // 2. Construir lista restringida de candidatos (RCL)
        vector<LayerCandidate> rcl = GraspUtils::selectFromRCL(candidates, alpha);
        if (rcl.empty()) break;

        // 3. Elegir una capa aleatoriamente
        int index = rand() % rcl.size();
        LayerCandidate selected = rcl[index];

        bool placed = false;

        for (size_t i = 0; i < rcl.size(); ++i) {
            int index = rand() % rcl.size();
            LayerCandidate candidate = rcl[index];

            if (!state.canPlaceLayer(candidate)) continue;
            //if (!state.hasStructuralSupport(candidate)) continue;

            state.placeLayer(candidate);
            placed = true;
            break;
        }

        if (!placed) break;
    }

    return state;
}
