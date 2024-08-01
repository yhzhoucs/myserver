#include "client.h"
#include <unistd.h>
#include <cstdio>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <termios.h>

char *get_password(char const *prompt) {
    static char buf[MAX_PASS_LEN + 1];
    char *ptr;
    sigset_t sig, osig;
    struct termios ts, ots;
    FILE *fp;
    int c;
    if ((fp = fopen(ctermid(NULL), "r+")) == NULL)
        return NULL;
    setbuf(fp, NULL);

    sigemptyset(&sig);
    sigaddset(&sig, SIGINT);
    sigaddset(&sig, SIGTSTP);
    sigprocmask(SIG_BLOCK, &sig, &osig);

    tcgetattr(fileno(fp), &ts);
    ots = ts;
    ts.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
    tcsetattr(fileno(fp), TCSAFLUSH, &ts);
    fputs(prompt, fp);

    ptr = buf;
    while ((c = getc(fp)) != EOF && c != '\n')
        if (ptr < &buf[MAX_PASS_LEN])
            *ptr++ = c;
    *ptr = 0;
    putc('\n', fp);

    tcsetattr(fileno(fp), TCSAFLUSH, &ots);
    sigprocmask(SIG_SETMASK, &osig, NULL);
    fclose(fp);
    return buf;
}

void clear_screen() {
    printf("\033[H");
    printf("\033[J");
}

void print_title(char const *username) {
    printf("\033[35;40m");
    printf("############################################################\n");
    printf("############################################################\n");
    printf("###################### W e l c o m e #######################\n");
    printf("### Dedicated to ZH. \n");
    printf("### Enter your pos in coordinate format `x,y` start from 1.\n");
    printf("### For example, \n");
    printf("### > 1,1 will put your mark on the left-top of the chessboard.\n");
    if (username != NULL) {
        printf("### User: %s\n", username);
    }
    printf("### \n\n");
    printf("\033[0;0m");
}

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

bool handle_response() {
    nlohmann::json j = nlohmann::json::parse(std::string(receive_buffer));
    receive_size = 0;
    RESPONSE_STATE response = j["code"];
    clear_screen();
    print_title(username);
    switch (response) {
        case WRONG_SECRET:
            printf("wrong secret error\n");
            return false;
        case BAD_REQUEST:
            printf("bad request error\n");
            return false;
        case LOGIN_FAILED:
            printf("login failed error\n");
            return false;
        case FULL:
            printf("server busy\n");
            return false;
        case PAIRING:
            printf("pairing\n");
            // reset epoll event for continuously receive pairing result from server
            modify_fd(epoll_fd, socket_fd, EPOLLIN, true);
            return true;
        case PAIRING_SUCCEED:
            handle_gaming(j);
            return true;
        case GAME_OVER:
            handle_game_over(j);
            return false;
        case RIVAL_LOGOUT:
            printf("rival logout\n");
            return false;
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
    std::putchar('\n');
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
    std::printf("> ");
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
    std::putchar('\n');
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