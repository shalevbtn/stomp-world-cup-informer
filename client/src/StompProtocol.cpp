#include "../include/StompProtocol.h"
#include <fstream>

StompProtocol::StompProtocol() 
: username(""), isConnected(false), subIdCounter(0), receiptIdCounter(0) {}

std::vector<StompMessage> StompProtocol::process(StompMessage msg) {
    std::lock_guard<std::mutex> lock(mtx);
    std::string command = msg.getCommand();

    if (command == "CONNECT") {
        handleLogin(msg);
    }
    else if (command == "DISCONNECT") {
        handleLogout(msg);
    }
    else if (command == "SUBSCRIBE") {
        handleJoin(msg);
    }
    else if (command == "SEND") {
        handleReport(msg);
    }
    else if (command == "UNSUBSCRIBE") {
        handleExit(msg);
    }
    else if(command == "SUMMARY") {
        handleSummary(msg);
    }
    else {
        //TODO: Handle Unknown command
    }

    return frames;
}

bool StompProtocol::processServerResponse(std::string message) {
    std::lock_guard<std::mutex> lock(mtx);
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

void StompProtocol::clearFrames() { frames.clear(); }


//Private Functions:

void StompProtocol::handleLogin(StompMessage& msg) {
    if (isConnected) {
        std::cout << "The client is already logged in, log out before trying again" << std::endl;
        return;
    }

    setUsername(msg.getHeader("login"));
    frames.push_back(msg);
}

void StompProtocol::handleLogout(StompMessage& msg) {
    if(!checkLogin()) return;

    int receiptId = receiptIdCounter++;
    receiptToCommands[receiptId] = "DISCONNECT";
    msg.addHeader("receipt", std::to_string(receiptId));

    frames.push_back(msg);
}

void StompProtocol::handleJoin(StompMessage& msg) {
    if(!checkLogin()) return;

    std::string gameName = msg.getHeader("destination");
    std::string subIdStr = msg.getHeader("id");
    std::string receiptIdStr = msg.getHeader("receipt");

    int subId = subIdCounter++;
    int receiptId = receiptIdCounter++;
    
    gameToSubId[gameName] = subId;
    receiptToCommands[receiptId] = "Joined channel " + gameName; // Remember to print this on receipt

    msg.addHeader("id", std::to_string(subId));
    msg.addHeader("receipt", std::to_string(receiptId));
    
    frames.push_back(msg);

    //return frame;
    // TODO: so all the frames we made shouln'd be made here? It was a big mistake? because proccess just proccess the input and this 
    //frame is only after the server handle the message and sent a receipt*/
}

void StompProtocol::handleReport(StompMessage& msg) {
    if(!checkLogin()) return;
    std::string filePath = msg.getHeader("file_path");
    names_and_events nne = parseEventsFile(filePath); // TODO: check how to parse the json. I dont know what is this
    std::string team_a_name = nne.team_a_name;
    std::string team_b_name = nne.team_b_name;
    std::string game_name = team_a_name + "_" + team_b_name;

    std::vector<Event> sortedEvents = nne.events;
    std::sort(sortedEvents.begin(), sortedEvents.end(), [](const Event& a, const Event& b) {
        return a.get_time() < b.get_time();
    });

    msg.removeHeader("file_path");
    msg.addHeader("user-file", filePath);
    msg.addHeader("destination", "/" + game_name);
    
    for(Event event : sortedEvents){
        gameData[game_name][username].push_back(event);

        StompMessage eventMessage = msg;
        eventMessage.setBody(getReportBody(event));       
        frames.push_back(eventMessage);
    }
}

void StompProtocol::handleExit(StompMessage& msg) {
    if(!checkLogin()) return;

    std::string gameName = msg.getHeader("destination");

    if (gameToSubId.find(gameName) == gameToSubId.end()) {
        std::cout << "You are not subscribed to " << gameName << std::endl;
        return;
    }

    int subId = gameToSubId[gameName];
    int receiptId = receiptIdCounter++;
    
    receiptToCommands[receiptId] = "Exited channel " + gameName;
    msg.addHeader("id", std::to_string(subId));
    msg.addHeader("receipt", std::to_string(receiptId));

    gameToSubId.erase(gameName);

    frames.push_back(msg);
}

void StompProtocol::handleSummary(StompMessage& msg) {
    std::string gameName = msg.getHeader("game_name");
    std::string user = msg.getHeader("user");
    std::string file = msg.getHeader("file");

    if (gameData.find(gameName) == gameData.end() || 
        gameData[gameName].find(user) == gameData[gameName].end()) {
        std::cout << "No stats available for " << user << " in game " << gameName << std::endl;
        return;
    }

    const std::vector<Event>& events = gameData[gameName][user];
    
    std::ofstream outFile(file, std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << file << " for writing." << std::endl;
        return;
    }

    if (!events.empty()) {
        outFile << events[0].get_team_a_name() << " vs " << events[0].get_team_b_name() << "\n";
    }
    
    outFile << "Game stats:\n";
    outFile << "General stats:\n";

    if (!events.empty()) {
        const Event& lastEvent = events.back();
        for (auto const& [key, val] : lastEvent.get_game_updates()) {
            outFile << key << ": " << val << "\n";
        }
        outFile << lastEvent.get_team_a_name() << " stats:\n";
        for (auto const& [key, val] : lastEvent.get_team_a_updates()) {
            outFile << key << ": " << val << "\n";
        }
        outFile << lastEvent.get_team_b_name() << " stats:\n";
        for (auto const& [key, val] : lastEvent.get_team_b_updates()) {
            outFile << key << ": " << val << "\n";
        }
    }

    outFile << "Game event reports:\n";
    for (const auto& event : events) {
        outFile << event.get_time() << " - " << event.get_name() << ":\n\n";
        outFile << event.get_discription() << "\n\n";
    }

    outFile.close();
    std::cout << "Summary written to " << file << std::endl;
}

void StompProtocol::handleMessage(std::stringstream& ss) {

}

void StompProtocol::handleReceipt(std::stringstream& ss) {
    std::string receiptID;
    ss >> receiptID;
    std::cout << receiptToCommands.at(std::stoi(receiptID)) << std::endl;
}

void StompProtocol::handleError(std::stringstream& ss) { 


}

bool StompProtocol::checkLogin(){
    if (!isConnected) {
            std::cout << "Please login first" << std::endl;
            return false;
    }
    return true;
}

std::string StompProtocol::getReportBody(Event event) {
    std::string body;
    body += "user: " + username + "\n"; // Added semicolon
    body += "team a: " + event.get_team_a_name() + "\n";
    body += "team b: " +  event.get_team_b_name() + "\n";
    body += "event name: " + event.get_name() + "\n";
    body += "time: " + std::to_string(event.get_time()) + "\n";
    body += "general game updates:\n";

    for (auto& update : event.get_game_updates()) {
        body += "\t" + update.first + ": " + update.second + "\n";
    }

    body += "team a updates:\n";
    for (auto& update : event.get_team_a_updates()) {
        body += "\t" + update.first + ": " + update.second + "\n";
    }

    body += "team b updates:\n";
    for (const auto& update : event.get_team_b_updates()) {
        body += "\t" + update.first + ": " + update.second + "\n";
    }
    
    body += "description:\n" + event.get_discription() + "\n";

    return body;
}

