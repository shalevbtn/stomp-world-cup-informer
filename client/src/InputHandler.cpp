#include "../include/InputHandler.h"
#include <map>

StompMessage InputHandler::processUserInput(const std::string& input) {
    std::stringstream ss(input);
    std::string command;
    ss >> command;

    if (command == "login")
        return handleLogin(ss);

    if (command == "join")
        return handleJoin(ss);

    if (command == "exit")
        return handleExit(ss);

    if (command == "report")
        return handleReport(ss);

    if (command == "summary")
        return handleSummary(ss);

    if (command == "logout")
        return handleLogout(ss);

    return StompMessage("INVALID", {}, "Invalid command");
}


// login {host:port} {username} {password}
StompMessage InputHandler::handleLogin(std::stringstream& ss) {
    std::string hostPort, user, pass;
    if (!(ss >> hostPort >> user >> pass))
        return StompMessage("INVALID", {}, "Invalid login arguments");

    std::map<std::string, std::string> headers;
    headers["host"] = hostPort;
    headers["login"] = user;
    headers["passcode"] = pass;

    return StompMessage("CONNECT", headers, "");
}

// join {game_name}
StompMessage InputHandler::handleJoin(std::stringstream& ss) {
    std::string game;
    if (!(ss >> game))
        return StompMessage("INVALID", {}, "Invalid join arguments");

    std::map<std::string, std::string> headers;
    headers["destination"] = game;

    return StompMessage("SUBSCRIBE", headers, "");
}

// exit {game_name}
StompMessage InputHandler::handleExit(std::stringstream& ss) {
    std::string game;
    if (!(ss >> game))
        return StompMessage("INVALID", {}, "Invalid exit arguments");

    std::map<std::string, std::string> headers;
    headers["destination"] = game;

    return StompMessage("UNSUBSCRIBE", headers, "");
}

// report {file}
StompMessage InputHandler::handleReport(std::stringstream& ss) {
    std::string file;
    if (!(ss >> file))
        return StompMessage("INVALID", {}, "Invalid report arguments");

    std::map<std::string, std::string> headers;
    headers["file_path"] = file;

    // Protocol will translate this to multiple SEND frames
    return StompMessage("SEND", headers, "");
}

// summary {game_name} {user} {file}
StompMessage InputHandler::handleSummary(std::stringstream& ss) {
    std::string game, user, file;
    if (!(ss >> game >> user >> file))
        return StompMessage("INVALID", {}, "Invalid summary arguments");

    std::map<std::string, std::string> headers;
    headers["game_name"] = game;
    headers["user"] = user;
    headers["file"] = file;

    return StompMessage("SUMMARY", headers, "");
}

StompMessage InputHandler::handleLogout(std::stringstream&) {
    return StompMessage("DISCONNECT", {}, "");
}
