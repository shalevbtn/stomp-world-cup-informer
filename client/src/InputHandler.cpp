#include "../include/InputHandler.h"
#include <map>

StompMessage InputHandler::processUserInput(const std::string& input) {
    std::stringstream ss(input);
    std::string command;
    ss >> command;

    if (command == "login") return parseLogin(ss);
    if (command == "join") return parseJoin(ss);
    if (command == "exit") return parseExit(ss);
    if (command == "report") return parseReport(ss);
    if (command == "logout") return parseLogout(ss);
    if (command == "summary") return parseSummary(ss);

    return StompMessage("INVALID", {}, "Unknown command");
}


StompMessage InputHandler::parseLogin(std::stringstream& ss) {
    std::string hostPort, username, pass;
    if (!(ss >> hostPort >> username >> pass))
        return StompMessage("INVALID", {}, "Invalid login arguments");

    std::map<std::string, std::string> headers;
    headers["accept-version"] = "1.2";         
    headers["host"] = "stomp.cs.bgu.ac.il"; 
    headers["login"] = username;
    headers["passcode"] = pass;

    return StompMessage("CONNECT", headers, "");
}

// join {game_name}
StompMessage InputHandler::parseJoin(std::stringstream& ss) {
    std::string game;
    if (!(ss >> game))
        return StompMessage("INVALID", {}, "Invalid join arguments");

    std::map<std::string, std::string> headers;
    headers["destination"] = "/" + game;

    return StompMessage("SUBSCRIBE", headers, "");
}

// exit {game_name}
StompMessage InputHandler::parseExit(std::stringstream& ss) {
    std::string game;
    if (!(ss >> game))
        return StompMessage("INVALID", {}, "Invalid exit arguments");

    std::map<std::string, std::string> headers;
    headers["destination"] = "/" + game;

    return StompMessage("UNSUBSCRIBE", headers, "");
}

// report {file}
StompMessage InputHandler::parseReport(std::stringstream& ss) {
    std::string file;
    if (!(ss >> file))
        return StompMessage("INVALID", {}, "Invalid report arguments");

    std::map<std::string, std::string> headers;
    headers["file_path"] = file;

    return StompMessage("SEND", headers, "");
}

// summary {game_name} {user} {file}
StompMessage InputHandler::parseSummary(std::stringstream& ss) {
    std::string game, user, file;
    if (!(ss >> game >> user >> file))
        return StompMessage("INVALID", {}, "Invalid summary arguments");

    std::map<std::string, std::string> headers;
    headers["game_name"] = game;
    headers["user"] = user;
    headers["file"] = file;

    return StompMessage("SUMMARY", headers, "");
}

StompMessage InputHandler::parseLogout(std::stringstream&) {
    return StompMessage("DISCONNECT", {}, "");
}
