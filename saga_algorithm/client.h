#ifndef CLIENT_H
#define CLIENT_H

#include <string>

using namespace std;

class Client {
private:
    int id;
    string name;
public:
    Client();
    Client(int id, const string& name);
    
    int getId() const;
    const string& getName() const;
    
    void setName(string name);
};

#endif /* CLIENT_H */

