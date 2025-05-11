#include "input.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <map>

Input::Input() {
}

Input::Input(
    vector<Client>& clients,
    vector<Block>& blocks,
    vector<TransportUnit>& transportUnits,
    vector<Route>& routes, 
    vector<Order>& orders, 
    string operationStartTime,
    string operationEndTime
) {
    this->clients = clients;
    this->blocks = blocks;
    this->transportUnits = transportUnits;
    this->routes = routes;
    this->orders = orders;
    this->operationStartTime = operationStartTime;
    this->operationEndTime = operationEndTime;
}




void Input::loadFromFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }

    string line;
    string section;
    int expectedCount = 0;
    map<int, Client*> clientMap;
    map<int, Block*> blockMap;

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line[0] == '@') {
            stringstream headerStream(line);
            headerStream >> section >> expectedCount;

            if (section == "@clients") clients.reserve(expectedCount);
            else if (section == "@blocks") blocks.reserve(expectedCount);
            else if (section == "@transport_units") transportUnits.reserve(expectedCount);
            else if (section == "@routes") routes.reserve(expectedCount);
            else if (section == "@orders") orders.reserve(expectedCount);
            continue;
        }

        stringstream ss(line);

        if (section == "@clients") {
            int id; string name;
            ss >> id >> ws;
            getline(ss, name);
            clients.emplace_back(id, name);
            clientMap[id] = &clients.back();
        }
        else if (section == "@blocks") {
            int id; double h, w, l, weight, fragility;
            ss >> id >> h >> w >> l >> weight >> fragility;
            blocks.emplace_back(id, h, w, l, weight, fragility);
            blockMap[id] = &blocks.back();
        }
        else if (section == "@transport_units") {
            int id; double h, w, l, maxW, minW;
            ss >> id >> h >> w >> l >> maxW >> minW;
            transportUnits.emplace_back(id, h, w, l, maxW, minW);
        }
        else if (section == "@routes") {
            int routeId, numPoints;
            ss >> routeId >> numPoints;
            vector<RoutePoint> points;
            for (int i = 0; i < numPoints; ++i) {
                if (!getline(file, line)) break;
                if (line.empty() || line[0] == '@') {
                    section = line;
                    break;
                }
                stringstream rpStream(line);
                int clientId, travelTime;
                if (!(rpStream >> clientId >> travelTime)) continue;
                if (clientMap.find(clientId) == clientMap.end()) continue;
                points.emplace_back(clientMap[clientId], travelTime);
            }
            if (!points.empty()) {
                routes.emplace_back(routeId, points);
            }
        }
        else if (section == "@orders") {
            int orderId, clientId, priority, numDeliveries;
            ss >> orderId >> clientId >> priority >> numDeliveries;
            vector<Delivery> deliveries;
            for (int i = 0; i < numDeliveries; ++i) {
                if (!getline(file, line)) break;
                stringstream dStream(line);
                int deliveryId, numBlocks;
                string shift, dueDate;
                dStream >> deliveryId >> shift >> dueDate >> numBlocks;
                vector<Block*> blocksToDeliver;
                for (int j = 0; j < numBlocks; ++j) {
                    if (!getline(file, line)) break;
                    stringstream bStream(line);
                    int blockId; bStream >> blockId;
                    if (blockMap.find(blockId) != blockMap.end()) {
                        blocksToDeliver.push_back(blockMap[blockId]);
                    }
                }
                deliveries.emplace_back(deliveryId, shift, dueDate, blocksToDeliver, nullptr);
            }
            if (clientMap.find(clientId) != clientMap.end()) {
                orders.emplace_back(orderId, clientMap[clientId], priority, deliveries);
                
                Order* newOrderPtr = &orders.back();
                for (Delivery& d : newOrderPtr->getDeliveries()) {
                    d.setParentOrder(newOrderPtr);
                }
            }
        }
    }

    operationStartTime = "08:00";
    operationEndTime = "18:00";
    file.close();
}




void Input::printInputData() {
    cout << "\n--- Clients ---" << endl;
    for (auto& c : clients) {
        cout << "Client " << c.getId() << ": " << c.getName() << endl;
    }

    cout << "\n--- Blocks ---" << endl;
    for (auto& b : blocks) {
        cout << "Block " << b.getId() << " (" << b.getHeight() << "x" << b.getWidth() << "x" << b.getLength() << ", " << b.getWeight() << "kg, fragility: " << b.getFragility() << ")" << endl;
    }

    cout << "\n--- Transport Units ---" << endl;
    for (auto& t : transportUnits) {
        cout << "Unit " << t.getId() << " (" << t.getHeight() << "x" << t.getWidth() << "x" << t.getLength() << ", max: " << t.getMaxWeight() << "kg)" << endl;
    }

    cout << "\n--- Routes ---" << endl;
    for (auto& r : routes) {
        cout << "Route " << r.getId() << ": ";
        for (auto& rp : r.getRoutePoints()) {
            cout << rp.getClient()->getId() << "(" << rp.getMinutesToClient() << "min) ";
        }
        cout << endl;
    }

    cout << "\n--- Orders ---" << endl;
    for (auto& o : orders) {
        cout << "Order " << o.getId() << " (Client " << o.getClient()->getId() << ", Priority: " << o.getPriority() << ")" << endl;
        for (auto& d : o.getDeliveries()) {
            cout << "  Delivery " << d.getId() << " [Due: " << d.getDueDate() << ", Shift: " << d.getShift() << "] -> Blocks: ";
            for (auto& b : d.getBlocksToDeliver()) {
                cout << b->getId() << " ";
            }
            cout << endl;
        }
    }
}







vector<Client>& Input::getClients() {
    return clients;
}

vector<Block>& Input::getBlocks() {
    return blocks;
}

vector<TransportUnit>& Input::getTransportUnits() {
    return transportUnits;
}

vector<Route>& Input::getRoutes() {
    return routes;
}

vector<Order>& Input::getOrders() {
    return orders;
}



vector<Delivery> Input::getAllDeliveriesFromOrders() {
    vector<Delivery> allDeliveries;

    for(int i=0; i < this->orders.size(); i++) {
        vector<Delivery>& currDeliveries = this->orders[i].getDeliveries();
        for(int j=0; j < currDeliveries.size(); j++) {
            Delivery& currDelivery = currDeliveries[j];
            allDeliveries.push_back(currDelivery);
        }
    }
    
    return allDeliveries;
}

vector<string> Input::getUniqueDueDates() {
    vector<Delivery> allDeliveries = getAllDeliveriesFromOrders();
 
    set<string> uniqueDueDates;
    for(int i=0; i < allDeliveries.size(); i++) {
        Delivery currDelivery = allDeliveries[i];
        uniqueDueDates.insert(currDelivery.getDueDate());
    }
    
    vector<string> uniqueDueDatesVector(uniqueDueDates.begin(), uniqueDueDates.end());
    return uniqueDueDatesVector;
}

vector<Delivery> Input::filterDeliveriesByDate(string date) {
    vector<Delivery> allDeliveries = getAllDeliveriesFromOrders();
    vector<Delivery> filteredDeliveries;
    
    for(int i=0; i < allDeliveries.size(); i++) {
        Delivery currDelivery = allDeliveries[i];
        if(currDelivery.getDueDate() == date) {
            filteredDeliveries.push_back(currDelivery);
        }
    }
    
    return filteredDeliveries;
}