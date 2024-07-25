#include "log.h"
#include <fcntl.h>
#include <uuid/uuid.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <algorithm>

#include <filesystem>
#include <format>

myserver::Log::~Log() {
    close(log_file_);
    delete[] log_buffer_;
}

void myserver::Log::init(std::string const &log_store_path,
                                int max_log_line, int max_buffer_size, bool async) {
    // create log store path
    log_store_path_ = log_store_path;
    std::filesystem::path path(log_store_path_);
    if (!exists(path)) {
        create_directory(path);
    }

    // create log file
    if (!new_log_file()) {
        std::printf("Fatal Error: can not create log file.");
        std::exit(1);
    }

    max_log_line_ = max_log_line;
    max_buffer_size_ = max_buffer_size;
    log_buffer_ = new char[max_buffer_size_];
    async_ = async;
    if (async_) {
        // start a log thread
        log_thread_ = std::thread(&Log::async_write_log, std::ref(*this));
        log_thread_.detach();
    }
}

void myserver::Log::async_write_log() {
    for (auto single_log = logs_.pop(); !terminate_; single_log = logs_.pop()) {
        std::lock_guard lk(mutex_);
        write(log_file_, single_log.c_str(), single_log.length());
    }
}

void myserver::Log::write_log(Level level, const char *format, ...) {
    {
        // write a log
        std::lock_guard lk(mutex_);
        ++log_line_;

        // If current log file reaches its max bound, create another file.
        if (log_line_ > max_log_line_) {
            close(log_file_);
            new_log_file();
            log_line_ = 0;
        }
    }
    char level_str[16];
    switch (level) {
        case DEBUG:
            std::snprintf(level_str, 16, "DEBUG");
            break;
        case INFO:
            std::snprintf(level_str, 16, "INFO");
            break;
        case WARN:
            std::snprintf(level_str, 16, "WARN");
            break;
        case ERROR:
            std::snprintf(level_str, 16, "ERROR");
            break;
        default:
            std::snprintf(level_str, 16, "INFO");
    }

    std::time_t t = std::time(nullptr); // get time now
    std::tm* now = std::localtime(&t);
    std::va_list vl;
    va_start(vl, format);

    std::string tmp;
    {
        std::lock_guard lk(mutex_);
        int n = snprintf(log_buffer_, 64, "[%d-%02d-%02d %02d:%02d:%02d] [%s] ",
                         now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
                         now->tm_hour, now->tm_min, now->tm_sec, level_str);
        int m = vsnprintf(log_buffer_ + n, max_buffer_size_ - n - 1, format, vl);
        log_buffer_[n + m] = '\n';
        log_buffer_[n + m + 1] = '\0';
        tmp = log_buffer_;
    }

    if (async_ && !logs_.full()) {
        logs_.push(tmp);
    } else {
        std::lock_guard lk(mutex_);
        write(log_file_, tmp.c_str(), tmp.length());
    }
}

bool myserver::Log::new_log_file() {
    std::time_t t = std::time(nullptr); // get time now
    std::tm* now = std::localtime(&t);
    uuid_t uuid; // generate uuid
    uuid_generate(uuid);
    char uuid_str[UUID_STR_LEN];
    uuid_unparse(uuid, uuid_str);
    std::string log_file_name = std::format(
            "[{}-{:02d}-{:02d}_{:02d}:{:02d}:{:02d}]_{}.log",
            now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
            now->tm_hour, now->tm_min, now->tm_sec, std::string(uuid_str).substr(0, 6));
    std::filesystem::path path(log_store_path_);
    path /= log_file_name;
    log_file_path_ = path.string();
    log_file_ = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log_file_ < 0) {
        return false;
    }
    return true;
}
