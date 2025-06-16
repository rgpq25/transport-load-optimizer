#ifndef LAYERCANDIDATE_H
#define LAYERCANDIDATE_H

#include "delivery.h"
#include "block.h"
#include <vector>

using namespace std;

class LayerCandidate {
public:
    Delivery* delivery;                 // Entrega asociada
    vector<Block*> blocks;              // Bloques de la capa
    vector<int> orientations;           // Orientación (0/1) de cada bloque
    double footprintLength;             // Huella en X
    double footprintWidth;              // Huella en Y
    double layerHeight;                 // Grosor = altura de la capa
    double volume;                      // footprintLength * footprintWidth * layerHeight

    LayerCandidate();
    
    // Constructor
    LayerCandidate(
        Delivery* d,
        const std::vector<Block*>& blks,
        const std::vector<int>& orients,
        double fpLen,
        double fpWid,
        double h
    );
};

#endif /* LAYERCANDIDATE_H */

