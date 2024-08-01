#include "thread_pool.h"
#include "log.h"

void myserver::ThreadPool::work() {
    // handle with client requests
    while (true) {
        Task task = requests_.pop();
        auto &[conn, type] = task;
        conn->process();
//        if (type == 0) {
//            // read
//            if (conn->read_data()) {
//                conn->process();
//            }
//        }
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
    for (int i{}; i < max_worker_number; ++i) {
        workers_.emplace_back(&ThreadPool::work, this);
        workers_.back().detach();
    }
    for (int i{}; i <max_internal_worker_number; ++i) {
        workers_.emplace_back(&ThreadPool::internal_work, this);
        workers_.back().detach();
    }
    log_info("Thread pool initialized successfully.");
}
