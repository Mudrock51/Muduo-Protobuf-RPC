#pragma once
#include <sys/unistd.h>

class SocketManager{
public:
    explicit SocketManager(int fd) : socketfd(fd) {}
    ~SocketManager(){
        if (socketfd == -1){
            close(socketfd);
        }
    }

private:
    int socketfd;
};