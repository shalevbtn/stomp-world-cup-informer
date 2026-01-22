#include "../include/StompProtocol.h"
#include <fstream>

StompProtocol::StompProtocol() 
:   username(""), 
    isConnected(false),
    subIdCounter(1),
    receiptIdCounter(1),
    mtx(),
    disconnectReceiptId(-1),      
    shouldTerminateClient(false),
    gameToSubId(),
    receiptToCommands(),
    gameData(), 
    frames()
{}

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
    else if(command == "INVALID") {
        std::cout << msg.getBody() << std::endl;
    }
    else {
        std::cout << "Unknown command" << std::endl;
    }

    return frames;
}

bool StompProtocol::processServerResponse(std::string message) {
    std::lock_guard<std::mutex> lock(mtx);

    StompMessage response(message);
    std::string responseType = response.getCommand();
    //FOR DEBUG
    std::cout << "---DEBUG PRINTS---" << std::endl;
    std::cout << "RECEIVED FRAME:" << std::endl;
    std::cout << message << std::endl;

    if(responseType == "CONNECTED") {
        std::cout << "Login successful" << std::endl;
        isConnected = true;
        return true;
    }

    else if(responseType == "RECEIPT") {
        handleReceipt(response);
        return true;
    }

    else if(responseType == "MESSAGE") {
        handleMessage(response);
        return true;
    }
    else {
        handleError(response);
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
    disconnectReceiptId = receiptId;

    receiptToCommands[receiptId] = "Disconnected from the server.";
    msg.addHeader("receipt", std::to_string(receiptId));

    frames.push_back(msg);
}

void StompProtocol::handleJoin(StompMessage& msg) {
    if(!checkLogin()) return;

    std::string gameName = msg.getHeader("destination");

    if (gameToSubId.find(gameName) != gameToSubId.end()) {
        std::cout << "You're already subscribed to that channel." << std::endl;
        return;
    }

    std::string subIdStr = msg.getHeader("id");
    std::string receiptIdStr = msg.getHeader("receipt");

    int subId = subIdCounter++;
    int receiptId = receiptIdCounter++;
    
    gameToSubId[gameName] = subId;
    receiptToCommands[receiptId] = "Joined channel " + gameName;

    msg.addHeader("id", std::to_string(subId));
    msg.addHeader("receipt", std::to_string(receiptId));
    
    frames.push_back(msg);
}

void StompProtocol::handleReport(StompMessage& msg) {
    if(!checkLogin()) return;
    std::string filePath = msg.getHeader("file_path");

    try {
        names_and_events nne = parseEventsFile(filePath); 
    
        std::string team_a_name = nne.team_a_name;
        std::string team_b_name = nne.team_b_name;
        std::string game_name = team_a_name + "_" + team_b_name;

        std::vector<Event> sortedEvents = nne.events;
        std::sort(sortedEvents.begin(), sortedEvents.end(), [](const Event& a, const Event& b) {
            bool a_before_half = true;
            bool b_before_half = true;

            if (a.get_game_updates().count("before halftime")) {
                a_before_half = (a.get_game_updates().at("before halftime") == "true");
            }
            if (b.get_game_updates().count("before halftime")) {
                b_before_half = (b.get_game_updates().at("before halftime") == "true");
            }

            if (a_before_half != b_before_half) {
                return a_before_half > b_before_half;
            }
            return a.get_time() < b.get_time();
        });

        msg.removeHeader("file_path");
        msg.addHeader("destination", "/" + game_name);

        bool firstSEND = true;
        
        for(Event event : sortedEvents){
            gameData[game_name][username].push_back(event);
            StompMessage eventMessage = msg;

            if(firstSEND) {
                eventMessage.addHeader("user-file", filePath);
                firstSEND = false;
            }
            
            eventMessage.setBody(getReportBody(event));       
            frames.push_back(eventMessage);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to parse file '" << filePath << "'." << std::endl;
        std::cerr << "Details: " << e.what() << std::endl;
        std::cerr << "Check that the file exists and the path is correct." << std::endl;
        return; 
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
    msg.removeHeader("destination");
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

    std::map<std::string, std::string> general_stats;
    std::map<std::string, std::string> team_a_stats;
    std::map<std::string, std::string> team_b_stats;

    for (const auto& event : events) {
        for (const auto& pair : event.get_game_updates()) {
            general_stats[pair.first] = pair.second;
        }
        for (const auto& pair : event.get_team_a_updates()) {
            team_a_stats[pair.first] = pair.second;
        }
        for (const auto& pair : event.get_team_b_updates()) {
            team_b_stats[pair.first] = pair.second;
        }
    }

    for (const auto& pair : general_stats) {
        outFile << pair.first << ": " << pair.second << "\n";
    }
    
    if (!events.empty()) {
        outFile << events[0].get_team_a_name() << " stats:\n";
    }
    for (const auto& pair : team_a_stats) {
        outFile << pair.first << ": " << pair.second << "\n";
    }
    
    if (!events.empty()) {
        outFile << events[0].get_team_b_name() << " stats:\n";
    }
    for (const auto& pair : team_b_stats) {
        outFile << pair.first << ": " << pair.second << "\n";
    }

    outFile << "Game event reports:\n";
    for (const auto& event : events) {
        outFile << event.get_time() << " - " << event.get_name() << ":\n\n";
        outFile << event.get_discription() << "\n\n";
    }

    outFile.close();
    std::cout << "Summary written to " << file << std::endl;
}

void StompProtocol::handleMessage(StompMessage& msg) {

    std::string gameName = msg.getHeader("destination");

    if (!gameName.empty() && gameName[0] == '/') {
        gameName = gameName.substr(1);
    }

    std::stringstream ss(msg.getBody());
    std::string line;

    std::string user;
    std::string teamA;
    std::string teamB;
    std::string eventName;
    int time = 0;
    std::string description;
    std::map<std::string, std::string> gameUpdates;
    std::map<std::string, std::string> teamAUpdates;
    std::map<std::string, std::string> teamBUpdates;

    int section = 0;

    while (std::getline(ss, line)) {
        std::cout << line << std::endl; 

        if (line.empty()) continue;

        if (line == "general game updates:") { section = 1; continue; }
        if (line == "team a updates:")     { section = 2; continue; }
        if (line == "team b updates:")     { section = 3; continue; }
        if (line == "description:")        { section = 4; continue; }

        if (section == 0) {
            size_t split = line.find(':');
            if (split != std::string::npos) {
                std::string key = line.substr(0, split);
                std::string val = line.substr(split + 1);
                if (!val.empty() && val[0] == ' ') val = val.substr(1);

                if (key == "user") user = val;
                else if (key == "team a") teamA = val;
                else if (key == "team b") teamB = val;
                else if (key == "event name") eventName = val;
                else if (key == "time") {
                    try {
                        time = std::stoi(val);
                    } catch (...) { time = 0; }
                }
            }
        } 
        else if (section >= 1 && section <= 3) {
            size_t split = line.find(':');
            if (split != std::string::npos) {
                size_t keyStart = 0;
                while (keyStart < line.length() && (line[keyStart] == '\t' || line[keyStart] == ' ')) keyStart++;
                
                std::string key = line.substr(keyStart, split - keyStart);
                std::string val = line.substr(split + 1);
                if (!val.empty() && val[0] == ' ') val = val.substr(1);

                if (section == 1) gameUpdates[key] = val;
                else if (section == 2) teamAUpdates[key] = val;
                else if (section == 3) teamBUpdates[key] = val;
            }
        }
        else if (section == 4) {
            description += line + "\n";
        }
    }

    if (!gameName.empty() && !user.empty()) {
        Event event(teamA, teamB, eventName, time, gameUpdates, teamAUpdates, teamBUpdates, description);
        gameData[gameName][user].push_back(event);
    }
}

void StompProtocol::handleReceipt(StompMessage& msg) {
    std::string receiptIdStr = msg.getHeader("receipt-id");
    
    if (receiptIdStr.empty()) return;

    try {
        int receiptId = std::stoi(receiptIdStr);
         if (receiptId == disconnectReceiptId) {
            shouldTerminateClient = true;
            isConnected = false;
            std::cout << "Disconnected from server" << std::endl; 
        }
        else if (receiptToCommands.count(receiptId)) {
            std::cout << receiptToCommands[receiptId] << std::endl;
            receiptToCommands.erase(receiptId);
        } else {
            std::cout << "Receipt " << receiptId << " received." << std::endl;
        }
    } catch (...) {
        std::cerr << "Error parsing receipt ID" << std::endl;
    }
}

void StompProtocol::handleError(StompMessage& msg) {
    std::cout << "Received Error from Server:" << std::endl;
    std::cout << "Message: " << msg.getHeader("message") << std::endl;
    std::cout << "Details:\n" << msg.getBody() << std::endl;
    std::cout << "Disconnected" << std::endl;
    isConnected = false;
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
