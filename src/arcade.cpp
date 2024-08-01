#include "arcade.h"
#include <cstring>

myserver::Arcade::PAIRING_STATE myserver::Arcade::add_player(int uuid, OneGameIter &one_game_iter) {
    auto player1 = users_waiting_.try_pop();
    if (player1.has_value()) {
        std::lock_guard lk(mutex_);
        OneGame tmp{
                game_uuid_++,
                player1.value(),
                uuid,
                1
        };
        std::memset(tmp.chess_board, '0', sizeof(tmp.chess_board));
        games_.emplace_front(tmp);
        one_game_iter = games_.begin();
        return PAIRING_SUCCEED;
    } else {
        if (users_waiting_.try_push(uuid)) {
            return PAIRING_FAILED_NOW_WAITING;
        } else {
            return PAIRING_FAILED_FULL;
        }
    }
}

bool myserver::Arcade::judge(char const *chessboard, int &winner) {
    // each row
    for (int i = 0; i < 3; ++i) {
        char head = chessboard[i * 3];
        bool flag = true;
        if (head != '0') {
            for (int j = 1; j < 3; ++j) {
                if (chessboard[i * 3 + j] != head) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                winner = head - '0';
                return true;
            }
        }
    }

    // each column
    for (int i = 0; i < 3; ++i) {
        char head = chessboard[i];
        bool flag = true;
        if (head != '0') {
            for (int j = 1; j < 3; ++j) {
                if (chessboard[j * 3 + i] != head) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                winner = head - '0';
                return true;
            }
        }
    }

    // diagonal
    char head = chessboard[0];
    if (head != '0') {
        bool flag = true;
        for (int i = 1; i < 3; ++i) {
            if (head != chessboard[i * 3 + i]) {
                flag = false;
                break;
            }
        }
        if (flag) {
            winner = head - '0';
            return true;
        }
    }
    head = chessboard[2];
    if (head != '0') {
        bool flag = true;
        for (int i = 1; i < 3; ++i) {
            if (head != chessboard[i * 3 + 2 - i]) {
                flag = false;
                break;
            }
        }
        if (flag) {
            winner = head - '0';
            return true;
        }
    }

    for (int i = 0; i < 9; ++i) {
        if (chessboard[i] == '0')
            return false;
    }

    winner = 0;
    return true;
}

void myserver::Arcade::erase_player(int user_id, int &rival_user_id) {
    // delete possible waiting user
    if (users_waiting_.erase(user_id)) {
        rival_user_id = -1; // no other user
        return;
    }
    // delete games concerning user
    std::list<OneGame>::iterator it;
    for (it = games_.begin(); it != games_.end(); ++it) {
        if (it->player1 == user_id || it->player2 == user_id) {
            break;
        }
    }
    if (it == games_.end()) return;
    int other_player = user_id == it->player1 ? it->player2 : it->player1;
    games_.erase(it);
    rival_user_id = other_player;
}