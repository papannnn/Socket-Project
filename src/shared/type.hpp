#ifndef __TYPE_SOCKET_PROJECT_SHARED__
#define __TYPE_SOCKET_PROJECT_SHARED__

#include "const.hpp"

#define TYPE_CREATE 1
#define TYPE_UPDATE 2
#define TYPE_DELETE 3
#define TYPE_CLOSE 4
#define TYPE_REFRESH 5

struct Payload {
    char type;
    char length;
    char data[DATA_SIZE];
};

struct Person {
    int id;
    char name[20];
    int age;
};

#endif