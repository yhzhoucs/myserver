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
