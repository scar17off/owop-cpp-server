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
        .open = [&protocol, &clients](auto *ws) {
            auto* socketData = (PerSocketData*)ws->getUserData();
            Client* client = new Client(ws);
            socketData->client = client;

            std::cout << "Client connected!" << std::endl;

            clients.push_back(client);

            // Send [captcha, 3] binary buffer
            uint8_t captchaCode = protocol["server"]["captcha"].get<uint8_t>();
            std::vector<uint8_t> buffer = {captchaCode, 3};
            ws->send(std::string_view(reinterpret_cast<char*>(buffer.data()), buffer.size()), uWS::OpCode::BINARY);

            // Send [0, Id, 0, 0, 0] binary buffer (id is the length of clients)
            uint8_t setIdCode = protocol["server"]["setId"].get<uint8_t>();
            uint8_t id = clients.size();
            buffer = {setIdCode, id, 0, 0, 0};
            ws->send(std::string_view(reinterpret_cast<char*>(buffer.data()), buffer.size()), uWS::OpCode::BINARY);
            client->setId(id);

            // Set default rank
            uint8_t setRankCode = protocol["server"]["setRank"].get<uint8_t>();
            uint8_t rank = 1;
            buffer = {setRankCode, rank};
            ws->send(std::string_view(reinterpret_cast<char*>(buffer.data()), buffer.size()), uWS::OpCode::BINARY);
            client->setRank(rank);
        },

        .message = [&clients](auto *ws, std::string_view message, uWS::OpCode opCode) {
            auto* socketData = (PerSocketData*)ws->getUserData();
            Client* client = socketData->client;

            if (opCode == uWS::OpCode::BINARY) {
                const uint8_t* data = reinterpret_cast<const uint8_t*>(message.data());
                size_t length = message.length();
                
                std::cout << "Received binary message of length: " << length << " | ";
                for (size_t i = 0; i < length; ++i) {
                    std::cout << "b" << i << ": " << static_cast<int>(data[i]);
                    if (i < length - 1) std::cout << " | ";
                }
                std::cout << std::endl;

                // Decode the first message from the client (world name)
                if (length > 2 && length - 2 <= 24 && client->getWorld().empty()) {
                    std::string world(reinterpret_cast<const char*>(data), length - 2);
                    // Remove non-alphanumeric characters and convert to lowercase
                    world.erase(std::remove_if(world.begin(), world.end(), [](char c) {
                        return !(std::isalnum(c) || c == '.' || c == '_');
                    }), world.end());
                    std::transform(world.begin(), world.end(), world.begin(), ::tolower);

                    if (world.empty()) {
                        world = "main";
                    }
                    client->setWorld(world);
                    std::cout << "Client joined world: " << world << std::endl;
                }
            } else {
                std::cout << "Received text message: " << message << std::endl;
                ws->send(message, opCode);
            }
        },

        .close = [&clients](auto *ws, int code, std::string_view message) {
            auto* socketData = ws->getUserData();
            Client* client = socketData->client;

            // Remove the client from the list
            clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
            delete client;

            std::cout << "Client disconnected!" << std::endl;
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

    std::cout << "Server shutting down" << std::endl;
    return 0;
}