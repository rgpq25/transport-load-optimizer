#ifndef LAYERCANDIDATE_H
#define LAYERCANDIDATE_H

#include <vector>
#include <map>
#include "block.h"

using namespace std;

class LayerCandidate {
private:
    vector<const Block*> blocks;             // Bloques en esta capa
    map<int, int> orientations;              // blockId → 0|1 (horizontal orientation)
    map<int, pair<double, double> > positions; // blockId → (x, y)
    double height;                           // Altura máxima de la capa
    double score;                            // Puntaje de calidad de la capa

public:
    LayerCandidate();

    void addBlock(const Block* block, int orientation, double x, double y);
    
    const vector<const Block*>& getBlocks() const;
    int getOrientation(int blockId) const;
    pair<double, double> getPosition(int blockId) const;

    void setHeight(double h);
    double getHeight() const;

    void setScore(double s);
    double getScore() const;
};

#endif /* LAYERCANDIDATE_H */

