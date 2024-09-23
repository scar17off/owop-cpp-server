#include "Command.h"
#include "Client.h"
#include "World.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unordered_map>

std::unordered_map<std::string, int> Command::commandPermissions = {
    {"help", 0},
    {"nick", 0},
    {"tell", 0},
    {"kick", 2},
    {"setrank", 3},
    {"tp", 2},
    {"adminlogin", 0},
    {"modlogin", 0},
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
    commandFunctions["nick"] = [this](const std::vector<std::string>& args) { this->nick(args); };
    commandFunctions["tell"] = [this](const std::vector<std::string>& args) { this->tell(args); };
    commandFunctions["kick"] = [this](const std::vector<std::string>& args) { this->kick(args); };
    commandFunctions["setrank"] = [this](const std::vector<std::string>& args) { this->setrank(args); };
    commandFunctions["tp"] = [this](const std::vector<std::string>& args) { this->tp(args); };
    commandFunctions["adminlogin"] = [this](const std::vector<std::string>& args) { this->adminlogin(args); };
    commandFunctions["modlogin"] = [this](const std::vector<std::string>& args) { this->modlogin(args); };
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

void Command::nick(const std::vector<std::string>& args) {
    const int maxNickLength = 16;
    if (args.empty()) {
        client->setNickname("");
        client->send("Your nickname has been reset.");
        return;
    }
    std::string newNick = args[0];
    if (newNick.length() > maxNickLength) {
        client->send("Nickname is too long. Maximum length is " + std::to_string(maxNickLength) + " characters.");
        return;
    }
    client->setNickname(newNick);
    client->send("Your nickname has been changed to: " + newNick);
}

void Command::tell(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        client->send("Usage: /tell <player_id> <message>");
        return;
    }
    int targetId;
    try {
        targetId = std::stoi(args[0]);
    } catch (const std::invalid_argument&) {
        client->send("Invalid player ID. Please use a number.");
        return;
    }
    std::string message = "";
    for (size_t i = 1; i < args.size(); ++i) {
        message += args[i] + " ";
    }
    Client* targetClient = client->getWorld()->getClientById(targetId);
    if (targetClient) {
        client->send("-> You tell " + std::to_string(targetId) + ": " + message);
        targetClient->send("-> " + std::to_string(client->getId()) + " tells you: " + message);
    } else {
        client->send("Player not found with ID: " + std::to_string(targetId));
    }
}

void Command::kick(const std::vector<std::string>& args) {
    if (args.empty()) {
        client->send("Usage: /kick <player_id> [reason]");
        return;
    }
    int targetId;
    try {
        targetId = std::stoi(args[0]);
    } catch (const std::invalid_argument&) {
        client->send("Invalid player ID. Please use a number.");
        return;
    }
    std::string reason = args.size() > 1 ? args[1] : "No reason specified";
    Client* targetClient = client->getWorld()->getClientById(targetId);
    if (targetClient) {
        if (targetClient->getRank() >= client->getRank()) {
            client->send("You can't kick a player with equal or higher rank.");
            return;
        }
        targetClient->send("You have been kicked. Reason: " + reason);
        targetClient->disconnect();
        client->getWorld()->broadcastMessage("Player " + std::to_string(targetId) + " has been kicked. Reason: " + reason);
    } else {
        client->send("Player not found with ID: " + std::to_string(targetId));
    }
}

void Command::setrank(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        client->send("Usage: /setrank <player_id> <rank>");
        return;
    }
    int targetId;
    try {
        targetId = std::stoi(args[0]);
    } catch (const std::invalid_argument&) {
        client->send("Invalid player ID. Please use a number.");
        return;
    }
    int newRank;
    try {
        newRank = std::stoi(args[1]);
    } catch (const std::invalid_argument&) {
        client->send("Invalid rank. Please use a number.");
        return;
    }
    if (newRank < 0 || newRank > 3) {
        client->send("Invalid rank. Rank must be between 0 and 3.");
        return;
    }
    Client* targetClient = client->getWorld()->getClientById(targetId);
    if (targetClient) {
        if (targetClient->getRank() >= client->getRank() || newRank >= client->getRank()) {
            client->send("You can't change the rank of a player with equal or higher rank, or set a rank equal to or higher than your own.");
            return;
        }
        targetClient->setRank(newRank);
        client->send("Rank of player " + std::to_string(targetId) + " has been set to " + std::to_string(newRank));
        targetClient->send("Your rank has been changed to " + std::to_string(newRank));
    } else {
        client->send("Player not found with ID: " + std::to_string(targetId));
    }
}

void Command::tp(const std::vector<std::string>& args) {
    if (args.size() < 1) {
        client->send("Usage: /tp <x> <y> or /tp <player_id>");
        return;
    }
    if (args.size() == 2) {
        try {
            double x = std::stod(args[0]);
            double y = std::stod(args[1]);
            client->setPosition(x, y, true);
            client->send("Teleported to (" + std::to_string(static_cast<int>(x)) + ", " + std::to_string(static_cast<int>(y)) + ")");
        } catch (const std::invalid_argument&) {
            client->send("Invalid coordinates. Please use numbers.");
        }
    } else {
        int targetId;
        try {
            targetId = std::stoi(args[0]);
        } catch (const std::invalid_argument&) {
            client->send("Invalid player ID. Please use a number.");
            return;
        }
        Client* targetClient = client->getWorld()->getClientById(targetId);
        if (targetClient) {
            auto [x, y] = targetClient->getPosition();
            client->setPosition(x, y, true);
            client->send("Teleported to player " + std::to_string(targetId) + " at (" + std::to_string(static_cast<int>(x)) + ", " + std::to_string(static_cast<int>(y)) + ")");
        } else {
            client->send("Player not found with ID: " + std::to_string(targetId));
        }
    }
}

void Command::adminlogin(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        client->send("Usage: /adminlogin <password>");
        return;
    }
    std::string password = getEnvValue("adminlogin");
    if (args[0] == password) {
      int8_t rank = 1;
        client->sendBinary({4, 3});
        client->setRank(3);
        client->send("Server: You are now an admin. Do /help for a list of commands.");
    } else {
        client->send("Wrong password.");
    }
}

void Command::modlogin(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        client->send("Usage: /modlogin <password>");
        return;
    }
    std::string password = getEnvValue("modlogin");
    if (args[0] == password) {
        client->sendBinary({4, 2});
        client->setRank(2);
        client->send("Server: You are now a moderator. Do /help for a list of commands.");
    } else {
        client->send("Wrong password.");
    }
}

std::string Command::getEnvValue(const std::string& key) {
    std::ifstream envFile(".env");
    std::string line;
    while (std::getline(envFile, line)) {
        std::istringstream iss(line);
        std::string envKey, envValue;
        if (std::getline(iss, envKey, '=') && std::getline(iss, envValue)) {
            if (envKey == key) {
                return envValue;
            }
        }
    }
    return "";
}