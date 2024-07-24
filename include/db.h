#ifndef MYSERVER_INCLUDE_DB_H_
#define MYSERVER_INCLUDE_DB_H_

#include <sqlite3.h>
#include <string>

namespace myserver {

namespace config {
    constexpr int max_db_connection_number = 4;
}

class DB {
public:
    explicit DB(std::string const &db_file);
    ~DB();
    inline constexpr
    int free_conn_num() const { return (wr_ptr_ + max_conn_num - rd_ptr_) % max_conn_num; }
    static constexpr int max_conn_num = config::max_db_connection_number;
private:
    sqlite3 *connections_[max_conn_num] = { nullptr };
    std::ptrdiff_t rd_ptr_ = 0;
    std::ptrdiff_t wr_ptr_ = 0;
};

}

#endif // MYSERVER_INCLUDE_DB_H_