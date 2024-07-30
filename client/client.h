#ifndef MYSERVER_CLIENT_H_
#define MYSERVER_CLIENT_H_

#include <nlohmann/json.hpp>

enum RESPONSE_STATE {
    WRONG_SECRET,
    BAD_REQUEST,
    LOGIN_FAILED,
    PAIRING,
    PAIRING_SUCCEED,
    FULL,
    GAME_OVER
};

int set_nonblocking(int fd);
void add_fd(int epoll_fd, int fd, bool one_shot, bool et_mode);
void remove_fd(int epoll_fd, int fd);
void modify_fd(int epoll_fd, int fd, int ev, bool et_mode);

bool read_socket();
bool write_socket();
void reset();
void handle_response();
void handle_gaming(nlohmann::json &j);
void handle_game_over(nlohmann::json &j);

extern char send_buffer[512];
extern char receive_buffer[512];
extern int send_size;
extern int receive_size;
extern int socket_fd;
extern int epoll_fd;
extern bool running;

#endif //MYSERVER_CLIENT_H_
