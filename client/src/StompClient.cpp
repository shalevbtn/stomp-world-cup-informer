#include <stdlib.h>
#include "../include/ConnectionHandler.h"
#include <thread>
#include <iostream>
#include <atomic>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl;
        return -1;
    }
    std::string host = argv[1];
    short port = atoi(argv[2]);
    
    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }
    std::cout << "Connected! Type 'login' to authenticate." << std::endl;

    std::atomic<bool> shouldClose{false};

    // 1. Socket Reader Thread
    std::thread reader([&connectionHandler, &shouldClose]() {
        while(!shouldClose) {
            std::string line;
            if (!connectionHandler.getLine(line)) { 
                std::cout << "Disconnected from server." << std::endl;
                shouldClose = true;
                break;
            }
            std::cout << "Server Response: " << line << std::endl;
        }
    });

    // 2. Keyboard Input Thread
    while(!shouldClose) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        std::string line(buf);

        if (line == "login") {
            // --- FIX IS HERE ---
            // Construct a valid STOMP frame
            std::string frame = "CONNECT\n"
                                "accept-version:1.2\n"
                                "host:stomp.cs.bgu.ac.il\n"
                                "login:meni\n"
                                "passcode:films\n"
                                "\n"; // End of headers
            
            // Send the frame followed by the NULL character
            connectionHandler.sendFrameAscii(frame, '\0'); 
            // -------------------
        } else {
            // For other commands, we just send them as lines for now (won't work until you implement more)
            connectionHandler.sendLine(line); 
        }
    }

    if (reader.joinable()) reader.join();
    return 0;
}