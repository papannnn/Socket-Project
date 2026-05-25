#include <../shared/type.hpp>
#include <sys/un.h>
#include <vector>
#include <map>

class Listener {
public:
    Listener(char* socketName_, const int &socketClientHandle, const int &bufferSize);
    void execListen();
private:
    char* socketName;
    int socketClientHandle;
    int bufferSize;
    std::vector<int> fdArr;
    fd_set fdSet;
    sockaddr_un* sockAddr;
    char buffer[BUFFER_SIZE];
    
    std::map<int, Person> dataTree;
    static int nextId;

    int initSocket();
    const sockaddr* initSockAddr();
    void socketBind(int connectionSocket, const sockaddr* sockAddr);
    void initListen(int socketFd);
    void insertFdToArrayFd(int fd);
    void refreshFdSet();
    int acceptClient(int connectionSocket);
    void registerClient(int clientFd);
    void scanFdSet(void (*callback) (int fd));
    int getSelectFdValue();
    Payload readClientBuffer(int fdClient);
    void handlePayload(Payload &payload);
    void handleTypeCreate(Payload &payload);
    void handleTypeUpdate(Payload &payload);
    void handleTypeDelete(Payload &payload);
};