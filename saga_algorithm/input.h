#ifndef INPUT_H
#define INPUT_H

#include <vector>
#include <string>
#include "client.h"
#include "block.h"
#include "transportUnit.h"
#include "route.h"
#include "order.h"
#include "delivery.h"

using namespace std;

class Input {
private:
    vector<Client> clients;
    vector<Block> blocks;
    vector<TransportUnit> transportUnits;
    vector<Route> routes;
    vector<Order> orders;
 
    string operationStartTime;
    string operationEndTime;
public:
    Input();
    Input(
        vector<Client>& clients,
        vector<Block>& blocks,
        vector<TransportUnit>& transportUnits,
        vector<Route>& routes, 
        vector<Order>& orders, 
        string operationStartTime,
        string operationEndTime
    );
    
    void loadFromFile(const string& filePath);
    void printInputData();
    
    vector<Client>& getClients();
    vector<Block>& getBlocks();
    vector<TransportUnit>& getTransportUnits();
    vector<Route>& getRoutes();
    vector<Order>& getOrders();
    
    vector<Delivery> getAllDeliveriesFromOrders();
    vector<string> getUniqueDueDates();
    vector<Delivery> filterDeliveriesByDate(string date);
};

#endif /* INPUT_H */