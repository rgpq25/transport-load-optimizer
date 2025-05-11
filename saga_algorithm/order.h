#ifndef ORDER_H
#define ORDER_H

#include "delivery.h"
#include "client.h"
#include <string>
#include <vector>

using namespace std;

class Order {
private:
    int id;
    Client* client;
    int priority;
    vector<Delivery> deliveries;
public:
    Order();
    Order(int id, Client* client, int priority, vector<Delivery>& deliveries);
    
    int getId() const;
    Client* getClient() const;
    int getPriority() const;
    vector<Delivery>& getDeliveries();
    const vector<Delivery>& getDeliveries() const;
};

#endif /* ORDER_H */

