#include <sys/un.h>
#include <sys/socket.h>
#include <cstring>

int initSocket() {
    int connectionSocketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connectionSocketFd == -1) {
        throw("Failed creating socket connection");
    }
    return connectionSocketFd;
}

const sockaddr* initSockAddr(sockaddr_un* sockAddr, char const* socketName) {
    memset(sockAddr, 0, sizeof(sockaddr_un));
    sockAddr->sun_family = AF_UNIX;
    strcpy(sockAddr->sun_path, socketName);
    return reinterpret_cast<const sockaddr*>(sockAddr);
}
