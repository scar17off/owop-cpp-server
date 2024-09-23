#ifndef WORLD_H
#define WORLD_H

#include <string>
#include <vector>

class Client;

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
    
    Client* getClientByNickname(const std::string& nickname);
    Client* getClientById(int id);
    void broadcastMessage(const std::string& message);
};

#endif // WORLD_H