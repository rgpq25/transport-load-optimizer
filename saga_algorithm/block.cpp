#include "block.h"

Block::Block() {}

Block::Block(int id, double height, double width, double length, double weight, double fragility) {
    this->id = id;
    this->height = height;
    this->width = width;
    this->length = length;
    this->weight = weight;
    this->fragility = fragility;
}

int Block::getId() const {
    return id;
}

double Block::getHeight() const {
    return height;
}

double Block::getWidth() const {
    return width;
}

double Block::getLength() const {
    return length;
}

double Block::getWeight() const {
    return weight;
}

double Block::getFragility() const {
    return fragility;
}