#include "db.h"
#include "log.h"
#include <stdlib.h>
#include <stdio.h>

myserver::DB::DB(std::string const &db_file) {
    for (int i = 0; i < max_conn_num; ++i) {
        if (sqlite3_open(db_file.c_str(), &connections_[wr_ptr_++]) != SQLITE_OK) {
            LOG_ERROR("error");
            exit(1);
        } else {
            printf("create db connection successfully\n");
        }
    }
}

myserver::DB::~DB() {
    for (int i = 0; i < max_conn_num; ++i) {
        if (sqlite3_close(connections_[i]) != SQLITE_OK) {
            LOG_ERROR("error");
            exit(1);
        } else {
            printf("close db connection successfully\n");
        }
    }
}