#include "Client.h"
#include "World.h"

Client::Client(uWS::WebSocket<false, true, PerSocketData>* socket)
    : ws(socket), id(0), rank(0), x(0.0), y(0.0), world(nullptr), pixelBucket(32, 4), chatBucket(32, 4), nickname("") {}

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

void Client::setPosition(double newX, double newY, bool teleport) {
    if (teleport) {
        std::vector<uint8_t> tp(9);
        *reinterpret_cast<uint8_t*>(&tp[0]) = 3;
        *reinterpret_cast<uint32_t*>(&tp[1]) = static_cast<uint32_t>(newX);
        *reinterpret_cast<uint32_t*>(&tp[5]) = static_cast<uint32_t>(newY);
        this->sendBinary(tp);
    }
    x = newX;
    y = newY;
}

std::pair<double, double> Client::getPosition() const {
    return {x, y};
}

void Client::setWorld(World* newWorld) {
    world = newWorld;
}

World* Client::getWorld() const {
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

void Client::send(const std::string& message) {
    ws->send(message, uWS::OpCode::TEXT);
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

void Client::setNickname(const std::string& newNickname) {
    nickname = newNickname;
}

std::string Client::getNickname() const {
    return nickname;
}

void Client::sendBinary(const std::vector<uint8_t>& buffer) {
    ws->send(std::string_view(reinterpret_cast<const char*>(buffer.data()), buffer.size()), uWS::OpCode::BINARY);
}