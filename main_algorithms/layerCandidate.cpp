#include "layerCandidate.h"

LayerCandidate::LayerCandidate() {
    height = 0.0;
    score = 0.0;
}

void LayerCandidate::addBlock(const Block* block, int orientation, double x, double y) {
    blocks.push_back(block);
    orientations[block->getId()] = orientation;
    positions[block->getId()] = make_pair(x, y);

    double blockHeight = block->getHeight();
    if (blockHeight > height) {
        height = blockHeight;
    }
}

const vector<const Block*>& LayerCandidate::getBlocks() const {
    return blocks;
}

int LayerCandidate::getOrientation(int blockId) const {
    return orientations.at(blockId);
}

pair<double, double> LayerCandidate::getPosition(int blockId) const {
    return positions.at(blockId);
}

void LayerCandidate::setHeight(double h) {
    height = h;
}

double LayerCandidate::getHeight() const {
    return height;
}

void LayerCandidate::setScore(double s) {
    score = s;
}

double LayerCandidate::getScore() const {
    return score;
}
