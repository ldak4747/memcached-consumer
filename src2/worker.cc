#include "worker.h"

Worker::Worker (Task process): running(true) {
    worker.reset(new std::thread([this, process] () {
        epollfd = epoll_create(MAXEVENT);
        if (0 > pipe(pipefd)) {
            throw std::runtime_error("pipe alloc error");
        }
        int opt = fcntl(pipefd[1], F_GETFL);
        fcntl(pipefd[1], F_SETFL, opt | O_NONBLOCK);

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        //ev.events = EPOLLIN;
        ev.data.fd = pipefd[0];
        epoll_ctl(epollfd, EPOLL_CTL_ADD, pipefd[0], &ev);
        struct epoll_event evs[1024];

        while (running) {
            int cnt = epoll_wait(epollfd, evs, 1024, timeval);
            for (int i = 0; i < cnt; i++) {
                if (evs[i].data.fd == pipefd[0] && evs[i].events & EPOLLIN) {
                    int newfd;
                    int len = read(pipefd[0], (void *)&newfd, 4);
                    sockinit(newfd);
                    ev.events = EPOLLIN | EPOLLET;
                    //ev.events = EPOLLIN;
                    ev.data.fd = newfd;
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, newfd, &ev);
                } else if (evs[i].events & EPOLLIN) {
                    int fd = evs[i].data.fd;
                    char *recvbuf = Read(fd);
                    if (recvbuf) {
                        request *req = new request(fd, strlen(recvbuf), recvbuf);
                        //response *resp = process(req);
                        int len = req->len;
                        char *respdata = new char[1000];
                        sprintf(respdata, "HTTP/1.0 200 OK\r\n\r\n Content-type: text/plain\r\n\r\nfuck");
                        response *resp = new response(fd, strlen(respdata), respdata);
                        Write(resp->fd, resp);
                        delete req;
                    }                    
                } else if (evs[i].events & EPOLLOUT) {
                    response *resp = static_cast<response *>(evs[i].data.ptr);
                    Write(resp->fd, resp);
                }
            }
        }
    }));

    close(pipefd[1]);
    close(pipefd[0]);
    close(epollfd);
}

Worker::~Worker () {
    running = false;
    if (worker->joinable()) {
        worker->join();
    }
}

void Worker::sockinit (int newfd) {
    int val = 1;
    setsockopt(newfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    setsockopt(newfd, SOL_SOCKET, SO_RCVBUF, (char *)&read_buffer_len, sizeof(read_buffer_len));
    setsockopt(newfd, SOL_SOCKET, SO_RCVBUF, (char *)&write_buffer_len, sizeof(write_buffer_len));

    int opt = fcntl(newfd, F_GETFL);
    fcntl(newfd, F_SETFL, opt | O_NONBLOCK);
}

char *Worker::Read (int fd) {
    int len = 0;
    char buf[read_buffer_len] = {0};
    while (1) {
        int curlen = recv(fd, buf + len, 1024, 0);
        if (curlen < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                break;
            }
//std::cout << "recv, fd " << fd << " : " << strerror(errno) << std::endl;
//std::cout.flush();
            close(fd);
            return nullptr;
        } else if (!curlen) {
//std::cout << "recv, peer close." << std::endl;
//std::cout.flush();
            close(fd);
            return nullptr;
        } else {
            len += curlen;
        }
    }

    char *recvbuf = new char[len + 1];
    memcpy(recvbuf, buf, len);
    recvbuf[len] = '\0';
    return recvbuf;
}

void Worker::Write (int fd, response *resp) {
    int curlen = 0;
    while (resp->relate) {
        int curlen = send(fd, resp->data + resp->written, resp->relate, 0);
        if (curlen < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                struct epoll_event ev;
                ev.events = EPOLLOUT | EPOLLET;
                //ev.events = EPOLLOUT;
                ev.data.ptr = resp;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
                return;
            } else {
                std::cout << "send, " << strerror(errno) << std::endl;
                delete resp;
                close(fd);
                return;
            }
        } else if (!curlen) {
            std::cout << "send, peer close." << std::endl;
            delete resp;
            close(fd);
            return;
        } else {
            resp->written += curlen;
            resp->relate -= curlen;
        }
    }

    delete resp;
    return;
}

