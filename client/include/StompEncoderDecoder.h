
#include <string>
#include <iostream>
#include "../include/StompMessage.h"

class StompEncoderDecoder
{
private:
    

public:
    StompEncoderDecoder();
    StompMessage processUserInput(std::string input);
};