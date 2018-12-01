#pragma once
#include <functional>
#include <future>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include "TriggerMessageBody.pb.h"

const int WORKERNUM = 96;
const int MAXQUEUELEN = 100000;
using Task = std::function<void ()>;
using Result = std::tuple<bool, std::string, std::shared_ptr<weibo::TriggerMessageBody>>;

using Cb = std::function<void (Result result)>;


class ThreadPool {
    int worker_num;
    std::vector<std::thread> workers;
    std::queue<Task> tasks;

    std::unique_ptr<std::thread> cb;
    std::queue<Result> retqueue;

    std::mutex mtx;
    std::mutex cbmtx;
    std::condition_variable cond;
    std::atomic<bool> running;

public:
    ThreadPool(Cb callback, int workernum = WORKERNUM);
    ~ThreadPool();
    template<class Func, class ...Args> bool AddTask(Func &&func, Args &&...args) {
        if (running.load()) {
            {
                std::unique_lock<std::mutex> lock(mtx);
                if (tasks.size() > MAXQUEUELEN) {
                    lock.unlock();
                    return false;
                }
            }

            //using Retype = decltype(func(args...));
            auto task = std::make_shared<std::packaged_task<Result ()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
            //std::future<std::string> future = task->get_future();

            std::unique_lock<std::mutex> lock(mtx);
            tasks.emplace([this, task] () {
                std::future<Result> future = task->get_future();
                (*task)();
                std::unique_lock<std::mutex> lock(cbmtx);
                retqueue.push(future.get());
            });
            lock.unlock();
            cond.notify_one();
            return true;
        } else {
            return false;
        }
    }
    
};
