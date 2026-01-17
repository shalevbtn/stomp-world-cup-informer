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
    StompMessage(std::string cm, std::map<std::string, std::string> hd, std::string bd);
    std::string getCommand();
    std::string getHeader(std::string headerKey);
    std::string getBody();
    void setBody(std::string body);
    void addHeader(std::string key, std::string value);
    void removeHeader(std::string key);
    std::string toString() const;
};

#endif
