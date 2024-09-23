#include <uwebsockets/App.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "components/Client.h"
#include "components/Chunk.h"
#include "components/Command.h"
#include "components/World.h"

struct PerSocketData {
    Client* client;
};

// Function to get MIME type based on file extension
std::string getMimeType(const std::string& extension) {
    static const std::unordered_map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".ttf", "font/ttf"},
        {".otf", "font/otf"}
    };

    auto it = mimeTypes.find(extension);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

int main() {
    nlohmann::json config = nlohmann::json::parse(std::ifstream("config.json"));
    nlohmann::json protocol = nlohmann::json::parse(std::ifstream("protocol.json"));

    const std::string routingDir = "./routing/";
    const int port = config["port"].get<int>();

    std::vector<Client*> clients;
    std::unordered_map<std::string, World*> worlds;

    uWS::App()
    .get("/*", [routingDir](auto *res, auto *req) {
        std::string path = std::string(req->getUrl());
        std::string filePath = routingDir + (path == "/" ? "index.html" : path.substr(1));

        // Check if the file exists and is not a directory
        if (std::filesystem::exists(filePath) && !std::filesystem::is_directory(filePath)) {
            std::ifstream file(filePath, std::ios::binary);
            if (file) {
                std::string extension = std::filesystem::path(filePath).extension().string();
                std::string mimeType = getMimeType(extension);
                
                file.seekg(0, std::ios::end);
                size_t size = file.tellg();
                file.seekg(0, std::ios::beg);

                std::string content(size, '\0');
                file.read(&content[0], size);

                // Send the file content with appropriate headers
                res->writeHeader("Content-Type", mimeType)
                   ->writeHeader("Cache-Control", "public, max-age=3600")
                   ->end(content);
            } else {
                res->writeStatus("404 Not Found")->end("File not found");
            }
        } else {
            // If the file doesn't exist, serve index.html
            std::ifstream indexFile(routingDir + "index.html");
            if (indexFile) {
                std::string content((std::istreambuf_iterator<char>(indexFile)), std::istreambuf_iterator<char>());
                res->writeHeader("Content-Type", "text/html")->end(content);
            } else {
                res->writeStatus("404 Not Found")->end("Index file not found");
            }
        }
    })
    .ws<PerSocketData>("/*", {
        .open = [&protocol, &clients, &config, &worlds](auto *ws) {
            auto* socketData = (PerSocketData*)ws->getUserData();
            Client* client = new Client(ws);
            socketData->client = client;

            std::cout << "Client " << client->getId() << " connected!" << std::endl;

            auto sendBuffer = [&ws](const std::vector<uint8_t>& buffer) {
                ws->send(std::string_view(reinterpret_cast<const char*>(buffer.data()), buffer.size()), uWS::OpCode::BINARY);
            };

            // Send captcha
            sendBuffer({protocol["server"]["captcha"].get<uint8_t>(), 3});

            // Set client ID
            uint8_t id = static_cast<uint8_t>(clients.size() + 1);
            sendBuffer({protocol["server"]["setId"].get<uint8_t>(), id, 0, 0, 0});
            client->setId(id);

            // Set default rank
            uint8_t rank = 1;
            sendBuffer({protocol["server"]["setRank"].get<uint8_t>(), rank});
            client->setRank(rank);

            // Set rank's quota
            const auto& rankConfig = config["bucket"];
            const std::string rankKey = (rank == 0) ? "none" : (rank == 1) ? "user" : (rank == 2) ? "mod" : "admin";

            const auto& pixelConfig = rankConfig["pixel"][rankKey];
            client->setPixelBucket(pixelConfig[0].get<double>(), pixelConfig[1].get<double>());

            const auto& chatConfig = rankConfig["chat"][rankKey];
            client->setChatBucket(chatConfig[0].get<double>(), chatConfig[1].get<double>());

            clients.push_back(client);
        },

        .message = [&protocol, &clients, &worlds](auto *ws, std::string_view message, uWS::OpCode opCode) {
            auto* socketData = (PerSocketData*)ws->getUserData();
            Client* client = socketData->client;

            if (opCode == uWS::OpCode::BINARY) {
                const uint8_t* data = reinterpret_cast<const uint8_t*>(message.data());
                size_t length = message.length();
                
                // Log the bytes
                /*
                std::cout << "Received binary message of length: " << length << " | ";
                for (size_t i = 0; i < length; ++i) {
                    std::cout << "b" << i << ": " << static_cast<int>(data[i]);
                    if (i < length - 1) std::cout << " | ";
                }
                std::cout << std::endl;
                */

                // Decode the first message from the client (world name)
                if (length > 2 && length - 2 <= 24 && client->getWorld() == nullptr) {
                    std::string world(reinterpret_cast<const char*>(data), length - 2);
                    // Remove non-alphanumeric characters and convert to lowercase
                    world.erase(std::remove_if(world.begin(), world.end(), [](char c) {
                        return !(std::isalnum(c) || c == '.' || c == '_');
                    }), world.end());
                    std::transform(world.begin(), world.end(), world.begin(), ::tolower);

                    if (world.empty()) {
                        world = "main";
                    }

                    // Create or get the world
                    World* worldObj;
                    auto it = worlds.find(world);
                    if (it == worlds.end()) {
                        worldObj = new World(world);
                        worlds[world] = worldObj;
                    } else {
                        worldObj = it->second;
                    }
                    client->setWorld(worldObj);
                    worldObj->addClient(client);

                    std::cout << "Client " << client->getId() << " joined world: " << world << std::endl;
                }

                // Define variables for protocol message lengths
                constexpr int rankVerificationLength = 1;
                constexpr int requestChunkLength = 8;
                constexpr int protectChunkLength = 10;
                constexpr int setPixelLength = 11;
                constexpr int playerUpdateLength = 12;
                constexpr int clearChunkLength = 13;
                constexpr int pasteLength = 776;

                switch(length) {
                    case rankVerificationLength: {
                        uint8_t clientRank = data[0];
                        if (clientRank > client->getRank()) {
                            // Client rank is higher than server rank, disconnecting client.
                            ws->close();
                        }
                        break;
                    }
                    case requestChunkLength: {
                        int32_t x = *reinterpret_cast<const int32_t*>(data);
                        int32_t y = *reinterpret_cast<const int32_t*>(data + 4);
                        
                        int tileX = x;
                        int tileY = y;

                        Chunk chunk(tileX, tileY, client->getWorld()->getName());
                        chunk.loadFromFile();

                        std::vector<uint8_t> chunkData = chunk.getData();
                        ws->send(std::string_view(reinterpret_cast<char*>(chunkData.data()), chunkData.size()), uWS::OpCode::BINARY);
                        break;
                    }
                    case protectChunkLength:
                        // Handle protect chunk message
                        break;
                    case setPixelLength: {
                        if (client->getRank() == 0) break;
                        if (!client->getPixelBucket().canSpend(1) && !client->getPixelBucket().isInfinite()) break;

                        int32_t x = *reinterpret_cast<const int32_t*>(data);
                        int32_t y = *reinterpret_cast<const int32_t*>(data + 4);
                        uint8_t r = data[8];
                        uint8_t g = data[9];
                        uint8_t b = data[10];

                        int tileX = x / Chunk::CHUNK_SIZE;
                        int tileY = y / Chunk::CHUNK_SIZE;
                        int pixX = x % Chunk::CHUNK_SIZE;
                        int pixY = y % Chunk::CHUNK_SIZE;

                        Chunk chunk(tileX, tileY, client->getWorld()->getName());
                        if (chunk.isProtected() && client->getRank() < 2) break; // No permission to place on a protected chunk
                        
                        RGB currentColor = chunk.getColor(pixX, pixY);
                        if (currentColor.r == r && currentColor.g == g && currentColor.b == b) break;

                        chunk.setColor(pixX, pixY, RGB(r, g, b));
                        chunk.saveToFile();
                        break;
                    }
                    case playerUpdateLength: {
                        int32_t x = *reinterpret_cast<const int32_t*>(data);
                        int32_t y = *reinterpret_cast<const int32_t*>(data + 4);
                        uint8_t r = data[8];
                        uint8_t g = data[9];
                        uint8_t b = data[10];
                        uint8_t tool = data[11];

                        client->setPosition(x / 16, y / 16);
                        client->setColor(r, g, b);
                        client->setTool(tool);
                        break;
                    }
                    case clearChunkLength:
                        // Handle clear chunk message
                        break;
                    case pasteLength:
                        // Handle paste message
                        break;
                    default:
                        std::cout << "Unhandled message length: " << length << std::endl;
                        break;
                }
            } else {
                std::string messageStr(message);
                if (!messageStr.empty() && messageStr[0] == '/') {
                    Command cmd(messageStr, client);
                    cmd.execute();
                } else {
                    // Handle regular chat messages
                    std::string senderName = client->getNickname().empty() ? std::to_string(client->getId()) : client->getNickname();
                    std::string response = senderName + ": " + std::string(message);
                    ws->send(response, opCode);
                    std::cout << senderName << ": " << message << std::endl;
                }
            }
        },

        .close = [&clients, &worlds](auto *ws, int code, std::string_view message) {
            auto* socketData = ws->getUserData();
            Client* client = socketData->client;

            World* world = client->getWorld();
            world->removeClient(client);
            if (world->getClients().empty()) {
                worlds.erase(world->getName());
                delete world;
            }

            clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
            delete client;

            std::cout << "Client " << client->getId() << " disconnected!" << std::endl;
        }
    }).listen(port, [port](auto *token) {
        if (token) {
            std::cout << "Server listening on port " << port << std::endl;
        } else {
            std::cerr << "Failed to listen on port " << port << std::endl;
        }
    }).run();

    for (auto* client : clients) {
        delete client;
    }

    // Clean up worlds
    for (auto& pair : worlds) {
        delete pair.second;
    }

    std::cout << "Server shutting down" << std::endl;
    return 0;
}