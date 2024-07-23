#include "db.h"

myserver::DB::DB(std::string const &db_file) {
    for (int i = 0; i < max_conn_num; ++i) {
        if (sqlite3_open(db_file.c_str(), &connections_[wr_ptr_++]) != SQLITE_OK) {
            
        }
    }
}