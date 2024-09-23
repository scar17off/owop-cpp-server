#ifndef WORLD_H
#define WORLD_H

#include <string>
#include <vector>

class Client; // Forward declaration

class World {
private:
    std::string name;
    std::vector<Client*> clients;

public:
    World(const std::string& worldName);
    
    void addClient(Client* client);
    void removeClient(Client* client);
    const std::vector<Client*>& getClients() const;
    const std::string& getName() const;
};

#endif // WORLD_H