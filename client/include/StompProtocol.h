#pragma once

#include "../include/ConnectionHandler.h"

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
    std::map<std::string, std::map<std::string, GameStats>> gameData;

    std::vector<std::string> processUserInput(std::string input);
    std::vector<std::string> processKeyboardCommand(std::string line);
    bool processServerResponse(std::string response);
    bool isLoggedIn() const { return isConnected; }
    void setLoggedIn(bool status) { isConnected = status; }
    void setUsername(std::string name) { username = name; }

    void handleLogin(std::stringstream ss);
    void handleLogout(std::stringstream ss);
    void handleJoin(std::stringstream ss);
    void handleReport(std::stringstream ss);
    void handleSummary(std::stringstream ss);
    void handleExit(std::stringstream ss);
    
public:
    StompProtocol();   
};
