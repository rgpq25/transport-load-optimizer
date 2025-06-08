#ifndef DELIVERYUTILS_H
#define DELIVERYUTILS_H

#include <vector>
#include <string>
#include "delivery.h"
#include "route.h"

namespace DeliveryUtils {
    vector<Delivery*> filterByDateAndRoute(const vector<Delivery*>& deliveries, const string& date, const Route& route);
    vector<Delivery*> filterByShift(const vector<Delivery*>& deliveries, const string& shift);
}

#endif /* DELIVERYUTILS_H */

