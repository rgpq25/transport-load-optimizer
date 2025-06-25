#ifndef ROUTE_H
#define ROUTE_H

#include "routePoint.h"
#include <vector>
#include <string>

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
    
    string getRouteAsString() const;
    
    vector<Client*> getClients() const;
};

#endif /* ROUTE_H */

