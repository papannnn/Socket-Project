#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include "Connector/connector.hpp"
#include "../shared/const.hpp"


int main () {
    
    Connector connector(SOCKET_NAME);
    try {
        connector.exec();
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    
}