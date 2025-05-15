#ifndef BIN3D_H
#define BIN3D_H

#include <vector>
#include <array>
#include "block.h"

using namespace std;

struct PlacedBox {
    const Block* block;
    array<double, 3> position;
    array<double, 3> size;
    int orientationIndex;
};

class Bin3D {
private:
    array<double, 3> dimensions;
    vector<PlacedBox> placedBoxes;

    // Each EMS is defined by bottom-left-back position + dimensions
    vector<pair<array<double, 3>, array<double, 3>>> EMSs;
    

    bool fitsInside(const array<double, 3>& boxSize, const array<double, 3>& position) const;
    bool collides(const array<double, 3>& position, const array<double, 3>& size) const;
    bool hasValidSupport(const array<double, 3>& position, const array<double, 3>& size, double fragility) const;

    void splitEMS(const array<double, 3>& position, const array<double, 3>& size);
    void pruneEMS();

public:
    Bin3D(double length, double width, double height);

    bool tryPlaceBlock(const Block* block, int orientation);
    const vector<PlacedBox>& getPlacedBoxes() const;
};

#endif /* BIN3D_H */

