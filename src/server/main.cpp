#include "Listener/listener.hpp"
#include "../shared/const.hpp"

int main () {
    Listener listener(SOCKET_NAME, SOCKET_CLIENT_HANDLE_CNT, BUFFER_SIZE);
    listener.execListen();
}