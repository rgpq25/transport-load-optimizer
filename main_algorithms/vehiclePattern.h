/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   vehiclePattern.h
 * Author: renzo
 *
 * Created on 16 de junio de 2025, 00:45
 */

#ifndef VEHICLEPATTERN_H
#define VEHICLEPATTERN_H

#include "transportUnit.h"
#include "layerCandidate.h"
#include <vector>

using namespace std;

class VehiclePattern {
public:
    TransportUnit* vehicle;                // Camión asignado
    vector<LayerCandidate> layers;    // Capas colocadas
    double currentWeight = 0.0;

    double usedVolume() const;
    double usedWeight() const { return currentWeight; }
};

#endif /* VEHICLEPATTERN_H */

