#ifndef MAXIMALSPACE_H
#define MAXIMALSPACE_H

class MaximalSpace {
public:
    double originX;
    double originY;
    double originZ;
    double length;
    double width;
    double height;
    
    MaximalSpace();
    
    MaximalSpace(
        double originX,
        double originY,
        double originZ,
        double length,
        double width,
        double height
    );
};


#endif /* MAXIMALSPACE_H */

