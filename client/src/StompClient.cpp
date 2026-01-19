#include <stdlib.h>
#include "../include/ConnectionHandler.h"
#include <thread>
#include <iostream>
#include <atomic>
#include "../include/StompProtocol.h"
#include "../include/InputHandler.h"

int main(int argc, char *argv[]) {
    StompProtocol protocol;
    ConnectionHandler* connectionHandler = nullptr;
    std::atomic<bool> shouldClose{false};
    std::thread reader;

    while(!shouldClose) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        std::string line(buf);

        if (line.empty()) continue;

        StompMessage userRequest = InputHandler::processUserInput(line);
        protocol.clearFrames();
        std::vector<StompMessage> framesToSend = protocol.process(userRequest);

        if (line.find("login") == 0 && !protocol.isLoggedIn() && framesToSend.size() > 0) {
            if (reader.joinable()) 
                reader.join();

            std::stringstream ss(line);
            std::string cmd, hostPort;
            ss >> cmd >> hostPort;
            
            size_t colonPos = hostPort.find(':');
            std::string host = hostPort.substr(0, colonPos);
            short port = (short)stoi(hostPort.substr(colonPos + 1));

            connectionHandler = new ConnectionHandler(host, port);
            if (!connectionHandler->connect()) {
                std::cerr << "Could not connect to server" << std::endl;
                delete connectionHandler;
                connectionHandler = nullptr;
                continue;
            }
            
            protocol.setLoggedIn(true);

            reader = std::thread([connectionHandler, &protocol, &shouldClose]() {
                while(!shouldClose) {
                    std::string frame;
                    if (!connectionHandler->getFrameAscii(frame, '\0')) {
                        std::cout << "Disconnected." << std::endl;
                        shouldClose = true;
                        protocol.setLoggedIn(false);
                        break;
                    }
                    if (!protocol.processServerResponse(frame)) {
                        shouldClose = true;
                    }
                }
            });
        }
        
        if (connectionHandler && protocol.isLoggedIn()) {
            for (const auto& frame : framesToSend) {
                connectionHandler->sendFrameAscii(frame.toString(), '\0');
            }
        }
    }

    if (reader.joinable()) reader.join();
    if (connectionHandler) delete connectionHandler;
    return 0;
}