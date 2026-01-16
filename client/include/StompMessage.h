#ifndef STOMPMESSAGE
#define STOMPMESSAGE

#include <string>
#include <map>
#include <sstream>

class StompMessage
{
private:

    std::string command;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string username;

    void parse(std::string msg);

    void handleLogin(std::stringstream& ss);
    void handleLogout(std::stringstream& ss);
    void handleJoin(std::stringstream& ss);
    void handleExit(std::stringstream& ss);
    void handleReport(std::stringstream& ss);
    void handleSummary(std::stringstream& ss);


public:

    StompMessage(std::string msg);
    std::string getHeader(std::string headerKey);
    std::string getBody();

};

#endif
