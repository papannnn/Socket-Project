#include "listener.hpp"
#include <sys/socket.h>
#include <cstring>
#include <sys/_select.h>
#include <unistd.h>
#include <../shared/type.hpp>

Listener::Listener(char* socketName_, const int &socketClientHandle_, const int &bufferSize_) : 
        socketName(socketName_), socketClientHandle(socketClientHandle_), bufferSize(bufferSize_) { 
    
    unlink(socketName);
    this->fdArr = std::vector<int>(socketClientHandle_ + 1, -1);
    this->sockAddr = new sockaddr_un();
}

void Listener::execListen() {
    int connectionSocketFd = initSocket();
    const sockaddr* sockAddr = initSockAddr();
    socketBind(connectionSocketFd, sockAddr);
    initListen(connectionSocketFd);

    insertFdToArrayFd(connectionSocketFd);
    while (true) {
        refreshFdSet();
        select(getSelectFdValue(), &(this->fdSet), NULL, NULL, NULL);

        if (FD_ISSET(connectionSocketFd, &(this->fdSet))) {
            int fdClient = accept(connectionSocketFd, NULL, NULL);
            registerClient(fdClient);
        } else {
            scanFdSet([] (int fd) {
                this->handlePayload();
            });
        }
    }
}

int Listener::initSocket() {
    int connectionSocketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connectionSocketFd == -1) {
        throw("Failed creating socket connection");
    }
    return connectionSocketFd;
}

const sockaddr* Listener::initSockAddr() {
    sockAddr->sun_family = AF_UNIX;
    strcpy(sockAddr->sun_path, this->socketName);
    return reinterpret_cast<const sockaddr*>(sockAddr);
}

void Listener::socketBind(int connectionSocketFd, const sockaddr* sockAddr) {
    int fd = bind(connectionSocketFd, sockAddr, sizeof(sockaddr_un));
    if (fd == -1) {
        throw("Failed to bind socket");
    }
}

void Listener::initListen(int connectionSocketFd) {
    int fd = listen(connectionSocketFd, this->socketClientHandle);
    if (fd == -1) {
        throw("Failed to listen socket");
    }
}

void Listener::insertFdToArrayFd(int fd) {
    for (int i = 0; i < fdArr.size(); i++) {
        if (fdArr[i] == -1) {
            fdArr[i] = fd;
            break;
        }
    }
}

void Listener::refreshFdSet() {
    FD_ZERO(&(this->fdSet));
    for (int i = 0; i < fdArr.size(); i++) {
        if (fdArr[i] != -1) {
            FD_SET(fdArr[i], &(this->fdSet));
        }
    }
}

int Listener::acceptClient(int connectionSocket) {
    int clientFd = accept(connectionSocket, NULL, NULL);
    if (clientFd == -1) {
        throw("Failed to accept client");
    }
    return clientFd;
}

void Listener::registerClient(int clientFd) {
    insertFdToArrayFd(clientFd);
    // Send data

}

int Listener::getSelectFdValue() {
    int result = -1;
    for (int i = 0 ; i < fdArr.size(); i++) {
        result = std::max(result, fdArr[i]);
    }
    return result + 1;
}

void Listener::scanFdSet(void (*callback)(int fd)) {
    
    for (int i = 0; i < fdArr.size(); i++) {
        if (FD_ISSET(fdArr[i], &(this->fdSet))) {
            Payload payload = readClientBuffer(fdArr[i]);
            callback(fdArr[i]);
        }
    }
}

Payload Listener::readClientBuffer(int fdClient) {
    memset(buffer, 0, bufferSize);
    int fdRead = read(fdClient, buffer, bufferSize);
    if (fdRead == -1) {
        throw("Failed reading client buffer");
    }

    Payload* result = reinterpret_cast<Payload*>(buffer);
    return *result;
}

void Listener::handlePayload(Payload &payload) {
    if (payload.type == TYPE_CREATE) {
        handleTypeCreate(payload);
    } else if (payload.type == TYPE_UPDATE) {
        handleTypeUpdate(payload);
    } else if (payload.type == TYPE_DELETE) {
        handleTypeDelete(payload);
    }
}

void Listener::handleTypeCreate(Payload &payload) {
    int length = payload.length;
    for (int i = 0; i < length; i++) {
        Person *person = reinterpret_cast<Person*>(payload.data);
        person->id = Listener::nextId++;
        dataTree.insert({person->id, *person});
    }
}

void Listener::handleTypeUpdate(Payload &payload) {
    int length = payload.length;
    for (int i = 0; i < length; i++) {
        Person *person = reinterpret_cast<Person*>(payload.data);
        dataTree.insert({person->id, *person});
    }
}

void Listener::handleTypeDelete(Payload &payload) {
    int length = payload.length;
    for (int i = 0 ; i < length; i++) {
        Person *person = reinterpret_cast<Person*>(payload.data);
        dataTree.erase(person->id);
    }
}