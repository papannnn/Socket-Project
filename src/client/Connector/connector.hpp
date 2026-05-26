#ifndef __CONNECTOR_SOCKET_PROJECT_CLIENT__
#define __CONNECTOR_SOCKET_PROJECT_CLIENT__

#include <map>
#include <../../shared/type.hpp>
#include <../../shared/const.hpp>
#include <sys/un.h>

class Connector {
public:
    Connector(char const* socketName_);
    void exec();
private:
    char const *socketName;
    std::map<int, Person> dataTree;
    sockaddr_un* sockAddr;
    char buffer[BUFFER_SIZE];

    int initSocketConnection();
    void showTitle();
    void showMenu(int socketConnectionFd);
    void readAndUpdateDataTree(int socketConnectionFd);
    void showData();
    void showPrompt();
    void handlePrompt(int &choose, int socketConnectionFd);
    void triggerRefresh(int socketConnectionFd);
    void writeRefreshPayload(int socketConnectionFd);
    void handleCreate(int socketConnectionFd);
    void handleUpdate(int socketConnectionFd);
    void handleDelete(int socketConnectionFd);
    void handleExit(int socketConnectionFd);
    void writePayload(Payload &payload, int socketConnectionFd); 
};

#endif