#ifndef CLIENT_H
#define CLIENT_H

#include <uwebsockets/WebSocket.h>
#include <string>
#include <vector>
#include "Bucket.h"

class World; // Forward declaration

struct PerSocketData;

class Client {
private:
    uWS::WebSocket<false, true, PerSocketData>* ws;
    int id;
    int rank;
    double x;
    double y;
    Bucket pixelBucket;
    Bucket chatBucket;
    uint8_t r, g, b;
    uint8_t tool;
    World* world; // Changed from std::string to World*

public:
    Client(uWS::WebSocket<false, true, PerSocketData>* socket);

    void setId(int newId);
    int getId() const;

    void setRank(int newRank);
    int getRank() const;

    void setPosition(double newX, double newY);
    std::pair<double, double> getPosition() const;

    void setWorld(World* newWorld);
    World* getWorld() const;

    void setPixelBucket(double rate, double time);
    Bucket& getPixelBucket();

    void setChatBucket(double rate, double time);
    Bucket& getChatBucket();

    void send(const std::string& message);
    void disconnect();

    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setTool(uint8_t tool);

    uint8_t getR() const;
    uint8_t getG() const;
    uint8_t getB() const;
    uint8_t getTool() const;

    double getX() const;
    double getY() const;
};

#endif // CLIENT_H