#include "server.h"
#include "log.h"
#include "thread_pool.h"
#include <utility>
#include <cstring>
#include <cassert>

myserver::Server::Server(std::string address, int port)
    : address_str_(std::move(address)), port_(port) {
    bzero(&address_, sizeof(address_));
    address_.sin_family = AF_INET;
    inet_pton(AF_INET, address_str_.c_str(), &address_.sin_addr);
    address_.sin_port = htons(port_);
}

void myserver::Server::start_server() {
    listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);
    assert(listen_fd_ >= 0);
    int ret{};
    int flag = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(listen_fd_, (sockaddr *)&address_, sizeof(address_));
    assert(ret >= 0);
    ret = listen(listen_fd_, 5);
    assert(ret >= 0);

    epoll_fd_ = epoll_create(5);
    assert(epoll_fd_ != -1);

    add_fd(epoll_fd_, listen_fd_, false, true);
    TcpConnection::epoll_fd = epoll_fd_;

    bool stop_server = false;
    while (!stop_server) {
        int number = epoll_wait(epoll_fd_, events_, max_event_number, -1);
        if (number < 0 && errno != EINTR) {
            log_error("%s", "epoll failure");
            break;
        }
        for (int i = 0; i < number; ++i) {
            int sock_fd = events_[i].data.fd;
            if (sock_fd == listen_fd_) {
                // build new connection
                handle_new_connection();
            } else if (events_[i].events & EPOLLIN) {
                ThreadPool::instance().new_request(&connections_.at(sock_fd), 0);
            } else if (events_[i].events & EPOLLOUT) {
                connections_.at(sock_fd).write_data();
            }
        }
    }
}

bool myserver::Server::handle_new_connection() {
    sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    int conn_fd = accept(listen_fd_, (sockaddr *)&client_address, &client_addrlength);
    if (conn_fd < 0) {
        log_error("%s:errno is:%d", "accept error", errno);
        return false;
    }
    connections_.emplace(conn_fd, TcpConnection(conn_fd));
}
