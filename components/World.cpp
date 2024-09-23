#include "World.h"
#include "Client.h"

World::World(const std::string& worldName) : name(worldName) {}

void World::addClient(Client* client) {
    clients.push_back(client);
}

void World::removeClient(Client* client) {
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
}

const std::vector<Client*>& World::getClients() const {
    return clients;
}

const std::string& World::getName() const {
    return name;
}