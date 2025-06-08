#include "client.h"

Client::Client() {
}

Client::Client(int id, const string& name) {
    this->id = id;
    this->name = name;
}

int Client::getId() const {
    return id;
}
    
const string& Client::getName() const {
    return name;
}

void Client::setName(string name) {
    this->name = name;
}
