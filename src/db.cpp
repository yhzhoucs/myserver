#include "db.h"
#include "log.h"
#include <cstdlib>
#include <cstdio>

myserver::DB::DB(std::string const &db_file) {
    using namespace myserver::utils;

    for (int i = 0; i < max_conn_num; ++i) {
        if (sqlite3_open(db_file.c_str(), &connections_[wr_ptr_++]) != SQLITE_OK) {
            log_error("connect to database error");
            exit(1);
        } else {
            printf("create db connection successfully\n");
        }
    }
}

myserver::DB::~DB() {
    using namespace myserver::utils;

    for (int i = 0; i < max_conn_num; ++i) {
        if (sqlite3_close(connections_[i]) != SQLITE_OK) {
            log_error("error");
            exit(1);
        } else {
            printf("close db connection successfully\n");
        }
    }
}