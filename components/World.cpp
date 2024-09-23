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

Client* World::getClientByNickname(const std::string& nickname) {
    for (Client* client : clients) {
        if (client->getNickname() == nickname) {
            return client;
        }
    }
    return nullptr;
}

Client* World::getClientById(int id) {
    for (Client* client : clients) {
        if (client->getId() == id) {
            return client;
        }
    }
    return nullptr;
}

void World::broadcastMessage(const std::string& message) {
    for (Client* client : clients) {
        client->send(message);
    }
}