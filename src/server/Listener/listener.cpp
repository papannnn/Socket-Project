#include "listener.hpp"
#include <sys/socket.h>
#include <cstring>
#include <sys/_select.h>
#include <unistd.h>
#include <unordered_map>
#include <../shared/type.hpp>
#include <iostream>
#include "../shared/helper.hpp"
#include <exception>

int Listener::nextId = 1;

Listener::Listener(char const* socketName_, const int &socketClientHandle_, const int &bufferSize_) : 
        socketName(socketName_), socketClientHandle(socketClientHandle_), bufferSize(bufferSize_) { 
    
    unlink(socketName);
    this->fdArr = std::vector<int>(socketClientHandle_ + 1, -1);
    this->sockAddr = new sockaddr_un();
}

void Listener::execListen() {
    int connectionSocketFd = initSocket();
    std::cout << "Master socket: " << connectionSocketFd << std::endl;
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
            handleClientRequest(connectionSocketFd);
        }
    }
}

void Listener::socketBind(int connectionSocketFd, const sockaddr* sockAddr) {
    int fd = bind(connectionSocketFd, sockAddr, sizeof(sockaddr_un));
    if (fd == -1) {
        throw std::runtime_error("Failed to bind socket");
    }
}

void Listener::initListen(int connectionSocketFd) {
    int fd = listen(connectionSocketFd, this->socketClientHandle);
    if (fd == -1) {
        throw std::runtime_error("Failed to listen socket");
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
        throw std::runtime_error("Failed to accept client");
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
    std::cout << "Sending data to " << clientFd << std::endl;
    int writeFd = write(clientFd, buffer, bufferSize);

    if (writeFd == -1) {
        throw("Error registering client");
    }

    std::cout << "Finished sending data to " << clientFd << std::endl;
}

int Listener::getSelectFdValue() {
    int result = -1;
    for (int i = 0 ; i < fdArr.size(); i++) {
        result = std::max(result, fdArr[i]);
    }
    return result + 1;
}

void Listener::handleClientRequest(int connectionSocketFd) {
    for (int i = 0; i < fdArr.size(); i++) {
        if (fdArr[i] == -1 || fdArr[i] == connectionSocketFd || !FD_ISSET(fdArr[i], &(this->fdSet))) {
            continue;
        }

        Payload payload = readClientBuffer(fdArr[i]);
        
        handlePayload(payload, fdArr[i]);
    }

    for (const auto &mapping : dataTree) {
        std::cout << mapping.second.id << " " << mapping.second.name << " " << mapping.second.age << std::endl;
    }
}

Payload Listener::readClientBuffer(int fdClient) {
    memset(buffer, 0, bufferSize);
    int fdRead = read(fdClient, buffer, bufferSize);
    // std::cout << fdRead <<std::endl;
    if (fdRead == -1) {
        throw std::runtime_error("Failed reading client buffer");
    }

    Payload* result = reinterpret_cast<Payload*>(buffer);
    return *result;
}

void Listener::handlePayload(Payload &payload, int fdClient) {
    int type = payload.type;
    int size = payload.length;
    // std::cout << "Type: " << type << std::endl;
    if (payload.type == TYPE_CREATE) {
        handleTypeCreate(payload);
    } else if (payload.type == TYPE_UPDATE) {
        std::cout << "INI MASUK" << std::endl;
        handleTypeUpdate(payload);
    } else if (payload.type == TYPE_DELETE) {
        handleTypeDelete(payload);
    } else if (payload.type == TYPE_CLOSE) {
        handleTypeClose(payload, fdClient);
        return;
    } else if (payload.type == TYPE_REFRESH) {
        std::cout << "HANDLED" << std::endl;
        std::cout << "Type: " << type << std::endl;
        std::cout << "Length :" << size <<std::endl;
        handleTypeRefresh(payload, fdClient);
    }
}

void Listener::handleTypeCreate(Payload &payload) {
    int length = payload.length;
    Person *person = reinterpret_cast<Person*>(payload.data);
    for (int i = 0; i < length; i++) {
        person[i].id = Listener::nextId++;
        dataTree.insert({person[i].id, person[i]});
    }
}

void Listener::handleTypeUpdate(Payload &payload) {
    int length = payload.length;
    Person *person = reinterpret_cast<Person*>(payload.data);
    for (int i = 0; i < length; i++) {
        dataTree.insert_or_assign(person[i].id, person[i]);
    }
}

void Listener::handleTypeDelete(Payload &payload) {
    int length = payload.length;
    Person *person = reinterpret_cast<Person*>(payload.data);
    for (int i = 0 ; i < length; i++) {
        dataTree.erase(person[i].id);
    }
}

void Listener::handleTypeClose(Payload &payload, int fdClient) {
    std::cout << "MASUK " << std::endl;
    std::cout << "Removing fd on" << fdClient << std::endl;
    removeFd(fdClient);
    close(fdClient);
}

void Listener::handleTypeRefresh(Payload &payload, int fdClient) {
    initNewData();
    sendNewData(fdClient);

}

Payload Listener::buildPayloadForRegister() {
    Payload payload = {};
    payload.type = TYPE_REFRESH;
    payload.length = dataTree.size();
    std::cout << "Ini size tree" << dataTree.size() << std::endl;
    std::vector<Person> arr;
    arr.reserve(payload.length);

    for (const auto &mapping : dataTree) {
        arr.push_back(mapping.second);
    }
    memset(payload.data, 0, DATA_SIZE);
    memcpy(payload.data, arr.data(), sizeof(Person) * payload.length);

    Person *arrPerson = reinterpret_cast<Person*>(payload.data);
    std::cout << "LOOPING" << std::endl;
    for (int i = 0 ; i < payload.length; i++) {
        std::cout << arrPerson[i].name << std::endl;
    }


    return payload;
}