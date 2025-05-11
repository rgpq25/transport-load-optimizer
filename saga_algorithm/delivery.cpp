#include "delivery.h"

Delivery::Delivery() {
}

Delivery::Delivery(int id, const string& shift, const string& dueDate, vector<Block*>& blocksToDeliver) {
    this->id = id;
    this->shift = shift;
    this->dueDate = dueDate;
    this->blocksToDeliver = blocksToDeliver;
}


void Delivery::printDelivery() {
    cout << "ID = " << id << " | shift = " << shift << " | dueDate = " << dueDate << endl;
    for(int i=0; i<blocksToDeliver.size(); i++){
        Block *curr = blocksToDeliver[i];
        cout << "   Block " << i << " -> ID = " <<curr->getId() << endl;
    }
}



int Delivery::getId() const {
    return id;
}

const string& Delivery::getShift() const {
    return shift;
}

const string& Delivery::getDueDate() const {
    return dueDate;
}

vector<Block*>& Delivery::getBlocksToDeliver() {
    return blocksToDeliver;
}

const vector<Block*>& Delivery::getBlocksToDeliver() const {
    return blocksToDeliver;
}
