#include "route.h"
#include "iostream"
#include <sstream>

Route::Route() {
}

Route::Route(int id, vector<RoutePoint>& routePoints) {
    this->id = id;
    this->routePoints = routePoints;
}

int Route::getId() const {
    return id;
}

vector<RoutePoint>& Route::getRoutePoints() {
    return routePoints;
}

const vector<RoutePoint>& Route::getRoutePoints() const {
    return routePoints;
}

string Route::getRouteAsString() const {
    ostringstream oss;
    
    for (int i = 0; i < this->routePoints.size(); i++) {
        auto& rp = this->routePoints[i];
        
        oss 
          << rp.getClient()->getName() 
          << "(" << rp.getMinutesToClient() << "min)";
        
        if (i != this->routePoints.size() - 1) {
            oss << " -> ";
        }
    }
    return oss.str();
}


vector<Client*> Route::getClients() const {
    vector<Client*> clients;
    for (const RoutePoint& rp : this->routePoints) {
        clients.push_back(rp.getClient());
    }
    return clients;
}