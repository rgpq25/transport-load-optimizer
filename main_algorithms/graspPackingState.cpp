#include "graspPackingState.h"

GraspPackingState::GraspPackingState(double length, double width, double height, const vector<const Block*>& blocks) {
    maxLength = length;
    maxWidth = width;
    maxHeight = height;
    currentHeight = 0.0;
    remainingBlocks = blocks;
}

double GraspPackingState::getCurrentHeight() const {
    return currentHeight;
}

double GraspPackingState::getRemainingHeight() const {
    return maxHeight - currentHeight;
}

const vector<const Block*>& GraspPackingState::getRemainingBlocks() const {
    return remainingBlocks;
}

const vector<LayerCandidate>& GraspPackingState::getPlacedLayers() const {
    return placedLayers;
}

bool GraspPackingState::canPlaceLayer(const LayerCandidate& layer) const {
    return currentHeight + layer.getHeight() <= maxHeight;
}

bool GraspPackingState::hasStructuralSupport(const LayerCandidate& newLayer) const {
    if (placedLayers.empty()) return true;

    const LayerCandidate& below = placedLayers.back();

    for (size_t i = 0; i < newLayer.getBlocks().size(); ++i) {
        const Block* b = newLayer.getBlocks()[i];
        pair<double, double> pos = newLayer.getPosition(b->getId());
        double lx = (newLayer.getOrientation(b->getId()) == 0) ? b->getLength() : b->getWidth();
        double ly = (newLayer.getOrientation(b->getId()) == 0) ? b->getWidth() : b->getLength();
        double centerX = pos.first + lx / 2.0;
        double centerY = pos.second + ly / 2.0;

        bool supported = false;
        for (size_t j = 0; j < below.getBlocks().size(); ++j) {
            const Block* support = below.getBlocks()[j];
            pair<double, double> spos = below.getPosition(support->getId());
            double slx = (below.getOrientation(support->getId()) == 0) ? support->getLength() : support->getWidth();
            double sly = (below.getOrientation(support->getId()) == 0) ? support->getWidth() : support->getLength();

            if (centerX >= spos.first && centerX <= spos.first + slx &&
                centerY >= spos.second && centerY <= spos.second + sly) {
                // Fragility check
                if (support->getFragility() >= b->getFragility()) {
                    supported = true;
                    break;
                }
            }
        }

        if (!supported) return false;
    }

    return true;
}

void GraspPackingState::placeLayer(const LayerCandidate& layer) {
    placedLayers.push_back(layer);
    currentHeight += layer.getHeight();

    const vector<const Block*>& layerBlocks = layer.getBlocks();
    for (size_t i = 0; i < layerBlocks.size(); ++i) {
        int removeId = layerBlocks[i]->getId();

        for (size_t j = 0; j < remainingBlocks.size(); ++j) {
            if (remainingBlocks[j]->getId() == removeId) {
                remainingBlocks.erase(remainingBlocks.begin() + j);
                break;
            }
        }
    }
}
