#ifndef MYSERVER_INCLUDE_ARCADE_H_
#define MYSERVER_INCLUDE_ARCADE_H_

#include "utils.h"
#include <mutex>
#include <list>

namespace myserver {

struct OneGame {
    int uuid;
    int player1;
    int player2;
    int current_acting; // 1 or 2
    char chess_board[9]; // 0,1,2
};

class Arcade {
public:
    typedef std::list<OneGame>::iterator OneGameIter;
    enum PAIRING_STATE {
        PAIRING_SUCCEED,
        PAIRING_FAILED_NOW_WAITING,
        PAIRING_FAILED_FULL
    };
    static Arcade &instance() {
        static Arcade arcade;
        return arcade;
    }
    PAIRING_STATE add_player(int uuid, OneGameIter &one_game_iter);
private:
    int game_uuid_{};
    utils::BlockingQueue<int> users_waiting_;
    std::list<OneGame> games_;
    std::mutex mutex_;
};

}

#endif // MYSERVER_INCLUDE_ARCADE_H_
