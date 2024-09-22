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
    int id;
    int rank;
    double x;
    double y;
    std::string world;
    Bucket pixelBucket;
    Bucket chatBucket;
    uint8_t r, g, b;
    uint8_t tool;

public:
    Client(uWS::WebSocket<false, true, PerSocketData>* socket);

    void setId(int newId);
    int getId() const;

    void setRank(int newRank);
    int getRank() const;

    void setPosition(double newX, double newY);
    std::pair<double, double> getPosition() const;

    void setWorld(const std::string& newWorld);
    std::string getWorld() const;

    void setPixelBucket(double rate, double time);
    Bucket& getPixelBucket();

    void setChatBucket(double rate, double time);
    Bucket& getChatBucket();

    void send(const std::vector<uint8_t>& data);
    void disconnect();

    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setTool(uint8_t tool);

    uint8_t getR() const;
    uint8_t getG() const;
    uint8_t getB() const;
    uint8_t getTool() const;

    double getX() const; // Add this method
    double getY() const; // Add this method
};

#endif // CLIENT_H