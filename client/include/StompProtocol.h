#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/event.h"
#include "../include/StompMessage.h"

class StompProtocol
{
private:
    std::string username;
    bool isConnected;
    int subIdCounter;
    int receiptIdCounter;
    std::mutex mtx;
    int disconnectReceiptId;
    std::atomic<bool> shouldTerminateClient;

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

    void handleMessage(StompMessage& response);
    void handleReceipt(StompMessage& response);
    void handleError(StompMessage& response);

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
    bool shouldTerminate() const { return shouldTerminateClient.load(); }
};
