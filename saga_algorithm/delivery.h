#ifndef DELIVERY_H
#define DELIVERY_H

#include "block.h"
#include <vector>
#include <string>

using namespace std;

class Delivery {
private:
    int id;
    string shift;       //"morning" | "afternoon" | "null"
    string dueDate;     //YYYY-MM-DD
    vector<Block*> blocksToDeliver;
public:
    Delivery();
    Delivery(int id, const string& shift, const string& dueDate, vector<Block*>& blocksToDeliver);
    
    void printDelivery();
    
    int getId() const;
    const string& getShift() const;
    const string& getDueDate()const;
    vector<Block*>& getBlocksToDeliver();
    const vector<Block*>& getBlocksToDeliver() const;
};

#endif /* DELIVERY_H */

