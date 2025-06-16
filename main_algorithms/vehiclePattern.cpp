#include "vehiclePattern.h"

double VehiclePattern::usedVolume() const {
    double total = 0.0;
    for (const auto& layer : layers) {
        total += layer.volume;
    }
    return total;
}

