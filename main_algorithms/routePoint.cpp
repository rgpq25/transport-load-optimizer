#include "routePoint.h"

RoutePoint::RoutePoint() {
}

RoutePoint::RoutePoint(Client* client, int minutesToClient) {
    this->client = client;
    this->minutesToClient = minutesToClient;
}

Client* RoutePoint::getClient() const {
    return client;
}

int RoutePoint::getMinutesToClient() const {
    return minutesToClient; 
}
