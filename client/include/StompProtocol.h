#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/event.h"

struct GameStats {
    std::map<std::string, std::string> general_stats;
    std::map<std::string, std::string> team_a_stats;
    std::map<std::string, std::string> team_b_stats;
    std::vector<std::string> event_reports; 
};

class StompProtocol
{
private:
    std::string username;
    bool isConnected;
    int subIdCounter;
    int receiptIdCounter;

    std::map<std::string, int> gameToSubId;
    std::map<int, std::string> receiptToCommands; // receipts to commands we asked from the server
    std::map<std::string, std::map<std::string, std::vector<Event>>> gameData;


    std::string handleLogin(std::stringstream& ss);
    std::string handleLogout(std::stringstream& ss);
    std::string handleJoin(std::stringstream& ss);
    std::string handleReport(std::stringstream &ss);
    std::string handleExit(std::stringstream &ss);
    std::string handleSummary(std::stringstream &ss);

    std::string handleExit(std::stringstream& ss);
    void handleNewMessage(std::stringstream& ss);
    void handleReceipt(std::stringstream& ss);
    void handleError(std::stringstream& ss);

    bool checkLogin();

public:
    StompProtocol();
    std::vector<std::string> processUserInput(std::string input);
    bool processServerResponse(std::string response);

    bool isLoggedIn() const { return isConnected; }
    void setLoggedIn(bool status) { isConnected = status; }
    void setUsername(std::string name) { username = name; }
};
