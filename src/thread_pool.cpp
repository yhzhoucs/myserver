#include "thread_pool.h"

void myserver::ThreadPool::work() {
    // handle with client requests
    while (true) {
        Task task = requests_.pop();
        auto &[conn, type] = task;
        if (type == 0) {
            // read
            if (conn->read_data()) {
                conn->process();
            }
        }
    }
}

void myserver::ThreadPool::internal_work() {
    while (true) {
        InternalTask task = internal_requests_.pop();
        auto &[conn, one_game_it] = task;
        conn->process_internal(one_game_it);
    }
}

void myserver::ThreadPool::init() {
    for (auto &t : workers_) {
        t = std::thread(&ThreadPool::work, this);
        t.detach();
    }
    for (auto &t : internal_workers_) {
        t = std::thread(&ThreadPool::internal_work, this);
        t.detach();
    }
}
