#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

class Client;

class Command {
public:
    Command(const std::string& chat, Client* client);
    void execute();

private:
    std::string command;
    std::vector<std::string> args;
    Client* client;

    static std::unordered_map<std::string, int> commandPermissions;
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> commandFunctions;

    void initializeCommandFunctions();

    void help(const std::vector<std::string>& args);
};