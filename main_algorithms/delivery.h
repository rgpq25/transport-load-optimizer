#ifndef DELIVERY_H
#define DELIVERY_H

#include "block.h"
#include "client.h"
#include <vector>
#include <string>

using namespace std;

class Order;
class Delivery {
private:
    int id;
    string shift;
    string dueDate;
    vector<Block*> blocksToDeliver;
    Order* parentOrder;
public:
    Delivery();
    Delivery(int id, const string& shift, const string& dueDate, vector<Block*>& blocksToDeliver, Order* parentOrder);
    
    void printDelivery();
    
    int getId() const;
    const string& getShift() const;
    const string& getDueDate()const;
    vector<Block*>& getBlocksToDeliver();
    const vector<Block*>& getBlocksToDeliver() const;
    
    void setParentOrder(Order* order);
    Order* getParentOrder();
    
    int getPriority() const;
    Client* getClient() const;
};

#endif /* DELIVERY_H */

