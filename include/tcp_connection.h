#ifndef MYSERVER_INCLUDE_TCP_CONNECTION_H_
#define MYSERVER_INCLUDE_TCP_CONNECTION_H_

#include <vector>
#include <nlohmann/json.hpp>
#include "arcade.h"

namespace myserver {

int set_nonblocking(int fd);
void add_fd(int epoll_fd, int fd, bool one_shot, bool et_mode);
void remove_fd(int epoll_fd, int fd);

class TcpConnection {
public:
    explicit TcpConnection(int socket);
    bool read_data();
    void process();
    static constexpr int read_buffer_size = 2048;
    static constexpr int write_buffer_size = 2048;
    enum REQUEST_TYPE {
        LOGIN,
        ACTION
    };
    enum RESPONSE_STATE {
        NOP,
        WRONG_SECRET,
        BAD_REQUEST,
        LOGIN_SUCCESS,
        LOGIN_FAILED,
        PAIRING,
        FULL,
        GAMING
    };
private:
    void process_read();
    void process_write();
    bool handle_login(nlohmann::json &data);
    void handle_pairing(nlohmann::json &data);
    void handle_action(nlohmann::json &data);
    int socket_;
    char read_buffer_[read_buffer_size]{};
    std::ptrdiff_t read_ptr_ = 0;
    char write_buffer_[write_buffer_size]{};
    std::ptrdiff_t write_ptr_ = 0;
    Arcade::OneGameIter game_;
    RESPONSE_STATE response_state_;
    int user_uuid_ = -1;
};

}

#endif // MYSERVER_INCLUDE_TCP_CONNECTION_H_
