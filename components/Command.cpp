#include "Command.h"
#include "Client.h"
#include "World.h"
#include <algorithm>
#include <sstream>
#include <iostream>

std::unordered_map<std::string, int> Command::commandPermissions = {
    {"help", 0},
};

Command::Command(const std::string& chat, Client* client)
    : client(client) {
    std::string chatWithoutPrefix = chat.substr(1);
    std::istringstream iss(chatWithoutPrefix);
    iss >> command;
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }

    initializeCommandFunctions();
}

void Command::initializeCommandFunctions() {
    commandFunctions["help"] = [this](const std::vector<std::string>& args) { this->help(args); };
}

void Command::execute() {
    auto it = commandFunctions.find(command);
    if (it != commandFunctions.end() && command != "sendTo") {
        int requiredRank = commandPermissions[command];
        if (client->getRank() >= requiredRank) {
            it->second(args);
        } else {
            client->send("You don't have permission!");
        }
    }
}

void Command::help(const std::vector<std::string>& args) {
    std::vector<std::string> availableCommands;
    for (const auto& cmd : commandPermissions) {
        if (client->getRank() >= cmd.second) {
            availableCommands.push_back(cmd.first);
        }
    }

    std::string helpString = "Commands: ";
    for (size_t i = 0; i < availableCommands.size(); ++i) {
        helpString += availableCommands[i];
        if (i < availableCommands.size() - 1) {
            helpString += ", ";
        }
    }

    client->send(helpString);
}