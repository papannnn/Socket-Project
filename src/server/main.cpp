#include "Listener/listener.hpp"
#include <iostream>
#include "../shared/const.hpp"

int main () {
    Listener listener(SOCKET_NAME, SOCKET_CLIENT_HANDLE_CNT, BUFFER_SIZE);
    try {
        listener.execListen();
    } catch(char *e) {
        std::cout << e << std::endl;
    }
    
}