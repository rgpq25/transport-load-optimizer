#include "order.h"

Order::Order() {
}

Order::Order(int id, Client* client, int priority, vector<Delivery>& deliveries) {
    this->id = id;
    this->client = client;
    this->priority = priority;
    this->deliveries = deliveries;
}

int Order::getId() const {
    return id;
}

Client* Order::getClient() const {
    return client;
}

int Order::getPriority() const {
    return priority;
}

vector<Delivery>& Order::getDeliveries() {
    return deliveries;
}

const vector<Delivery>& Order::getDeliveries() const {
    return deliveries;
}