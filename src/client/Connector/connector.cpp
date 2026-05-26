#include "connector.hpp"
#include "../../shared/helper.hpp"
#include "../../shared/type.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

Connector::Connector(char const* socketName_) : socketName(socketName_) {
    this->sockAddr = new sockaddr_un();
}

void Connector::exec() {
    std::cout << "HELLO" << std::endl;
    int socketConnectionFd = initSocketConnection();
    std::cout << "HELLO" << std::endl;
    readAndUpdateDataTree(socketConnectionFd);
    std::cout << "HELLO" << std::endl;
    showMenu(socketConnectionFd);
    std::cout << "HELLO" << std::endl;
}

int Connector::initSocketConnection() {
    int socketConnectionFd = initSocket();
    std::cout <<socketConnectionFd << std::endl;
    const sockaddr* sockAddr = initSockAddr(this->sockAddr, socketName);
    std::cout << "HELLO" << std::endl;
    int connectFd = connect(socketConnectionFd, sockAddr, sizeof(sockaddr_un));
    std::cout <<connectFd << std::endl;
    if (connectFd == -1) {
        throw("Failed to connect to server");
    }
    return socketConnectionFd;
}

void Connector::readAndUpdateDataTree(int socketConnectionFd) {
    memset(buffer, 0, BUFFER_SIZE);
    int readFd = read(socketConnectionFd, buffer, BUFFER_SIZE);
    if (readFd == -1) {
        throw("Failed to read to server");
    }
    Payload *payload = reinterpret_cast<Payload*>(buffer);
    if (payload->type != TYPE_REFRESH) {
        return;
    }

    dataTree.clear();
    Person *data = reinterpret_cast<Person*>(payload->data);
    for (int i = 0; i < payload->length; i++) {
        dataTree.insert({data->id, *data});
        data++;
    }
}

void Connector::triggerRefresh(int socketConnectionFd) {
    writeRefreshPayload(socketConnectionFd);
    readAndUpdateDataTree(socketConnectionFd);
}

void Connector::writeRefreshPayload(int socketConnectionFd) {
    Payload payload;
    payload.type = TYPE_REFRESH;
    memset(buffer, 0, BUFFER_SIZE);
    memcpy(buffer, &payload, sizeof(Payload));
    int writeFd = write(socketConnectionFd, buffer, BUFFER_SIZE);
    if (writeFd == -1) {
        throw ("Failed writing to socket");
    }
}

void Connector::showTitle() {
    std::cout << "  _________________  _________  ____  __.______________________ ____________________ ________        ____._______________________________" << std::endl;
    std::cout << " /   _____/\\_____  \\ \\_   ___ \\|    |/ _|\\_   _____/\\__    ___/ \\______   \\______   \\\\_____  \\      |    |\\_   _____/\\_   ___ \\__    ___/" << std::endl;
    std::cout << " \\_____  \\  /   |   \\/    \\  \\/|      <   |    __)_   |    |     |     ___/|       _/ /   |   \\     |    | |    __)_ /    \\  \\/ |    |   " << std::endl;
    std::cout << " /        \\/    |    \\     \\___|    |  \\  |        \\  |    |     |    |    |    |   \\/    |    \\/\\__|    | |        \\\\     \\____|    |   " << std::endl;
    std::cout << "/_______  /\\_______  /\\______  /____|__ \\/_______  /  |____|     |____|    |____|_  /\\_______  /\\________|/_______  / \\______  /|____|   " << std::endl;
    std::cout << "        \\/         \\/        \\/        \\/        \\/                               \\/         \\/                   \\/         \\/          " << std::endl;
    std::cout << std::endl << std::endl << std::endl;
}

void Connector::showMenu(int socketConnectionFd) {
    int choose = -1;
    while (choose != 4) {
        triggerRefresh(socketConnectionFd);
        showTitle();
        showData();
        showPrompt();
        handlePrompt(choose, socketConnectionFd);
    }
}

void Connector::showData() {
    if (dataTree.size() == 0) {
        std::cout << "No Data" << std::endl;
        return;
    }

    for (const auto &mapping : dataTree) {
        std::cout << mapping.second.id << " " << mapping.second.name << " " << mapping.second.age << std::endl;
    }
}

void Connector::showPrompt() {
    std::cout << "1. Create Data" << std::endl;
    std::cout << "2. Update Data" << std::endl;
    std::cout << "3. Delete Data" << std::endl;
    std::cout << "4. Exit" << std::endl;
    std::cout << "Choose [1-4]: ";
}

void Connector::handlePrompt(int &choose, int socketConnectionFd) {
    std::cin >> choose;
    if (choose == 1) {
        handleCreate(socketConnectionFd);
    } else if (choose == 2) {
        handleUpdate(socketConnectionFd);
    } else if (choose == 3) {
        handleDelete(socketConnectionFd);
    } else if (choose == 4) {
        handleExit(socketConnectionFd);
    }
}

void Connector::handleCreate(int socketConnectionFd) {

}

void Connector::handleUpdate(int socketConnectionFd) {
    
}

void Connector::handleDelete(int socketConnectionFd) {
    
}

void Connector::handleExit(int socketConnectionFd) {
    memset(buffer, 0, BUFFER_SIZE);
    Payload payload;
    payload.type = TYPE_CLOSE;
    payload.length = 1;
    memcpy(buffer, &payload, sizeof(Payload));
    int writeFd = write(socketConnectionFd, buffer, BUFFER_SIZE);
    std::cout << writeFd << std::endl;
    if (writeFd == -1) {
        throw ("Failed write to socket exit");
    }

    close(socketConnectionFd);
}