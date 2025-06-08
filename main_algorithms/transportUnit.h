#ifndef TRANSPORTUNIT_H
#define TRANSPORTUNIT_H

// Dont forget TransportUnits have a max daily loading factor

class TransportUnit {
private:
    int id;
    double height;
    double width;
    double length;
    double maxWeight;   //kg
    double minWeight;   //kg
public:
    TransportUnit();
    TransportUnit(int id, double height, double width, double length, double maxWeight, double minWeight);

    int getId() const;
    double getHeight() const;
    double getWidth() const;
    double getLength() const;
    double getMaxWeight() const;
    double getMinWeight() const;
    
    double getVolume() const;
};

#endif /* TRANSPORTUNIT_H */

