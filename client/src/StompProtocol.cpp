#include "../include/StompProtocol.h"
#include "StompProtocol.h"

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
    // TODO: Implement 'report', 'summary' later
    
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

std::string StompProtocol::handleLogout(std::stringstream& ss) {
    std::string hostPort, user, pass;
    if(!checkLogin) return;
    
    std::string frame = "DISCONNECT\n"
                        "receipt:" + std::to_string(receiptIdCounter++) + "\n"
                        "\n";
    return frame;
}

std::string StompProtocol::handleJoin(std::stringstream& ss) {
    if(!checkLogin) return;

    std::string gameName;
    ss >> gameName;

    int subId = subIdCounter++;
    int receiptId = receiptIdCounter++;
    
    gameToSubId[gameName] = subId;
    receiptToCommands[receiptId] = "Joined channel " + gameName; // Remember to print this on receipt

    std::string frame = "SUBSCRIBE\n"
                        "destination:/" + gameName + "\n"
                        "id:" + std::to_string(subId) + "\n"
                        "receipt:" + std::to_string(receiptId) + "\n"
                        "\n";
    return frame;
}

std::string StompProtocol::handleReport(std::stringstream& ss) {
    if(!checkLogin) return;

    std::string allFrames = "";
    std::string filePath;
    ss >> filePath;

    names_and_events nne = parseEventsFile(filePath);

    std::string game_name = nne.team_a_name + "_" + nne.team_b_name;

    std::string team_a_name = nne.team_a_name;
    std::string team_b_name = nne.team_b_name;

    std::vector<Event> sortedEvents = nne.events;
    std::sort(sortedEvents.begin(), sortedEvents.end(), [](const Event& a, const Event& b) {
        return a.get_time() < b.get_time();
    });

    for(Event event : sortedEvents){
        gameData[game_name][username].push_back(event);
        std::string frame = "SEND\n"
                        "destination: " + game_name + "\n\n"
                        "user: " + username + "\n"
                        "team a: " + event.get_team_a_name() + "\n"
                        "team b: " + event.get_team_b_name() + "\n"
                        "event name: " + event.get_name() + "\n"
                        "time: " + std::to_string(event.get_time()) + "\n"
                        "general game updates: " + "\n";
        for (auto& update : event.get_game_updates()) {
            frame += "    " + update.first + ": " + update.second + "\n";
        }

        frame += "team a updates:\n";
        for (auto& update : event.get_team_a_updates()) {
            frame += "    " + update.first + ": " + update.second + "\n";
        }

        frame += "team b updates:\n";
        for (const auto& update : event.get_team_b_updates()) {
            frame += "    " + update.first + ": " + update.second + "\n";
        }
        
        frame += "description:\n" + event.get_discription() + "\n" + "\0";

        allFrames += frame;
    }
    return allFrames;
}

std::string StompProtocol::handleExit(std::stringstream& ss) {
    if(!checkLogin) return;
    std::string gameName;
    ss >> gameName;

    if (gameToSubId.find(gameName) == gameToSubId.end()) {
        std::cout << "You are not subscribed to " << gameName << std::endl;
        return;
    }

    int subId = gameToSubId[gameName];
    int receiptId = receiptIdCounter++;
    
    receiptToCommands[receiptId] = "Exited channel " + gameName;

    std::string frame = "UNSUBSCRIBE\n"
                        "id:" + std::to_string(subId) + "\n"
                        "receipt:" + std::to_string(receiptId) + "\n"
                        "\n";

    gameToSubId.erase(gameName);
    return frame;
}

bool StompProtocol::checkLogin(){
    if (!isConnected) {
            std::cout << "Please login first" << std::endl;
            return false;
    }
    return true;
}

bool StompProtocol::processServerResponse(std::string message) {
    std::stringstream ss(message);
    std::string responseType;
    ss >> responseType;

    if(responseType == "CONNECTED") {
        std::cout << "Login successful" << std::endl;
        //TODO: Log to the DB probably
        return true;
    }

    else if(responseType == "RECEIPT") {
        handleReceipt(ss);
        return true;
    }

    else if(responseType == "MESSAGE") {
        handleMessage(ss);
        return true;
    }
    else {
        handleError(ss);
        return false;
    }

}

void StompProtocol::handleReceipt(std::stringstream& ss) {
    std::string receiptID;
    ss >> receiptID;
    std::cout << receiptToCommands.at(std::stoi(receiptID)) << std::endl;
}

void StompProtocol::handleMessage(std::stringstream& ss) {

}

void StompProtocol::handleError(std::stringstream& ss) { 

}