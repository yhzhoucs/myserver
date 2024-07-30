#include "client.h"
#include <cstdio>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <thread>
#include <iostream>

bool running = true;

int main(int argc, char *argv[]) {
    char server_ip[] = "127.0.0.1";
    int port = 8848;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &address.sin_addr);
    address.sin_port = htons(port);

    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(socket_fd >= 0);

    if (connect(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        printf("Connection failed.\n");
        close(socket_fd);
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    epoll_fd = epoll_create(5);
    add_fd(epoll_fd, socket_fd, true, true);
    epoll_event events[100];

    using nlohmann::json;
    json j;
    j["secret"] = "KIA";
    j["type"] = 0;

    char username[32];
    char password[32];
    std::memset(username, '\0', sizeof(username));
    std::memset(password, '\0', sizeof(password));
    std::printf("username: ");
    std::scanf("%31s", username);
    std::printf("password: ");
    std::scanf("%31s", password);

    j["name"] = std::string(username);
    j["password"] = std::string(password);

    std::strcpy(send_buffer, to_string(j).c_str());
    send_size = std::strlen(send_buffer);
    modify_fd(epoll_fd, socket_fd, EPOLLOUT, true);

    while (running) {
        int number = epoll_wait(epoll_fd, events, 100, -1);
        for (int i = 0; i < number; ++i) {
            if (events[i].events & EPOLLIN) {
                read_socket();
                handle_response();
            } else if (events[i].events & EPOLLOUT) {
                write_socket();
            }
        }
    }

    close(socket_fd);
    return 0;
}