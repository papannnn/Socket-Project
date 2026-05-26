#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include "Connector/connector.hpp"
#include "../shared/const.hpp"


int main () {
    
    Connector connector(SOCKET_NAME);
    connector.exec();

    // int socketConnection = socket(AF_UNIX, SOCK_DGRAM, 0);
    // if (socketConnection == -1) {
    //     //
    // }

    // sockaddr_un sockAddr;
    // memset(&sockAddr, 0, sizeof(sockaddr_un));
    // sockAddr.sun_family = AF_UNIX;
    // strcpy(sockAddr.sun_path, SOCKET_NAME);

    // int connectFd = connect(socketConnection, reinterpret_cast<const sockaddr*>(&sockAddr), sizeof(sockaddr_un));
    // if (connectFd == -1) {
    //     //
    // }


}