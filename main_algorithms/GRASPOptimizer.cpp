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

        // 4. Verificar si se puede colocar (por altura)
        if (!state.canPlaceLayer(selected)) break;

        if (!state.hasStructuralSupport(selected)) break;
        
        // 5. Agregarla al estado
        state.placeLayer(selected);
    }

    return state;
}
