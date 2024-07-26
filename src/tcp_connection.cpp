#include "tcp_connection.h"
#include "db.h"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <nlohmann/json.hpp>

int myserver::set_nonblocking(int fd) {
    int old = fcntl(fd, F_GETFL);
    int now = old | O_NONBLOCK;
    fcntl(fd, F_SETFL, now);
    return old;
}

void myserver::add_fd(int epoll_fd, int fd, bool one_shot, bool et_mode) {
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

void myserver::remove_fd(int epoll_fd, int fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

void myserver::modify_fd(int epoll_fd, int fd, int ev, bool et_mode) {
    epoll_event event{};
    event.data.fd = fd;
    if (1 == et_mode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

myserver::TcpConnection::TcpConnection(int socket)
    : socket_(socket) {
    // add event
    add_fd(epoll_fd, socket_, true, true);
    reset();
}

void myserver::TcpConnection::process() {
    process_read(); // generate response code
    process_write(); // generate response content
}

void myserver::TcpConnection::process_internal(Arcade::OneGameIter it) {
    // handle signal from enemy thread
    game_ = it;
    using nlohmann::json;
    generate_response();
    write_ptr_ = std::strlen(write_buffer_);
}

void myserver::TcpConnection::process_read() {
    using json = nlohmann::json;
    json data = json::parse(std::string(read_buffer_));
    if (!data.contains("secret") || data["secret"] != SECRET) {
        response_state_ = WRONG_SECRET;
        return;
    }
    auto type = data["type"];
    if (type.is_null() || !isdigit(type)) {
        response_state_ = BAD_REQUEST;
        return;
    }
    switch(static_cast<REQUEST_TYPE>(type)) {
        case LOGIN:
            if (handle_login(data)) {
                handle_pairing(data);
            }
            break;
        case ACTION:
            handle_action(data);
            break;
    }
}

void myserver::TcpConnection::process_write() {
    using nlohmann::json;
    json j;
    switch(response_state_) {
        case WRONG_SECRET:
            j["code"] = WRONG_SECRET;
            j["message"] = "wrong secret";
            std::strcpy(write_buffer_, to_string(j).c_str());
            break;
        case BAD_REQUEST:
            j["code"] = BAD_REQUEST;
            j["message"] = "bad request";
            std::strcpy(write_buffer_, to_string(j).c_str());
            break;
        case LOGIN_FAILED:
            j["code"] = LOGIN_FAILED;
            j["message"] = "loging failed";
            std::strcpy(write_buffer_, to_string(j).c_str());
            break;
        case FULL:
            j["code"] = FULL;
            j["message"] = "full";
            std::strcpy(write_buffer_, to_string(j).c_str());
            break;
        case PAIRING:
            j["code"] = PAIRING;
            j["message"] = "pairing";
            std::strcpy(write_buffer_, to_string(j).c_str());
        case PAIRING_SUCCEED:
            generate_response();
            dispatch_across(); // send message to the other player
            break;
    }
    write_ptr_ = std::strlen(write_buffer_);
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

bool myserver::TcpConnection::handle_login(nlohmann::json &data) {
    auto name = data["name"];
    auto pwd = data["password"];
    if (name.is_null() || pwd.is_null()) {
        response_state_ = BAD_REQUEST;
        return false;
    }
    if (UserCache::instance().verify(name, pwd)) {
        user_uuid_ = UserCache::instance().get_uuid(name);
        return true;
    } else {
        response_state_ = LOGIN_FAILED;
        return false;
    }
}

void myserver::TcpConnection::handle_pairing(nlohmann::json &data) {
    Arcade::OneGameIter it;
    Arcade::PAIRING_STATE state = Arcade::instance().add_player(user_uuid_, it);
    switch (state) {
        case Arcade::PAIRING_SUCCEED:
            game_ = it;
            response_state_ = PAIRING_SUCCEED;
            break;
        case Arcade::PAIRING_FAILED_NOW_WAITING:
            response_state_ = PAIRING;
            break;
        case Arcade::PAIRING_FAILED_FULL:
            response_state_ = FULL;
            break;
    }
}

void myserver::TcpConnection::handle_action(nlohmann::json &data) {
    if (game_ == Arcade::OneGameIter{} || game_->current_acting != user_uuid_) {
        response_state_ = BAD_REQUEST;
        return;
    }
    auto pos = data["pos"];
    if (pos.is_null()) {
        response_state_ = BAD_REQUEST;
        return;
    }
    if (pos < 0 || pos >= 9 || game_->chess_board[pos] != '0') {
        response_state_ = BAD_REQUEST;
        return;
    }
    if (user_uuid_ == game_->player1) {
        game_->chess_board[pos] = '1';
        game_->current_acting = game_->player2;
    } else {
        game_->chess_board[pos] = '2';
        game_->current_acting = game_->player1;
    }
}

void myserver::TcpConnection::generate_response() {
    using nlohmann::json;
    json j;
    j["code"] = PAIRING_SUCCEED;
    j["current_acting"] = game_->current_acting;
    j["chessboard"] = std::string(game_->chess_board);
    std::strcpy(write_buffer_, to_string(j).c_str());
}

bool myserver::TcpConnection::write_data() {
    if (write_ptr_ == 0) {
        modify_fd(epoll_fd, socket_, EPOLLIN, true);
        reset();
        return true;
    }
    int data_to_send = write_ptr_;
    while (true) {
        int ret = send(socket_, write_buffer_, data_to_send, 0);
        if (ret < 0) {
            if (errno == EAGAIN) {
                modify_fd(epoll_fd, socket_, EPOLLOUT, true);
                return true;
            }
            return false;
        }
        data_to_send -= ret;
        if (data_to_send <= 0) {
            modify_fd(epoll_fd, socket_, EPOLLIN, true);
            reset();
            return true;
        }
    }
}

int myserver::TcpConnection::epoll_fd = 0;

void myserver::TcpConnection::reset() {
    read_ptr_ = 0;
    write_ptr_ = 0;
    memset(read_buffer_, '\0', sizeof(read_buffer_));
    memset(write_buffer_, '\0', sizeof(write_buffer_));
}

void myserver::TcpConnection::dispatch_across() {
    // inform the other player

}
