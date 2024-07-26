#ifndef MYSERVER_INCLUDE_SERVER_H_
#define MYSERVER_INCLUDE_SERVER_H_

#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string>
#include "tcp_connection.h"

namespace myserver {

class Server {
public:
    Server(std::string address, int port);
    void start_server();
    static constexpr int max_event_number = 100;
private:
    bool handle_new_connection();
    std::string address_str_;
    sockaddr_in address_;
    int port_;
    int epoll_fd_;
    epoll_event events_[max_event_number];
    int listen_fd_;
    std::map<int, TcpConnection> connections_; // sock_fd -> connection
};

}

#endif //MYSERVER_INCLUDE_SERVER_H_
