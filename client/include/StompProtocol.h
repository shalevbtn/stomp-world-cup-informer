#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/event.h"
#include "../include/StompMessage.h"

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
    std::mutex mtx;

    std::map<std::string, int> gameToSubId;
    std::map<int, std::string> receiptToCommands; // receipts to commands we asked from the server
    std::map<std::string, std::map<std::string, std::vector<Event>>> gameData;

    std::vector<StompMessage> frames;

    void handleLogin(StompMessage& msg);
    void handleLogout(StompMessage& msg);
    void handleJoin(StompMessage& msg);
    void handleReport(StompMessage& msg);
    void handleExit(StompMessage& msg);
    void handleSummary(StompMessage& msg);

    void handleMessage(std::stringstream& ss);
    void handleReceipt(std::stringstream& ss);
    void handleError(std::stringstream& ss);

    bool checkLogin();
    std::string getReportBody(Event event);
public:
    StompProtocol();
    std::vector<StompMessage> process(StompMessage msg);
    bool processServerResponse(std::string response);

    bool isLoggedIn() const { return isConnected; }
    void setLoggedIn(bool status) { isConnected = status; }
    void setUsername(std::string name) { username = name; }
    void clearFrames();
};
