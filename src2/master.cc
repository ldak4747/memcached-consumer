#include "master.h"

Master::Master (const char *host, int port, Task process, int slavenum): workernum(slavenum) {
    for (int i = 0; i < workernum; i++) {
        workers.emplace_back(new Worker(process));
    }

    listener.reset(new std::thread([this, host, port] () {
        Listen(host, port);
        running = true;

        epollfd = epoll_create(MAXEVENT);
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = serverfd;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &ev);

        struct epoll_event evs[1024];

        while (running) {
            int cnt = epoll_wait(epollfd, evs, 1024, timeval);
            if (cnt) {
                for (int i = 0; i < cnt; i++) {
                    if (evs[i].data.fd == serverfd) {
                        socklen_t addrlen;
                        struct sockaddr_in;
                        int newfd = accept(serverfd, (struct sockaddr *)&addrlen, &addrlen);
                        while (newfd > 0) {
                            int pipefd = workers[index++]->getPipewritefd();
                            write(pipefd, (void *)&newfd, 4);
                            if (index == workers.size()) {
                                index = 0;
                            }
                            newfd = accept(serverfd, (struct sockaddr *)&addrlen, &addrlen);
                        }
                    }       
                }
                    
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = serverfd;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &ev);
            }
        }

    }));
}

Master::~Master () {
    running = true;
    listener->join();
    close(serverfd);
    close(epollfd);
}

void Master::Listen (const char *host, int port) {
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > serverfd) {
        throw std::runtime_error("socket");
    }

    int reuse = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    int opt = fcntl(serverfd, F_GETFL);
    fcntl(serverfd, F_SETFL, opt | O_NONBLOCK);

    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(host);
    srv.sin_port = htons(port);

    if (0 > bind(serverfd, (struct sockaddr *)&srv, sizeof(srv))) {
        throw std::runtime_error("bind");
    }

    listen(serverfd, 128);
}
