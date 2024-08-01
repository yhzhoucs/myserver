#ifndef MYSERVER_INCLUDE_SERVER_H_
#define MYSERVER_INCLUDE_SERVER_H_

#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string>
#include "tcp_connection.h"
#include "timer.h"

namespace myserver {

class Server {
public:
    Server(std::string address, int port);
    void start_server();
    [[nodiscard]] std::map<int, TcpConnection> const &connections() const { return connections_; };
    static constexpr int max_event_number = 100;
    static constexpr int time_slot = 10;
private:
    struct ExpireCallback {
        int sock_fd;
        Server *server;
        void operator()();
    };
    static int sig_pipe[2];
    static void sig_handler(int sig);
    static void register_sig(int sig, void (*hdr)(int), bool restart);
    bool handle_new_connection();
    bool handle_new_signal();
    std::string address_str_;
    sockaddr_in address_;
    int port_;
    int epoll_fd_;
    epoll_event events_[max_event_number];
    int listen_fd_;
    std::map<int, TcpConnection> connections_; // sock_fd -> connection
    TimerManager<ExpireCallback> timer_manager_;

    bool timeout_ = false;
    bool run_server_ = false;
};

}

#endif //MYSERVER_INCLUDE_SERVER_H_
