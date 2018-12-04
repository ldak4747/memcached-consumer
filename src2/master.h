#pragma once
#include "worker.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <vector>

const int WORKERNUM = 100;

class Master {
    int epollfd;
    int serverfd;
    bool running = false;
    std::unique_ptr<std::thread> listener;
    std::vector<std::unique_ptr<Worker>> workers;
    int index = 0;
    const int workernum;
    
    void Listen(const char *, int);
public:
    Master(const char *host, int port, Task process, int slavenum = WORKERNUM);
    ~Master();
};
