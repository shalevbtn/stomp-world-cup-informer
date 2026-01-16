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

    void parse(std::string msg);

public:

    StompMessage(std::string msg);

    std::string getCommand();
    std::string getHeader(std::string headerKey);
    std::string getBody();

};

#endif
