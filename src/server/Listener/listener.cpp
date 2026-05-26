#include "listener.hpp"
#include <sys/socket.h>
#include <cstring>
#include <sys/_select.h>
#include <unistd.h>
#include <unordered_map>
#include <../shared/type.hpp>
#include <iostream>
#include "../shared/helper.hpp"

int Listener::nextId = 1;

Listener::Listener(char const* socketName_, const int &socketClientHandle_, const int &bufferSize_) : 
        socketName(socketName_), socketClientHandle(socketClientHandle_), bufferSize(bufferSize_) { 
    
    unlink(socketName);
    this->fdArr = std::vector<int>(socketClientHandle_ + 1, -1);
    this->sockAddr = new sockaddr_un();
}

void Listener::execListen() {
    int connectionSocketFd = initSocket();
    const sockaddr* sockAddr = initSockAddr(this->sockAddr, socketName);
    socketBind(connectionSocketFd, sockAddr);
    initListen(connectionSocketFd);

    insertFdToArrayFd(connectionSocketFd);
    std::cout << "Listening..." << std::endl;
    while (true) {
        refreshFdSet();
        select(getSelectFdValue(), &(this->fdSet), NULL, NULL, NULL);

        if (FD_ISSET(connectionSocketFd, &(this->fdSet))) {
            int fdClient = accept(connectionSocketFd, NULL, NULL);
            registerClient(fdClient);
        } else {
            handleClientRequest();
        }
    }
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

void Listener::removeFd(int fd) {
    for (int i = 0; i < fdArr.size(); i++) {
        if (fdArr[i] == fd) {
            fdArr[i] = -1;
            break;
        }
    }

    FD_CLR(fd, &(fdSet));
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
    initNewData();
    sendNewData(clientFd);
}

void Listener::initNewData() {
    Payload payload = buildPayloadForRegister();
    memset(buffer, 0, bufferSize);
    memcpy(buffer, &payload, sizeof(Payload));
}

void Listener::sendNewData(int clientFd) {
    int writeFd = write(clientFd, buffer, bufferSize);
    if (writeFd == -1) {
        throw("Error registering client");
    }
}

int Listener::getSelectFdValue() {
    int result = -1;
    for (int i = 0 ; i < fdArr.size(); i++) {
        result = std::max(result, fdArr[i]);
    }
    return result + 1;
}

void Listener::handleClientRequest() {
    for (int i = 0; i < fdArr.size(); i++) {
        if (!FD_ISSET(fdArr[i], &(this->fdSet))) {
            continue;
        }

        Payload payload = readClientBuffer(fdArr[i]);

        handlePayload(payload, fdArr[i]);
    }

    initNewData();
    for (int i = 0; i < fdArr.size(); i++) {
        if (!FD_ISSET(fdArr[i], &(this->fdSet))) {
            continue;
        }

        sendNewData(fdArr[i]);
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

void Listener::handlePayload(Payload &payload, int fdClient) {
    std::cout << "Type: " << payload.type << std::endl;
    if (payload.type == TYPE_CREATE) {
        handleTypeCreate(payload);
    } else if (payload.type == TYPE_UPDATE) {
        handleTypeUpdate(payload);
    } else if (payload.type == TYPE_DELETE) {
        handleTypeDelete(payload);
    } else if (payload.type == TYPE_CLOSE) {
        handleTypeClose(payload, fdClient);
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

void Listener::handleTypeClose(Payload &payload, int fdClient) {
    std::cout << "MASUK " << std::endl;
    removeFd(fdClient);
    close(fdClient);
}

Payload Listener::buildPayloadForRegister() {
    Payload payload;
    payload.type = TYPE_REFRESH;
    payload.length = dataTree.size();
    std::vector<Person> arr;
    arr.reserve(payload.length);

    for (const auto &mapping : dataTree) {
        arr.push_back(mapping.second);
    }
    memcpy(payload.data, arr.data(), sizeof(Person) * payload.length);
    return payload;
}