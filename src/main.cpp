#include <uwebsockets/App.h>
#include <iostream>

struct PerSocketData {
};

int main() {
    uWS::App().ws<PerSocketData>("/*", {
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
    }).listen(9001, [](auto *token) {
        if (token) {
            std::cout << "Server listening on port 9001" << std::endl;
        } else {
            std::cerr << "Failed to listen on port 9001" << std::endl;
        }
    }).run();

    std::cout << "Server shutting down" << std::endl;
    return 0;
}