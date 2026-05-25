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
    void handleClientRequest(void (*callback) (int fd));
    int getSelectFdValue();
    Payload readClientBuffer(int fdClient);
    std::vector<Payload> handlePayload(Payload &payload);
    std::vector<Payload> handleTypeCreate(Payload &payload);
    std::vector<Payload> handleTypeUpdate(Payload &payload);
    std::vector<Payload> handleTypeDelete(Payload &payload);
    std::unordered_map<int, std::vector<Person>> buildMappingPayload(std::vector<Payload> &payload);
};