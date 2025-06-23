#ifndef VEHICLEPATTERN_H
#define VEHICLEPATTERN_H

#include "transportUnit.h"
#include "layerCandidate.h"
#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

class VehiclePattern {
public:
    TransportUnit* vehicle;                // Camión asignado
    vector<LayerCandidate> layers;    // Capas colocadas
    double currentWeight = 0.0;

    double usedVolume() const;
    double usedWeight() const { return currentWeight; };
    void printPattern() {
        std::cout << "=== Vehicle Pattern Debug ===\n";
        std::cout << "Truck ID: " << vehicle->getId() << "\n";
        std::cout << "Used Volume: " << std::fixed << std::setprecision(4) << usedVolume() << "\n";
        std::cout << "Used Weight: " << std::fixed << std::setprecision(2) << usedWeight() << "\n";
        std::cout << "Number of Layers: " << layers.size() << "\n";

        for (size_t i = 0; i < layers.size(); ++i) {
            const LayerCandidate& ly = layers[i];
            std::cout << "  Layer " << i << " -> Delivery ID: " 
                      << ly.delivery->getId() << "\n";

            // Bloques en la capa
            std::cout << "    Blocks: ";
            for (Block* b : ly.blocks) {
                std::cout << b->getId() << " ";
            }
            std::cout << "\n";

            // Orientaciones de cada bloque
            std::cout << "    Orientations: ";
            for (int ori : ly.orientations) {
                std::cout << ori << " ";
            }
            std::cout << "\n";

            // Dimensiones de la capa
            std::cout << "    Footprint Length x Width x Height: "
                      << ly.footprintLength << " x "
                      << ly.footprintWidth  << " x "
                      << ly.layerHeight     << "\n";

            // Volumen estimado de la capa (si existe campo volume)
            #ifdef VOLUME_IN_LAYER_CANDIDATE
            std::cout << "    Layer Volume: " << ly.volume << "\n";
            #endif
        }
        std::cout << "==============================\n";
    };
};

#endif /* VEHICLEPATTERN_H */

