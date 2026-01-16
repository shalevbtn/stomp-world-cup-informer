
#include <string>
#include <iostream>
#include "../include/StompMessage.h"

class StompEncoderDecoder
{
private:
    

public:

    static StompMessage Login(std::string input);
    static StompMessage Logout(std::string input);
    static StompMessage Join(std::string input);
    static StompMessage Report(std::string input);
    static StompMessage Exit(std::string input);
    static StompMessage Summary(std::string input);
};