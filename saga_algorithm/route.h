#ifndef ROUTE_H
#define ROUTE_H

#include "routePoint.h"
#include <vector>

using namespace std;

class Route {
private:
    int id;
    vector<RoutePoint> routePoints;
public:
    Route();
    Route(int id, vector<RoutePoint>& routePoints);
    
    int getId() const;
    vector<RoutePoint>& getRoutePoints();
    const vector<RoutePoint>& getRoutePoints() const;
    
    vector<Client*> getClients() const;
};

#endif /* ROUTE_H */

