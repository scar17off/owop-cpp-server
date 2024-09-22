#include <uwebsockets/App.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <string>
#include <unordered_map>

struct PerSocketData {
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
    std::ifstream configFile("config.json");
    if (!configFile.is_open()) {
        std::cerr << "Failed to open config.json" << std::endl;
        return 1;
    }

    nlohmann::json config;
    configFile >> config;
    
    int port = config["port"].get<int>();

    const std::string routingDir = "./routing/";

    uWS::App()
    .get("/*", [routingDir](auto *res, auto *req) {
        std::string path = std::string(req->getUrl());
        std::string filePath = routingDir + (path == "/" ? "index.html" : path.substr(1));

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

                res->writeHeader("Content-Type", mimeType)
                   ->writeHeader("Cache-Control", "public, max-age=3600")
                   ->end(content);
            } else {
                res->writeStatus("404 Not Found")->end("File not found");
            }
        } else {
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
        .open = [](auto *ws) {
            std::cout << "Client connected!" << std::endl;
        },

        .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
            std::cout << "Received message: " << message << std::endl;
            ws->send(message, opCode);
        },

        .close = [](auto *ws, int code, std::string_view message) {
            std::cout << "Client disconnected!" << std::endl;
        }
    }).listen(port, [port](auto *token) {
        if (token) {
            std::cout << "Server listening on port " << port << std::endl;
        } else {
            std::cerr << "Failed to listen on port " << port << std::endl;
        }
    }).run();

    std::cout << "Server shutting down" << std::endl;
    return 0;
}