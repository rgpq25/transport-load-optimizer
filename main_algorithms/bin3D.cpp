#include "bin3D.h"
#include <cmath>

Bin3D::Bin3D(double length, double width, double height) {
    dimensions = {length, width, height};
    
    // Initially, the whole bin is empty
    EMSs.clear();
    EMSs.push_back({{0.0, 0.0, 0.0}, dimensions});
}

bool Bin3D::fitsInside(const array<double, 3>& size, const array<double, 3>& position) const {
    return (
        position[0] + size[0] <= dimensions[0] &&
        position[1] + size[1] <= dimensions[1] &&
        position[2] + size[2] <= dimensions[2]
    );
}

bool Bin3D::collides(const array<double, 3>& position, const array<double, 3>& size) const {
    for (const PlacedBox& box : placedBoxes) {
        bool overlapX = !(position[0] + size[0] <= box.position[0] || box.position[0] + box.size[0] <= position[0]);
        bool overlapY = !(position[1] + size[1] <= box.position[1] || box.position[1] + box.size[1] <= position[1]);
        bool overlapZ = !(position[2] + size[2] <= box.position[2] || box.position[2] + box.size[2] <= position[2]);

        if (overlapX && overlapY && overlapZ) {
            return true;
        }
    }
    return false;
}

bool Bin3D::tryPlaceBlock(const Block* block, int orientation) {
    array<double, 3> size;

    // Step 1: Determine dimensions based on orientation
    if (orientation == 0) { //original
        size = {block->getLength(), block->getWidth(), block->getHeight()};
    } else {
        size = {block->getWidth(), block->getLength(), block->getHeight()};
    }

    // Step 2: Try placing at each EMS origin
    for (size_t i = 0; i < EMSs.size(); ++i) {
        const auto& ems = EMSs[i];
        const array<double, 3>& pos = ems.first;
        const array<double, 3>& emsSize = ems.second;

        // Check if block fits inside EMS
        if (size[0] > emsSize[0] || size[1] > emsSize[1] || size[2] > emsSize[2]) {
            continue; // Doesn't fit in this EMS
        }

        // Check if placement fits inside bin (extra guard)
        if (!fitsInside(size, pos)) continue;

        // Check collision with other boxes
        if (collides(pos, size)) continue;

        // TODO: Add support and fragility checks here
        if (!hasValidSupport(pos, size, block->getFragility())) continue;

        // If all good: place the block
        PlacedBox box;
        box.block = block;
        box.position = pos;
        box.size = size;
        box.orientationIndex = orientation;  // index passed during rotation test
        placedBoxes.push_back(box);

        // Update EMS (to be implemented)
        splitEMS(pos, size);
        pruneEMS();

        return true;
    }

    return false;  // No EMS could accommodate the block
}




bool Bin3D::hasValidSupport(const array<double, 3>& position, const array<double, 3>& size, double fragility) const {
    double z = position[2];
    if (z == 0.0) return true;  // Sitting on the floor

    double totalArea = size[0] * size[1];
    double supportedArea = 0.0;

    for (const PlacedBox& pb : placedBoxes) {
        // Support must come from box that sits exactly below (touching top surface)
        double topZ = pb.position[2] + pb.size[2];
        if (fabs(topZ - z) > 1e-6) continue; // not on same support plane

        // Check fragility constraint
        if (fragility < pb.block->getFragility()) return false;  // can't sit on weaker block

        // Compute overlap area in X-Y plane
        double x1 = max(position[0], pb.position[0]);
        double x2 = min(position[0] + size[0], pb.position[0] + pb.size[0]);
        double y1 = max(position[1], pb.position[1]);
        double y2 = min(position[1] + size[1], pb.position[1] + pb.size[1]);

        double overlap = max(0.0, x2 - x1) * max(0.0, y2 - y1);
        supportedArea += overlap;
    }

    return supportedArea >= totalArea * 0.95;
}



void Bin3D::splitEMS(const array<double, 3>& position, const array<double, 3>& size) {
    vector<pair<array<double, 3>, array<double, 3>>> newEMSs;

    for (auto it = EMSs.begin(); it != EMSs.end(); ++it) {
        const array<double, 3>& emsPos = it->first;
        const array<double, 3>& emsSize = it->second;

        // Skip EMSs that do not contain the placed box
        if (position[0] < emsPos[0] || position[1] < emsPos[1] || position[2] < emsPos[2]) continue;
        if (position[0] + size[0] > emsPos[0] + emsSize[0]) continue;
        if (position[1] + size[1] > emsPos[1] + emsSize[1]) continue;
        if (position[2] + size[2] > emsPos[2] + emsSize[2]) continue;

        // Split along X
        if (position[0] + size[0] < emsPos[0] + emsSize[0]) {
            array<double, 3> newPos = {position[0] + size[0], emsPos[1], emsPos[2]};
            array<double, 3> newSize = {
                emsPos[0] + emsSize[0] - (position[0] + size[0]),
                emsSize[1],
                emsSize[2]
            };
            newEMSs.push_back({newPos, newSize});
        }

        // Split along Y
        if (position[1] + size[1] < emsPos[1] + emsSize[1]) {
            array<double, 3> newPos = {emsPos[0], position[1] + size[1], emsPos[2]};
            array<double, 3> newSize = {
                emsSize[0],
                emsPos[1] + emsSize[1] - (position[1] + size[1]),
                emsSize[2]
            };
            newEMSs.push_back({newPos, newSize});
        }

        // Split along Z
        if (position[2] + size[2] < emsPos[2] + emsSize[2]) {
            array<double, 3> newPos = {emsPos[0], emsPos[1], position[2] + size[2]};
            array<double, 3> newSize = {
                emsSize[0],
                emsSize[1],
                emsPos[2] + emsSize[2] - (position[2] + size[2])
            };
            newEMSs.push_back({newPos, newSize});
        }

        // Remove the used EMS
        EMSs.erase(it);
        break;  // only split one EMS (the one we placed into)
    }

    // Add the new EMSs
    EMSs.insert(EMSs.end(), newEMSs.begin(), newEMSs.end());
}



void Bin3D::pruneEMS() {
    vector<bool> keep(EMSs.size(), true);

    for (size_t i = 0; i < EMSs.size(); ++i) {
        const auto& aPos = EMSs[i].first;
        const auto& aSize = EMSs[i].second;

        for (size_t j = 0; j < EMSs.size(); ++j) {
            if (i == j) continue;

            const auto& bPos = EMSs[j].first;
            const auto& bSize = EMSs[j].second;

            // Check if EMS i is fully inside EMS j
            if (aPos[0] >= bPos[0] &&
                aPos[1] >= bPos[1] &&
                aPos[2] >= bPos[2] &&
                aPos[0] + aSize[0] <= bPos[0] + bSize[0] &&
                aPos[1] + aSize[1] <= bPos[1] + bSize[1] &&
                aPos[2] + aSize[2] <= bPos[2] + bSize[2]) {
                keep[i] = false;
                break;
            }
        }
    }

    // Remove those marked as false
    vector<pair<array<double, 3>, array<double, 3>>> pruned;
    for (size_t i = 0; i < EMSs.size(); ++i) {
        if (keep[i]) pruned.push_back(EMSs[i]);
    }

    EMSs = pruned;
}

const vector<PlacedBox>& Bin3D::getPlacedBoxes() const { return placedBoxes; }

