#include "../include/StompProtocol.h"
#include "../include/StompHelper.h"


StompProtocol::StompProtocol() 
: username(""), isConnected(false), subIdCounter(0), receiptIdCounter(0) {}

std::vector<StompMessage> StompProtocol::process(StompMessage msg) {

    std::string command = msg.getCommand();

    //TODO: I have this thing twice.. Is it smart?
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
    else if{
        handleSummary(msg);
    }
    //TODO: handle unexepted commands
    //TODO: להסביר לי מה עושה הפונקציה הזו, ואיך אפשר לשלוח דברים בלי שיש שדה קונקשיין או קונקשיין הנדלר
}

void StompProtocol::handleLogin(StompMessage& msg) {
    try{
        std::string hostPort(msg.getHeader("host"));
        std::string user(msg.getHeader("login"));
        std::string pass(msg.getHeader("passcode"));

        username = user;
    
        if (isConnected) {
            std::cout << "The client is already logged in, log out before trying again" << std::endl;
            return;
        }

        StompMessage connectMsg = StompHelper::getLoginFrame(hostPort, user, pass);

        //TODO: login

    } catch(...){
        std::cout << "”Could not connect to server" << std::endl;
    }
}

void StompProtocol::handleLogout(StompMessage& msg) {
    std::string hostPort(msg.getHeader("host"));
    std::string user(msg.getHeader("login"));
    std::string pass(msg.getHeader("passcode"));

    if(!checkLogin) return;
    
    StompMessage disconnectMsg = StompHelper::getLogoutFrame(user);
}

void StompProtocol::handleJoin(StompMessage& msg) {
    if(!checkLogin) return;

    std::string gameName = msg.getHeader("destination");
    std::string subIdStr = msg.getHeader("id");
    std::string receiptIdStr = msg.getHeader("receipt");

    int subId = subIdCounter++;
    int receiptId = receiptIdCounter++;
    
    gameToSubId[gameName] = subId;
    receiptToCommands[receiptId] = "Joined channel " + gameName; // Remember to print this on receipt

    std::string frame = "SUBSCRIBE\n"
                        "destination:/" + gameName + "\n"
                        "id:" + std::to_string(subId) + "\n"
                        "receipt:" + std::to_string(receiptId) + "\n"
                        "\n";
    return frame; //TODO: so all the frames we made shouln'd be made here? It was a big mistake? because proccess just proccess the input and this 
                  //frame is only after the server handle the message and sent a receipt
}

void StompProtocol::handleReport(StompMessage& msg) {
    if(!checkLogin) return;

    std::vector<std::string> allFrames;
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
        
        frame += "description:\n" + event.get_discription() + "\n";

        allFrames.push_back(frame);
    }
}

void StompProtocol::handleExit(StompMessage& msg) {
    if(!checkLogin) return;
    std::string gameName = msg.getHeader("destination");

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