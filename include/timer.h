#ifndef MYSERVER_INCLUDE_TIMER_H_
#define MYSERVER_INCLUDE_TIMER_H_

#include <functional>
#include <unistd.h>
#include <vector>
#include <chrono>

namespace myserver {

template<typename ExpireCallBack>
struct Timer {
    int id;
    int64_t expire_time;
    ExpireCallBack expire_callback;
};

template<typename ExpireCallBack>
class TimerManager {
public:
    typedef std::vector<Timer<ExpireCallBack>>::iterator TimerIterator;
    typedef std::vector<Timer<ExpireCallBack>>::pointer TimerPointer;

    struct Comparer {
        bool operator()(Timer<ExpireCallBack> const &t1, Timer<ExpireCallBack> const &t2) {
            return t1.expire_time > t2.expire_time;
        }
    };

    static constexpr Comparer comparer = {};

    TimerManager() : TimerManager(5) {}

    explicit TimerManager(int time_slot) : time_slot_(time_slot), default_duration_(3 * time_slot) {}

    void add_timer(int id, ExpireCallBack callback) {
        auto expire = std::chrono::system_clock::now() + std::chrono::seconds(default_duration_);
        min_heap_.emplace_back(id, std::chrono::duration_cast<std::chrono::seconds>(
                expire.time_since_epoch()).count(), callback);
        std::push_heap(min_heap_.begin(), min_heap_.end(), comparer);
    }

    void adjust_timer(int id) {
        TimerIterator it;
        for (it = min_heap_.begin(); it != min_heap_.end(); ++it) {
            if (it->id == id) {
                break;
            }
        }
        if (it == min_heap_.end()) return;
        std::pop_heap(it, min_heap_.end(), comparer); // swap target timer with the last timer
        auto expire = std::chrono::system_clock::now() + std::chrono::seconds(default_duration_);
        min_heap_.back().expire_time = std::chrono::duration_cast<std::chrono::seconds>(
                expire.time_since_epoch()).count();
        std::push_heap(min_heap_.begin(), min_heap_.end(), comparer); // shift up
    }

    void delete_timer(int id) {
        TimerIterator it;
        for (it = min_heap_.begin(); it != min_heap_.end(); ++it) {
            if (it->id == id) {
                break;
            }
        }
        if (it == min_heap_.end()) return;
        it->expire_callback();
        std::pop_heap(it, min_heap_.end(), comparer);
        min_heap_.pop_back();
    }

    void tick() {
        int64_t curr = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        while (!min_heap_.empty() && curr > min_heap_.front().expire_time) {
            min_heap_.front().expire_callback();
            std::pop_heap(min_heap_.begin(), min_heap_.end(), comparer);
            min_heap_.pop_back();
        }
    }

private:
    std::vector<Timer<ExpireCallBack>> min_heap_;
    int time_slot_;
    int default_duration_;
};

}

#endif // MYSERVER_INCLUDE_TIMER_H_
