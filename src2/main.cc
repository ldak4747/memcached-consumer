#include "master.h"
#include <chrono>
#include <signal.h>

response *process (const request *req) {
    int fd = req->fd;
    int len = req->len;
    char *data = req->data;

    std::cout << data << std::endl;

    char *respdata = new char[len * 10];
    for (int i = 0; i < 10; i++) {
        memcpy(respdata + i * len, req->data, len);
    }
    response *resp = new response(fd, len * 10, respdata);
    return resp;
}

int main (int argc, char *argv[]) {
    signal(SIGPIPE,SIG_IGN);
    Master master(argv[1], std::stoi(argv[2]), process);
    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(100));
    }
    return 0;
}
