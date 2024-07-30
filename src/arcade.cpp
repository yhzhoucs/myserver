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