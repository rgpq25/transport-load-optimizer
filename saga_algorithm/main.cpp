#include "block.h"
#include "transportUnit.h"
#include "client.h"
#include "input.h"
#include <iostream>
#include <vector>
#include <string>

//Dates are given like:
//2024/12/10-03:00
//2024/12/12-18:00

//Pass by value: Will not modify the original value passed
//Pass by reference: Will modify the original value passed

using namespace std;

int main(int argc, char** argv) {
    
    Input mainInput;
    mainInput.loadFromFile("data.txt");
    mainInput.printInputData();
    
    cout << endl << "=========MAIN PROGRAM =========" << endl;
    vector<string> uniqueDueDates = mainInput.getUniqueDueDates();
    for(int i=0; i < uniqueDueDates.size(); i++) {
        cout << "Current due date is " << uniqueDueDates[i] << endl;
        vector<Delivery> ordersForDate = mainInput.filterDeliveriesByDate(uniqueDueDates[i]);
        
        
        for(int j=0; j < mainInput.getRoutes().size(); j++) {
            vector<Delivery> ordersInRoute = mainInput
        }
    }

    
    return 0;
}

