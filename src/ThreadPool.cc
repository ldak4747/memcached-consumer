#include "ThreadPool.h"

ThreadPool::ThreadPool (Cb callback, int workernum): worker_num(workernum) {
    running.store(true);
    cb.reset(new std::thread([this, callback] () {
        while (running.load()) {
            std::unique_lock<std::mutex> lock(cbmtx);
            if (!retqueue.empty()) {
                Result result = std::move(retqueue.front());
                retqueue.pop();
                lock.unlock();
                callback(result);
            }
        }
    }));

    for (int i = 0; i < worker_num; i++) {
        workers.emplace_back([this] () {
            while (running.load()) {
                std::unique_lock<std::mutex> lock(mtx);
                cond.wait(lock, [this] {
                    return !running.load() || !tasks.empty();
                });

                if (!running.load()) {
                    return;
                }

                Task newtask = std::move(tasks.front());
                tasks.pop();
                lock.unlock();

                newtask();
            }
        });
    }
}

ThreadPool::~ThreadPool () {
    running.store(false);
    cond.notify_all();

    for (auto &th: workers) {
        if (th.joinable()) {
            th.join();
        }
    }

    cb->join();
}

