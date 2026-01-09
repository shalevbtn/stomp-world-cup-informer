#include "ConnectionHandler.h"
#include <thread>
#include <iostream>
#include <string>
#include <atomic>

//Note: install socat: sudo apt install socat and run socat -v TCP-LISTEN:8080,reuseaddr,fork EXEC:'/bin/cat'

int main() {
    std::string host = "127.0.0.1";
    short port = 37167;
    ConnectionHandler connectionHandler(host, port);
    std::atomic<bool> shouldClose{false};
    if (!connectionHandler.connect()) {
        std::cerr << "Could not connect to server." << std::endl;
        return 1;
    }

    std::thread reader_thread([&connectionHandler, &shouldClose]() {
        while (!shouldClose.load()) {
            std::string line;
            if (!connectionHandler.getLine(line)) {
                // If reading fails (connection closed or error), break the loop
                if (!shouldClose.load())
                    std::cerr << "Failed to read from server." << std::endl;
                else
                    std::cout << "Connection closed." << std::endl;
                shouldClose.store(true);
                break;
            }
            std::cout << "<Server> " << line << std::endl;
        }
    });

    // Main thread: Read input from stdin and send to server
    while (!shouldClose.load()) {
        std::string input;
        if (!std::getline(std::cin, input)) {
            shouldClose.store(true);
            connectionHandler.close();
            break;
        }
        if (input == "exit") {
            shouldClose.store(true);
            connectionHandler.close();
            break;
        }
        if(!connectionHandler.sendLine(input) and !shouldClose.load()) {
            std::cerr << "Failed to send to server." << std::endl;
            shouldClose.store(true);
            connectionHandler.close();
            break;
        }
    }

    // Cleanup: Close connection and join reader thread
    connectionHandler.close();

    reader_thread.join();

    return 0;
}