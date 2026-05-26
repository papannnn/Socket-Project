#include "listener.hpp"
#include <sys/socket.h>
#include <cstring>
#include <sys/_select.h>
#include <unistd.h>
#include <unordered_map>
#include <../shared/type.hpp>
#include <iostream>

int Listener::nextId = 1;

Listener::Listener(char const* socketName_, const int &socketClientHandle_, const int &bufferSize_) : 
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
    Payload payload = buildPayloadForRegister();
    memset(buffer, 0, bufferSize);
    memcpy(buffer, &payload, sizeof(Payload));
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
    std::unordered_map<int, Payload> sendBackMap;
    for (int i = 0; i < fdArr.size(); i++) {
        if (!FD_ISSET(fdArr[i], &(this->fdSet))) {
            continue;
        }

        Payload payload = readClientBuffer(fdArr[i]);

        std::pair<int, std::vector<Person>> result = handlePayload(payload);
        insertPersonVectorToMapping(sendBackMap, result);
    }

    for (int i = 0; i < fdArr.size(); i++) {
        if (!FD_ISSET(fdArr[i], &(this->fdSet))) {
            continue;
        }

        for (const auto mapping : sendBackMap) {
            memset(buffer, 0, bufferSize);
            memcpy(buffer, &(mapping.second), sizeof(Payload));
            int writeFd = write(fdArr[i], buffer, bufferSize);
            if (writeFd == -1) {
                throw ("Failed to write a buffer");
            }
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

std::pair<int, std::vector<Person>> Listener::handlePayload(Payload &payload) {
    if (payload.type == TYPE_CREATE) {
        return std::make_pair(TYPE_CREATE, handleTypeCreate(payload));
    } else if (payload.type == TYPE_UPDATE) {
        return std::make_pair(TYPE_CREATE, handleTypeUpdate(payload));
    } else if (payload.type == TYPE_DELETE) {
        return std::make_pair(TYPE_CREATE, handleTypeDelete(payload));
    }

    return std::make_pair(-1, std::vector<Person>());
}

std::vector<Person> Listener::handleTypeCreate(Payload &payload) {
    std::vector<Person> sendBack;
    int length = payload.length;
    for (int i = 0; i < length; i++) {
        Person *person = reinterpret_cast<Person*>(payload.data);
        person->id = Listener::nextId++;
        dataTree.insert({person->id, *person});

        memcpy(payload.data, person, bufferSize);
        sendBack.push_back(*person);
    }
    return sendBack;
}

std::vector<Person> Listener::handleTypeUpdate(Payload &payload) {
    std::vector<Person> sendBack;
    int length = payload.length;
    for (int i = 0; i < length; i++) {
        Person *person = reinterpret_cast<Person*>(payload.data);
        dataTree.insert({person->id, *person});

        memcpy(payload.data, person, bufferSize);
        sendBack.push_back(*person);
    }
    return sendBack;
}

std::vector<Person> Listener::handleTypeDelete(Payload &payload) {
    std::vector<Person> sendBack;
    int length = payload.length;
    for (int i = 0 ; i < length; i++) {
        Person *person = reinterpret_cast<Person*>(payload.data);
        dataTree.erase(person->id);

        sendBack.push_back(*person);
    }
    return sendBack;
}

void Listener::insertPersonVectorToMapping(std::unordered_map<int, Payload> &mapping,
     std::pair<int, std::vector<Person>> &sendBackPair) {
    
    Payload &curr = mapping[sendBackPair.first];
    int currSize = curr.length;
    Person *currArrPos = reinterpret_cast<Person*>(curr.data);
    currArrPos += currSize;

    memcpy(currArrPos, sendBackPair.second.data(), sizeof(Person) * currSize);
    curr.length += sendBackPair.second.size();
}

Payload Listener::buildPayloadForRegister() {
    Payload payload;
    payload.type = TYPE_REGISTER;
    payload.length = dataTree.size();
    std::vector<Person> arr;
    arr.reserve(payload.length);

    for (const auto &mapping : dataTree) {
        arr.push_back(mapping.second);
    }
    memcpy(payload.data, arr.data(), sizeof(Person) * payload.length);
    return payload;
}