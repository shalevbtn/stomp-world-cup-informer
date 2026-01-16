#pragma once

#include <string>
#include <sstream>
#include "../include/StompMessage.h"

class InputHandler {
private:
    static StompMessage handleLogin(std::stringstream& ss);
    static StompMessage handleLogout(std::stringstream& ss);
    static StompMessage handleJoin(std::stringstream& ss);
    static StompMessage handleExit(std::stringstream& ss);
    static StompMessage handleReport(std::stringstream& ss);
    static StompMessage handleSummary(std::stringstream& ss);

public:
    static StompMessage processUserInput(const std::string& input);
};
