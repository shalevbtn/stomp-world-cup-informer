#pragma once

#include "../include/StompMessage.h"
#include <string>

class StompHelper{
    private:

    public:

        static StompMessage getLoginFrame(std::string hostPort, std::string user, std::string pass);
        static StompMessage getLogoutFrame(std::string input);
        static StompMessage getJoinFrame(std::string input);
        static StompMessage getReportFrame(std::string input);
        static StompMessage getExitFrame(std::string input);
        static StompMessage getSummaryFrame(std::string input);
};