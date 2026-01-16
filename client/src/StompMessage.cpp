#include "../include/StompMessage.h"

std::string command;

    StompMessage::StompMessage(std::string msg){
        parse(msg);
    }

    void StompMessage::parse(std::string msg){
        std::stringstream ss(msg);
        ss >> command;

        if (command == "login") {
            handleLogin(ss);
        }
        else if (command == "logout") {
            handleLogout(ss);
        }
        else if (command == "join") {
            handleJoin(ss);
        }
        else if (command == "exit") {
             handleExit(ss);
        }
        else if (command == "report") {
            handleReport(ss);
        }
        else if (command == "summary") {
            handleSummary(ss);
        }
    }

    void StompMessage::handleLogin(std::stringstream& ss){
        // Implementation for handling login message
    }
    void StompMessage::handleLogout(std::stringstream& ss){
        // Implementation for handling logout message
    }
    void StompMessage::handleJoin(std::stringstream& ss){
        // Implementation for handling join message
    }
    void StompMessage::handleExit(std::stringstream& ss){
        // Implementation for handling exit message
    }
    void StompMessage::handleReport(std::stringstream& ss){
        // Implementation for handling report message
    }
    void StompMessage::handleSummary(std::stringstream& ss){
        // Implementation for handling summary message
    }   

    StompMessage(std::string msg);
    std::string getHeader(std::string headerKey);
    std::string getBody();