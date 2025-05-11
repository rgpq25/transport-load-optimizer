#include "deliveryUtils.h"
#include <unordered_set>

namespace DeliveryUtils {
    vector<Delivery> filterByDateAndRoute(const vector<Delivery>& deliveries, const string& date, const Route& route) {
        vector<Delivery> result;
        vector<Client*> routeClientsVector = route.getClients();
        unordered_set<Client*> routeClients(routeClientsVector.begin(), routeClientsVector.end());
        
        for (const Delivery& d : deliveries) {
            Client* deliveryClient = d.getClient();
            if (d.getDueDate() == date && routeClients.count(deliveryClient) == 1) {
                result.push_back(d);
            }
        }
        return result;
    }
    
    vector<Delivery> filterByShift(const vector<Delivery>& deliveries, const string& shift) {
        vector<Delivery> result;
        for (const Delivery& d : deliveries) {
            if (d.getShift() == shift) {
                result.push_back(d);
            }
        }
        return result;
    }
}
