#include "../include/StompEncoderDecoder.h"
#include <vector>

StompEncoderDecoder::StompEncoderDecoder() {}

StompMessage StompEncoderDecoder::processUserInput(std::string input) {
    std::vector<std::string> frames;
    std::stringstream ss(input);
    std::string command;
    ss >> command;

    //TODO: craete a stomp object
}