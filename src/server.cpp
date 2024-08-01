#include "server.h"
#include "log.h"
#include "thread_pool.h"
#include <utility>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <signal.h>

void myserver::Server::ExpireCallback::operator()() {
    if (!server->connections_.contains(sock_fd)) {
        return;
    }
    log_info("Close socket %d", sock_fd);
    epoll_ctl(server->epoll_fd_, EPOLL_CTL_DEL, sock_fd, server->events_);
    close(sock_fd);
    if (!TcpConnection::socket_user_map.contains(sock_fd)) {
        // not even login
        return;
    }
    int user_id = TcpConnection::socket_user_map[sock_fd];
    TcpConnection::socket_user_map.erase(sock_fd);
    TcpConnection::user_socket_map.erase(user_id);
    server->connections_.erase(sock_fd);
    // erase concerned game and connections
    int rival_user_id;
    Arcade::instance().erase_player(user_id, rival_user_id);
    if (rival_user_id != -1) {
        int rival_socket_fd = TcpConnection::user_socket_map[rival_user_id];
        server->connections_.at(rival_socket_fd).inform_user_logout(); // force send logout message
        log_info("Close socket %d", rival_socket_fd);
        epoll_ctl(server->epoll_fd_, EPOLL_CTL_DEL, rival_socket_fd, server->events_);
        close(rival_socket_fd);
        TcpConnection::socket_user_map.erase(rival_socket_fd);
        TcpConnection::user_socket_map.erase(rival_user_id);
        server->connections_.erase(rival_socket_fd);
        server->timer_manager_.delete_timer(rival_socket_fd);
    }
}

myserver::Server::Server(std::string address, int port)
    : address_str_(std::move(address)), port_(port), timer_manager_(time_slot) {
    bzero(&address_, sizeof(address_));
    address_.sin_family = AF_INET;
    inet_pton(AF_INET, address_str_.c_str(), &address_.sin_addr);
    address_.sin_port = htons(port_);
    socketpair(AF_UNIX, SOCK_STREAM, 0, sig_pipe);
    set_nonblocking(sig_pipe[1]);
}

int myserver::Server::sig_pipe[2];

void myserver::Server::sig_handler(int sig) {
    int save_errno = errno;
    int msg = sig;
    send(sig_pipe[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void myserver::Server::register_sig(int sig, void (*hdr)(int), bool restart) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = hdr;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

bool myserver::Server::handle_new_signal() {
    char msg[1024];
    int ret = recv(sig_pipe[0], msg, sizeof(msg), 0);
    if (ret <= 0) {
        return false;
    }
    for (int i = 0; i < ret; ++i) {
        switch (msg[i]) {
            case SIGALRM:
                timeout_ = true;
                break;
            case SIGTERM:
                run_server_ = false;
                break;
        }
    }
    return true;
}

void myserver::Server::start_server() {
    listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);
    assert(listen_fd_ >= 0);
    int ret{};
    int flag = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    log_info("Starting server at %s:%d...", address_str_.c_str(), port_);
    ret = bind(listen_fd_, (sockaddr *)&address_, sizeof(address_));
    assert(ret >= 0);
    ret = listen(listen_fd_, 5);
    assert(ret >= 0);

    epoll_fd_ = epoll_create(5);
    assert(epoll_fd_ != -1);

    add_fd(epoll_fd_, listen_fd_, false, true);
    add_fd(epoll_fd_, sig_pipe[0], false, false);
    register_sig(SIGPIPE, SIG_IGN, false);
    register_sig(SIGALRM, sig_handler, false);
    register_sig(SIGTERM, sig_handler, false);
    alarm(time_slot);

    TcpConnection::epoll_fd = epoll_fd_;
    run_server_ = true;

    while (run_server_) {
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
            } else if (sock_fd == sig_pipe[0]) {
                if (!handle_new_signal()) {
                    log_error("Handle signal failed");
                }
            } else if (events_[i].events & EPOLLIN) {
                if (connections_.at(sock_fd).read_data()) {
                    timer_manager_.adjust_timer(sock_fd);
                    ThreadPool::instance().new_request(&connections_.at(sock_fd), 0);
                } else {
                    timer_manager_.delete_timer(sock_fd);
                }
            } else if (events_[i].events & EPOLLOUT) {
                if (connections_.at(sock_fd).write_data()) {
                    timer_manager_.adjust_timer(sock_fd);
                } else {
                    timer_manager_.delete_timer(sock_fd);
                }
            }
        }
        if (timeout_) {
            timer_manager_.tick();
            timeout_ = false;
            alarm(time_slot); // reset alarm
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
    char tmp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, tmp, INET_ADDRSTRLEN);
    log_info("Receive connection from %s:%d.", tmp, ntohs(client_address.sin_port));
    connections_.emplace(conn_fd, TcpConnection(conn_fd, connections_));
    timer_manager_.add_timer(conn_fd, { conn_fd, this });
    return true;
}
