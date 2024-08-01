#ifndef MYSERVER_CLIENT_H_
#define MYSERVER_CLIENT_H_

#include <nlohmann/json.hpp>

#define MAX_PASS_LEN 16

enum RESPONSE_STATE {
    WRONG_SECRET,
    BAD_REQUEST,
    LOGIN_FAILED,
    PAIRING,
    PAIRING_SUCCEED,
    FULL,
    GAME_OVER,
    RIVAL_LOGOUT
};

// console concerned
char *get_password(char const *prompt);
void clear_screen();
void print_title(char const *username);

// control concerned
int set_nonblocking(int fd);
void add_fd(int epoll_fd, int fd, bool one_shot, bool et_mode);
void remove_fd(int epoll_fd, int fd);
void modify_fd(int epoll_fd, int fd, int ev, bool et_mode);

// socket concerned
bool read_socket();
bool write_socket();
void reset();
bool handle_response();
void handle_gaming(nlohmann::json &j);
void handle_game_over(nlohmann::json &j);

// global variables
extern char send_buffer[512];
extern char receive_buffer[512];
extern int send_size;
extern int receive_size;
extern int socket_fd;
extern int epoll_fd;
extern bool running;
extern char username[32];

#endif //MYSERVER_CLIENT_H_
