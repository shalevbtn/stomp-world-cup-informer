#include "../include/StompMessage.h"

StompMessage::StompMessage(std::string rawMessage){
    parse(rawMessage);
}

StompMessage::StompMessage(std::string cm, std::map<std::string, std::string> hd, std::string bd) : command(cm), headers(hd), body(bd) {}

void StompMessage::parse(std::string msg){
    std::stringstream ss(msg);
    std::getline(ss, command);

    std::string line;
    while (std::getline(ss, line) && !line.empty()) {
        std::size_t colonPos = line.find(':');
        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);
        headers[key] = value;
    }

    std::stringstream bodyStream; 
    bodyStream << ss.rdbuf();
    body = bodyStream.str();
}

std::string StompMessage::getCommand() {
    return command;
}

std::string StompMessage::getHeader(std::string headerKey){
    return headers[headerKey];
}

std::string StompMessage::getBody(){
    return body;
}

void StompMessage::setBody(std::string body) {
    this->body = body;
}

void StompMessage::addHeader(std::string key, std::string value) {
    headers[key] = value;
}

void StompMessage::removeHeader(std::string key) {
    headers.erase(key);
}

std::string StompMessage::toString() const {
    std::string result;
    result += command +"\n";
    for (const auto& pair : headers) {
        result += pair.first + ":" + pair.second + "\n";
    }
    result += "\n";
    result += body;
    return result;
}