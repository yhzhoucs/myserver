#include "tcp_connection.h"
#include "db.h"

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
        response_state_ = LOGIN_SUCCESS;
        user_uuid_ = UserCache::instance().get_uuid(name);
        return true;
    } else {
        response_state_ = LOGIN_FAILED;
        return false;
    }
}

void myserver::TcpConnection::handle_pairing(nlohmann::json &data) {
    Arcade::OneGameIter one_game;
    Arcade::PAIRING_STATE state = Arcade::instance().add_player(user_uuid_, one_game);
    switch (state) {
        case Arcade::PAIRING_SUCCEED:
            game_ = one_game;
            response_state_ = GAMING;
            // TODO inform player 1
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
    game_->chess_board[pos] = user_uuid_ == game_->player1 ? '1' : '2';
    game_->current_acting = user_uuid_ == game_->player1 ? game_->player2 : game_->player1;
    response_state_ = GAMING;
    // TODO inform another player to acting
}
