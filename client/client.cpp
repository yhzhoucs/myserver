#include <cstdlib>
#include <cstdio>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <nlohmann/json.hpp>
#include <thread>

int main(int argc, char *argv[]) {
    char server_ip[] = "127.0.0.1";
    int port = 8848;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &address.sin_addr);
    address.sin_port = htons(port);

    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(socket_fd >= 0);

    if (connect(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        printf("Connection failed.");
        close(socket_fd);
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));

    using nlohmann::json;
    json j;
    j["secret"] = "KIA";
    j["type"] = 0;
    j["name"] = "creator";
    j["password"] = "creator123";

    char send_buffer[512];
    std::memset(send_buffer, '\0', sizeof(send_buffer));
    std::strcpy(send_buffer, to_string(j).c_str());
    int send_bytes = std::strlen(send_buffer);

    int ret = send(socket_fd, send_buffer, send_bytes, 0);
    assert(ret == send_bytes);

    char rcv_buffer[512];
    std::memset(rcv_buffer, '\0', sizeof(rcv_buffer));
    ret = recv(socket_fd, rcv_buffer, 512, 0);

    close(socket_fd);

    printf("%s", rcv_buffer);

    return 0;
}