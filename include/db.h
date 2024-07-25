#ifndef MYSERVER_INCLUDE_DB_H_
#define MYSERVER_INCLUDE_DB_H_

#include <sqlite3.h>
#include <string>
#include <condition_variable>
#include <mutex>
#include <map>

namespace myserver {

namespace config {
    constexpr int max_db_connection_number = 4;
}

class DB {
public:
    static DB &instance() {
        static DB db;
        return db;
    }
    void init(std::string const &db_file);
    ~DB();
    // This function is NOT thread-safe!
    [[nodiscard]] inline
    int connection_available() const {
        return static_cast<int>(wr_ptr_ + buffer_size - rd_ptr_) % buffer_size;
    }
    // This function is NOT thread-safe!
    [[nodiscard]] inline
    bool buffer_empty() const {
        return rd_ptr_ == wr_ptr_;
    }
    // This function is NOT thread-safe!
    [[nodiscard]] inline
    bool buffer_full() const {
        return (wr_ptr_ + 1) % buffer_size == rd_ptr_;
    }
    sqlite3 *get_connection();
    void back_connection(sqlite3 *connection);
    static constexpr int max_conn_num = config::max_db_connection_number;
    static constexpr int buffer_size = max_conn_num + 1;
private:
    DB() = default;
    sqlite3 *connections_[buffer_size] = { nullptr };
    std::ptrdiff_t rd_ptr_ = 0;
    std::ptrdiff_t wr_ptr_ = 0;
    std::condition_variable idle_connections_cv_ = {};
    std::mutex mutex_ = {};
};

class ScopedDBConn {
public:
    ScopedDBConn() {
        connection_ = DB::instance().get_connection();
    }
    ~ScopedDBConn() {
        DB::instance().back_connection(connection_);
    }
    sqlite3 *get() {
        return connection_;
    }
private:
    sqlite3 *connection_;
};

class UserCache {
public:
    typedef std::map<int, std::pair<std::string, std::string>> UserMap;
    static UserCache &instance() {
        static UserCache cache;
        return cache;
    }
    static int callback(void *data, int argc, char **argv,char **az_col_name);
    void init();
private:
    UserCache() = default;
    UserMap users_{};
};

}

#endif // MYSERVER_INCLUDE_DB_H_