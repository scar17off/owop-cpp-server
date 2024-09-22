#include "Client.h"

Client::Client(uWS::WebSocket<false, true, PerSocketData>* socket)
    : ws(socket), id(0), rank(0), x(0.0), y(0.0), world(""), pixelBucket(32, 4), chatBucket(32, 4) {}

void Client::setId(int newId) {
    id = newId;
}

int Client::getId() const {
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

void Client::setPixelBucket(double rate, double time) {
    pixelBucket = Bucket(rate, time);
}

Bucket& Client::getPixelBucket() {
    return pixelBucket;
}

void Client::setChatBucket(double rate, double time) {
    chatBucket = Bucket(rate, time);
}

Bucket& Client::getChatBucket() {
    return chatBucket;
}

void Client::send(const std::vector<uint8_t>& data) {
    ws->send(std::string_view(reinterpret_cast<const char*>(data.data()), data.size()), uWS::OpCode::BINARY);
}

void Client::disconnect() {
    ws->close();
}

void Client::setColor(uint8_t newR, uint8_t newG, uint8_t newB) {
    r = newR;
    g = newG;
    b = newB;
}

void Client::setTool(uint8_t newTool) {
    tool = newTool;
}

uint8_t Client::getR() const { return r; }
uint8_t Client::getG() const { return g; }
uint8_t Client::getB() const { return b; }
uint8_t Client::getTool() const { return tool; }

double Client::getX() const { return x; }
double Client::getY() const { return y; }