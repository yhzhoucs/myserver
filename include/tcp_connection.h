#ifndef MYSERVER_INCLUDE_TCP_CONNECTION_H_
#define MYSERVER_INCLUDE_TCP_CONNECTION_H_

#include <vector>
#include <nlohmann/json.hpp>
#include "arcade.h"

namespace myserver {

int set_nonblocking(int fd);
void add_fd(int epoll_fd, int fd, bool one_shot, bool et_mode);
void remove_fd(int epoll_fd, int fd);
void modify_fd(int epoll_fd, int fd, int ev, bool et_mode);

class TcpConnection {
public:
    explicit TcpConnection(int socket);
    bool read_data();
    bool write_data();
    void process();
    void process_internal(Arcade::OneGameIter it);
    static constexpr int read_buffer_size = 2048;
    static constexpr int write_buffer_size = 2048;
    static int epoll_fd;
    enum REQUEST_TYPE {
        LOGIN,
        ACTION
    };
    enum RESPONSE_STATE {
        WRONG_SECRET,
        BAD_REQUEST,
        LOGIN_FAILED,
        PAIRING,
        PAIRING_SUCCEED,
        FULL
    };
private:
    void reset();
    void process_read();
    void process_write();
    void dispatch_across();
    bool handle_login(nlohmann::json &data);
    void handle_pairing(nlohmann::json &data);
    void handle_action(nlohmann::json &data);
    void generate_response();
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
