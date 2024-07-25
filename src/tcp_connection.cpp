#include "tcp_connection.h"

#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <nlohmann/json.hpp>

myserver::TcpConnection::TcpConnection(int socket)
    : socket_(socket) {
    memset(read_buffer_, '\0', sizeof(read_buffer_));
    memset(write_buffer_, '\0', sizeof(write_buffer_));
}

void myserver::TcpConnection::process() {
    using json = nlohmann::json;
    json data = json::parse(std::string(read_buffer_));
    int type = data.at("type");

}

bool myserver::TcpConnection::read_data() {
    if (read_ptr_ >= read_buffer_size) {
        return false;
    }
    // read the whole data from socket in ET mode
    while (true) {
        int ret = recv(socket_, read_buffer_ + read_ptr_, read_buffer_size - read_ptr_, 0);
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return false;
        } else if (ret == 0) {
            return false;
        }
        read_ptr_ += ret;
    }
    return true;
}
