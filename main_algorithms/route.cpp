#include "route.h"
#include "iostream"

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



vector<Client*> Route::getClients() const {
    vector<Client*> clients;
    for (const RoutePoint& rp : this->routePoints) {
        clients.push_back(rp.getClient());
    }
    return clients;
}