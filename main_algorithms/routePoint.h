#ifndef ROUTEPOINT_H
#define ROUTEPOINT_H

#include "client.h"

class RoutePoint {
private:
    Client* client;
    int minutesToClient; //minutes to travel to this client
public:
    RoutePoint();
    RoutePoint(Client* client, int minutesToClient);
    
    Client* getClient() const;
    int getMinutesToClient() const;
};

#endif /* ROUTEPOINT_H */
