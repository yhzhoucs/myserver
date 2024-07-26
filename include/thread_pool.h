#ifndef MYSERVER_INCLUDE_THREAD_POOL_H_
#define MYSERVER_INCLUDE_THREAD_POOL_H_

#include "utils.h"
#include "tcp_connection.h"

namespace myserver {

class ThreadPool {
public:
    typedef std::pair<TcpConnection*, int> Task;
    typedef std::pair<TcpConnection*, Arcade::OneGameIter> InternalTask;
    static ThreadPool &instance() {
        static ThreadPool thread_pool;
        return thread_pool;
    }
    void init();
    void new_request(TcpConnection *conn, int type) {
        requests_.push(Task{conn, type});
    }
    void new_internal_request(TcpConnection *conn, Arcade::OneGameIter it) {
        internal_requests_.push(InternalTask{conn, it});
    }
    static constexpr int max_worker_number = 4;
    static constexpr int max_internal_worker_number = 2;
private:
    ThreadPool() = default;
    void work();
    void internal_work();
    utils::BlockingQueue<Task> requests_{};
    utils::BlockingQueue<InternalTask> internal_requests_{};
    std::vector<std::thread> workers_{};
    std::vector<std::thread> internal_workers_{};
};

}

#endif // MYSERVER_INCLUDE_THREAD_POOL_H_
