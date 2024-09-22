#ifndef CLIENT_H
#define CLIENT_H

#include <uwebsockets/WebSocket.h>
#include <string>
#include <vector>
#include "Bucket.h"

struct PerSocketData;

class Client {
private:
    uWS::WebSocket<false, true, PerSocketData>* ws;
    uint8_t id;
    int rank;
    double x;
    double y;
    std::string world;
    Bucket pixelBucket;

public:
    Client(uWS::WebSocket<false, true, PerSocketData>* socket);

    void setId(uint8_t newId);
    uint8_t getId() const;

    void setRank(int newRank);
    int getRank() const;

    void setPosition(double newX, double newY);
    std::pair<double, double> getPosition() const;

    void setWorld(const std::string& newWorld);
    std::string getWorld() const;

    Bucket& getPixelBucket();

    void send(const std::vector<uint8_t>& data);
    void disconnect();
};

#endif // CLIENT_H