#include "../include/StompProtocol.h"

StompProtocol::StompProtocol() 
: username(""), isConnected(false), subIdCounter(0), receiptIdCounter(0) {}

std::vector<std::string> StompProtocol::processUserInput(std::string input) {
    std::vector<std::string> frames;
    std::stringstream ss(input);
    std::string command;
    ss >> command;

    if (command == "login") {
        std::string frame = handleLogin(ss);
        if(frame.empty()) {
            return frames;
        }
        else {frames.push_back(frame);}
    }
    else if (command == "join") {
        std::string frame = handleJoin(ss);
        if(frame.empty()) {
            return frames;
        }
        else {frames.push_back(frame);}
    }
    else if (command == "exit") {
        std::string frame = handleExit(ss);
        if(frame.empty()) {
            return frames;
        }
        else {frames.push_back(frame);}
    }
    // TODO: Implement 'report', 'logout', 'summary' later
    
    return frames;
}

std::string StompProtocol::handleLogin(std::stringstream& ss) {
    std::string hostPort, user, pass;
    ss >> hostPort >> user >> pass;

    if (isConnected) {
        std::cout << "The client is already logged in, log out before trying again" << std::endl;
        return;
    }

    username = user;
    
    std::string frame = "CONNECT\n"
                        "accept-version:1.2\n"
                        "host:stomp.cs.bgu.ac.il\n"
                        "login:" + user + "\n"
                        "passcode:" + pass + "\n"
                        "\n";
    return frame;
}

std::string StompProtocol::handleJoin(std::stringstream& ss) {
    if (!isConnected) {
        std::cout << "Please login first" << std::endl;
        return;
    }
    std::string gameName;
    ss >> gameName;

    int subId = subIdCounter++;
    int receiptId = receiptIdCounter++;
    
    gameToSubId[gameName] = subId;
    receiptToCommands[receiptId] = "joined " + gameName; // Remember to print this on receipt

    std::string frame = "SUBSCRIBE\n"
                        "destination:/" + gameName + "\n"
                        "id:" + std::to_string(subId) + "\n"
                        "receipt:" + std::to_string(receiptId) + "\n"
                        "\n";
    return frame;
}

std::string StompProtocol::handleExit(std::stringstream& ss) {
    if (!isConnected) {
            std::cout << "Please login first" << std::endl;
            return;
    }
    std::string gameName;
    ss >> gameName;

    if (gameToSubId.find(gameName) == gameToSubId.end()) {
        std::cout << "You are not subscribed to " << gameName << std::endl;
        return;
    }

    int subId = gameToSubId[gameName];
    int receiptId = receiptIdCounter++;
    
    receiptToCommands[receiptId] = "exited " + gameName;

    std::string frame = "UNSUBSCRIBE\n"
                        "id:" + std::to_string(subId) + "\n"
                        "receipt:" + std::to_string(receiptId) + "\n"
                        "\n";

    gameToSubId.erase(gameName);
    return frame;
}
