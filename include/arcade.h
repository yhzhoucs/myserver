#ifndef MYSERVER_INCLUDE_ARCADE_H_
#define MYSERVER_INCLUDE_ARCADE_H_

#include "utils.h"
#include <mutex>
#include <map>

namespace myserver {

class Arcade {
public:
    static Arcade &instance() {
        static Arcade arcade;
        return arcade;
    }
private:
    utils::BlockingQueue<int> users_waiting_;
    std::map<int, int> game_pair_;
    std::mutex game_pair_mutex_;
};

}

#endif // MYSERVER_INCLUDE_ARCADE_H_
