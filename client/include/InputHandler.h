#pragma once

#include <string>
#include <sstream>
#include "../include/StompMessage.h"

class InputHandler {
private:
    static StompMessage parseLogin(std::stringstream& ss);
    static StompMessage parseLogout(std::stringstream& ss);
    static StompMessage parseJoin(std::stringstream& ss);
    static StompMessage parseExit(std::stringstream& ss);
    static StompMessage parseReport(std::stringstream& ss);
    static StompMessage parseSummary(std::stringstream& ss);

public:
    static StompMessage processUserInput(const std::string& input);
};
