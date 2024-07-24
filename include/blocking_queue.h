#ifndef MYSERVER_INCLUDE_BLOCKING_QUEUE_H_
#define MYSERVER_INCLUDE_BLOCKING_QUEUE_H_

#include <mutex>
#include <deque>
#include <condition_variable>
#include <optional>

namespace myserver::utils {

template<typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(std::size_t capacity = 10): capacity_(capacity) {}
    template<typename U>
    bool try_push(U &&value) {
        {
            std::lock_guard lk(mutex_);
            if (data_.size() == capacity_) {
                return false;
            }
            data_.emplace_back(value);
        }
        not_empty_cv_.notify_one();
        return true;
    }
    template<typename U>
    void push(U &&value) {
        {
            std::unique_lock lk(mutex_);
            not_full_cv_.wait(lk, [this]() { return data_.size() < capacity_; });
            data_.emplace_back(value);
        }
        not_empty_cv_.notify_one();
    }
    std::optional<T> try_pop() {
        std::optional<T> tmp;
        {
            std::lock_guard lk(mutex_);
            if (data_.empty()) {
                return std::nullopt;
            }
            tmp = std::make_optional<T>(data_.front());
            data_.pop_front();
        }
        not_full_cv_.notify_one();
        return tmp;
    }
    T pop() {
        T tmp{};
        {
            std::unique_lock lk(mutex_);
            not_empty_cv_.wait(lk, [this]() { return !data_.empty(); });
            tmp = data_.front();
            data_.pop_front();
        }
        not_full_cv_.notify_one();
        return std::move(tmp);
    }
    bool empty() const {
        std::lock_guard lk(mutex_);
        return data_.empty();
    }
    bool full() const {
        std::lock_guard lk(mutex_);
        return data_.size() == capacity_;
    }
private:
    std::deque<T> data_;
    std::size_t capacity_;
    mutable std::mutex mutex_;
    std::condition_variable not_empty_cv_;
    std::condition_variable not_full_cv_;
};

}

#endif