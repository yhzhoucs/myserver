#include "db.h"
#include "log.h"
#include <cstdlib>
#include <cstring>

void myserver::DB::init(std::string const &db_file) {
    for (int i = 0; i < max_conn_num; ++i) {
        if (sqlite3_open(db_file.c_str(), &connections_[wr_ptr_]) == SQLITE_OK) {
            log_info("create db connection %d successfully", i);
            wr_ptr_ = (wr_ptr_ + 1) % buffer_size;
        } else {
            log_error("connect to database error: %s", sqlite3_errmsg(connections_[wr_ptr_]));
            exit(1);
        }
    }
}

myserver::DB::~DB() {
    for ( ; rd_ptr_ != wr_ptr_; rd_ptr_ = (rd_ptr_ + 1) % buffer_size) {
        if (sqlite3_close(connections_[rd_ptr_]) != SQLITE_OK) {
            printf("error when closing db");
            exit(1);
        }
    }
}

sqlite3 *myserver::DB::get_connection() {
    std::unique_lock lk(mutex_);
    if (connection_available() == 0) {
        // no connections available, block the thread
        idle_connections_cv_.wait(lk);
    }
    sqlite3 *tmp = connections_[rd_ptr_];
    rd_ptr_ = (rd_ptr_ + 1) % buffer_size;
    return tmp;
}

void myserver::DB::back_connection(sqlite3 *connection) {
    std::lock_guard lk(mutex_);
    connections_[wr_ptr_] = connection;
    wr_ptr_ = (wr_ptr_ + 1) % buffer_size;
    // activate one blocked thread if any
    idle_connections_cv_.notify_one();
}

int myserver::UserCache::callback(void *data, int argc, char **argv, char **az_col_name) {
    auto *map = reinterpret_cast<UserMap*>(data);
    if (argc != 3 || std::strcmp(az_col_name[0], "uuid") != 0
        || std::strcmp(az_col_name[1],"name") != 0 || std::strcmp(az_col_name[2], "password") != 0) {
        log_error("user cache fetching user data failed: col not match.");
        exit(1);
    }
    int uuid = std::stoi(argv[0]);
    std::string name = argv[1];
    std::string pwd = argv[2];
    map->emplace(name, std::make_pair(uuid, pwd));
    return 0;
}

void myserver::UserCache::init() {
    ScopedDBConn s_conn;
    sqlite3 *conn = s_conn.get();
    char const *sql = "SELECT uuid,name,password FROM user";
    char *z_err_msg;
    if (sqlite3_exec(conn, sql, callback, (void *)&users_, &z_err_msg) != SQLITE_OK) {
        log_error("user cache fetching user data failed: %s", z_err_msg);
        sqlite3_free(z_err_msg);
    }
    log_info("UserCache initialized successfully.");
    for (auto &item : users_) {
        log_debug("Fetch user info (%s, %d, %s).", item.first.c_str(), item.second.first, item.second.second.c_str());
    }
}

bool myserver::UserCache::verify(std::string const &name, std::string const &password) {
    return users_.contains(name) && users_.at(name).second == password;
}

int myserver::UserCache::get_uuid(const std::string &name) {
    return users_.at(name).first;
}
