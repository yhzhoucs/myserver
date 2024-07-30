#include "client.h"
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>

int set_nonblocking(int fd) {
    int old = fcntl(fd, F_GETFL);
    int now = old | O_NONBLOCK;
    fcntl(fd, F_SETFL, now);
    return old;
}

void add_fd(int epoll_fd, int fd, bool one_shot, bool et_mode) {
    epoll_event event{};
    event.data.fd = fd;
    if (1 == et_mode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;
    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    set_nonblocking(fd);
}

void remove_fd(int epoll_fd, int fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

void modify_fd(int epoll_fd, int fd, int ev, bool et_mode) {
    epoll_event event{};
    event.data.fd = fd;
    if (1 == et_mode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

void handle_response() {
    nlohmann::json j = nlohmann::json::parse(std::string(receive_buffer));
    receive_size = 0;
    RESPONSE_STATE response = j["code"];
    switch (response) {
        case WRONG_SECRET:
            printf("wrong secret error\n");
            std::exit(-1);
        case BAD_REQUEST:
            printf("bad request error\n");
            std::exit(-1);
        case LOGIN_FAILED:
            printf("login failed error\n");
            std::exit(-1);
        case FULL:
            printf("server busy\n");
            std::exit(-1);
        case PAIRING:
            printf("pairing\n");
            // reset epoll event for continuously receive pairing result from server
            modify_fd(epoll_fd, socket_fd, EPOLLIN, true);
            break;
        case PAIRING_SUCCEED:
            handle_gaming(j);
            break;
        case GAME_OVER:
            handle_game_over(j);
            break;
    }
}

void print_chessboard(char const *chessboard) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            char c;
            switch (chessboard[i * 3 + j]) {
                case '0':
                    c = '`';
                    break;
                case '1':
                    c = 'O';
                    break;
                case '2':
                    c = 'X';
                    break;
            }
            std::printf("%3c", c);
        }
        std::putchar('\n');
    }
}

void handle_gaming(nlohmann::json &j) {
    std::string chessboard = j["chessboard"];
    print_chessboard(chessboard.c_str());
    bool current_acting = j["current_acting"];
    if (!current_acting) {
        printf("waiting for the other player.\n");
        modify_fd(epoll_fd, socket_fd, EPOLLIN, true);
        return;
    }
    nlohmann::json request;
    request["secret"] = "KIA";
    request["type"] = 1; // action
    int x, y;
    std::printf("enter your pos (format coordinate `x,y` start from 1): ");
    std::scanf("%d,%d", &x, &y);
    request["pos"] = (x-1) * 3 + y - 1;
    std::strcpy(send_buffer, to_string(request).c_str());
    send_size = std::strlen(send_buffer);
    // register send event
    modify_fd(epoll_fd, socket_fd, EPOLLOUT, true);
}

void handle_game_over(nlohmann::json &j) {
    std::string chessboard = j["chessboard"];
    print_chessboard(chessboard.c_str());
    std::printf("game over\n");
    bool tie = j["tie"];
    if (tie) {
        std::printf("tie\n");
    } else {
        bool is_winner = j["is_winner"];
        if (is_winner) {
            std::printf("you win!\n");
        } else {
            std::printf("you lose!\n");
        }
    }
    running = false;
    close(socket_fd);
}

bool read_socket() {
    std::memset(receive_buffer, '\0', sizeof(receive_buffer));
    receive_size = 0;
    while (true) {
        int ret = recv(socket_fd, receive_buffer + receive_size, sizeof(receive_buffer) - receive_size, 0);
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            return false;
        } else if (ret == 0) {
            return false;
        }
        receive_size += ret;
    }
//    printf("%s\n", receive_buffer);
    return true;
}

bool write_socket() {
    if (send_size == 0) {
        modify_fd(epoll_fd, socket_fd, EPOLLIN, true);
        reset();
        return true;
    }
    int data_to_send = send_size;
    while (true) {
        int ret = send(socket_fd, send_buffer, data_to_send, 0);
        if (ret < 0) {
            if (errno == EAGAIN) {
                modify_fd(epoll_fd, socket_fd, EPOLLOUT, true);
                return true;
            }
            return false;
        }
        data_to_send -= ret;
        if (data_to_send <= 0) {
            modify_fd(epoll_fd, socket_fd, EPOLLIN, true);
            reset();
            return true;
        }
    }
}

void reset() {
    std::memset(send_buffer, '\0', sizeof(send_buffer));
    std::memset(receive_buffer, '\0', sizeof(receive_buffer));
    send_size = 0;
    receive_size = 0;
}

char send_buffer[512];
char receive_buffer[512];
int send_size;
int receive_size;
int socket_fd;
int epoll_fd;