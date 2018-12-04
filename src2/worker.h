#pragma once
#include <memory>
#include <thread>
#include <functional>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <iostream>
    
const int timeval = 1;  //ms
const int MAXEVENT = 102400;
const int MAXREADBUFLEN = 102400;
const int MAXWRITEBUFLEN = 102400;

struct request {
    int fd;
    int len;
    char *data;
    request(int _fd, int _len, char *_data): fd(_fd), len(_len), data(_data) {}
    ~request() { delete []data; }
};
struct response {
    int fd;
    int written;
    int relate;
    char *data;
    response(int _fd, int _relate, char *_data, int _written = 0): fd(_fd), relate(_relate), data(_data), written(_written) {}
    ~response() { delete []data; }
};
using Task = std::function<response * (request *)>;

class Worker {
    int epollfd;
    int pipefd[2];

    const int read_buffer_len = MAXREADBUFLEN;
    const int write_buffer_len = MAXWRITEBUFLEN;

    std::unique_ptr<std::thread> worker;
    bool running = false;
 
    void sockinit(int newfd);
    char *Read(int fd);
    void Write(int fd, response *str);
public:
    Worker(Task process);
    ~Worker();
    int getPipewritefd() { return pipefd[1]; }
    int getPipereadfd() { return pipefd[0]; }
};
