#ifndef BLOCK_H
#define BLOCK_H

#include <iostream>

class Block {
private:
    int id;
    double height;
    double width;
    double length;
    double weight;
    double fragility;
public:
    Block();
    Block(int id, double height, double width, double length, double weight, double fragility);

    int getId() const;
    double getHeight() const;
    double getWidth() const;
    double getLength() const;
    double getWeight() const;
    double getFragility() const;
};

#endif /* BLOCK_H */

