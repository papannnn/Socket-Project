#ifndef __HELPER_SOCKET_PROJECT_SHARED__
#define __HELPER_SOCKET_PROJECT_SHARED__

#include <sys/socket.h>
#include <sys/un.h>

int initSocket();
const sockaddr* initSockAddr(sockaddr_un* sockAddr, char const* socketName);

#endif