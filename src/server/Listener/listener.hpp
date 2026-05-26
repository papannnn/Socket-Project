#ifndef __LISTENER_SOCKET_PROJECT_SERVER__
#define __LISTENER_SOCKET_PROJECT_SERVER__

#include <../shared/type.hpp>
#include <sys/un.h>
#include <sys/socket.h>
#include <vector>
#include <map>

class Listener {
public:
    Listener(char const * socketName_, const int &socketClientHandle, const int &bufferSize);
    void execListen();
private:
    char const* socketName;
    int socketClientHandle;
    int bufferSize;
    std::vector<int> fdArr;
    fd_set fdSet;
    sockaddr_un* sockAddr;
    char buffer[BUFFER_SIZE];
    
    std::map<int, Person> dataTree;
    static int nextId;

    void socketBind(int connectionSocket, const sockaddr* sockAddr);
    void initListen(int socketFd);
    void insertFdToArrayFd(int fd);
    void refreshFdSet();
    int acceptClient(int connectionSocket);
    void registerClient(int clientFd);
    void handleClientRequest();
    int getSelectFdValue();
    Payload readClientBuffer(int fdClient);
    void handlePayload(Payload &payload, int fdClient);
    void handleTypeCreate(Payload &payload);
    void handleTypeUpdate(Payload &payload);
    void handleTypeDelete(Payload &payload);
    void handleTypeClose(Payload &payload, int fdClient);
    void removeFd(int fd);
    void insertPersonVectorToMapping(std::unordered_map<int, Payload> &mapping, std::pair<int, std::vector<Person>> &sendBackPair);
    Payload buildPayloadForRegister();
    void initNewData();
    void sendNewData(int clientFd);
};

#endif