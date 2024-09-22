#include "Client.h"

Client::Client(uWS::WebSocket<false, true, PerSocketData>* socket)
    : ws(socket), id(""), rank(0), x(0.0), y(0.0), world("") {}

void Client::setId(uint8_t newId) {
    id = newId;
}

uint8_t Client::getId() const {
    return id;
}

void Client::setRank(int newRank) {
    rank = newRank;
}

int Client::getRank() const {
    return rank;
}

void Client::setPosition(double newX, double newY) {
    x = newX;
    y = newY;
}

std::pair<double, double> Client::getPosition() const {
    return {x, y};
}

void Client::setWorld(const std::string& newWorld) {
    world = newWorld;
}

std::string Client::getWorld() const {
    return world;
}

void Client::send(const std::vector<uint8_t>& data) {
    ws->send(std::string_view(reinterpret_cast<const char*>(data.data()), data.size()), uWS::OpCode::BINARY);
}

void Client::disconnect() {
    ws->close();
}