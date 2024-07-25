#ifndef MYSERVER_INCLUDE_LOG_H_
#define MYSERVER_INCLUDE_LOG_H_

#include <unistd.h>
#include <thread>
#include <string>
#include "utils.h"

namespace myserver {

class Log {
public:
    static Log &instance() {
        static Log log;
        return log;
    }
    enum Level {
        DEBUG,
        INFO,
        WARN,
        ERROR
    };
    ~Log();
    void init(std::string const &log_store_path,
              int max_log_line, int max_buffer_size, bool async = false);
    [[nodiscard]] std::string const &log_file_path() const { return log_file_path_; }
    void write_log(Level level, char const *format, ...);
    void flush_log() { fsync(log_file_); }
    void terminate() {
        terminate_ = true;
        if (async_ && logs_.empty())
            logs_.push(""); // release logging thread
    }
private:
    Log() = default;
    [[noreturn]] void async_write_log();
    bool new_log_file();
    int max_log_line_ = 0;
    int max_buffer_size_ = 0;
    char *log_buffer_ = nullptr;
    std::string log_store_path_{};
    int log_file_ = 0;
    std::string log_file_path_{};
    int log_line_ = 0;
    bool async_ = false;
    utils::BlockingQueue<std::string> logs_{};
    std::mutex mutex_{};
    std::thread log_thread_{};
    bool terminate_ = false;
};

template<typename... Args>
constexpr void log_debug(Args&&... args) {
    Log::instance().write_log(Log::DEBUG, std::forward<Args>(args)...);
    Log::instance().flush_log();
}

template<typename... Args>
constexpr void log_info(Args&&... args) {
    Log::instance().write_log(Log::INFO, std::forward<Args>(args)...);
    Log::instance().flush_log();
}

template<typename... Args>
constexpr void log_warn(Args&&... args) {
    Log::instance().write_log(Log::WARN, std::forward<Args>(args)...);
    Log::instance().flush_log();
}

template<typename... Args>
constexpr void log_error(Args&&... args) {
    Log::instance().write_log(Log::ERROR, std::forward<Args>(args)...);
    Log::instance().flush_log();
}

}

#endif // MYSERVER_INCLUDE_LOG_H_