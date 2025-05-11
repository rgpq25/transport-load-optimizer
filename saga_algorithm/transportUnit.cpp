#include "transportUnit.h"

TransportUnit::TransportUnit() {
}

TransportUnit::TransportUnit(int id, double height, double width, double length, double maxWeight, double minWeight) {
    this->id = id;
    this->height = height;
    this->width = width;
    this->length = length;
    this->maxWeight = maxWeight;
    this->minWeight = minWeight;
}

int TransportUnit::getId() const {
    return id;
}

double TransportUnit::getHeight() const {
    return height;
}

double TransportUnit::getWidth() const {
    return width;
}

double TransportUnit::getLength() const {
    return length;
}

double TransportUnit::getMaxWeight() const {
    return maxWeight;
}

double TransportUnit::getMinWeight() const {
    return minWeight;
}