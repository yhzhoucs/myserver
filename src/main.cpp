#include <format>
#include "db.h"
#include "log.h"
#include "arcade.h"
#include "thread_pool.h"
#include "server.h"

int main(int argc, char const *argv[]) {
    using namespace myserver;

    Log::instance().init(LOG_STORE_PATH, 1000, 1024, true);
    DB::instance().init(DATABASE_PATH);
    UserCache::instance().init();
    Arcade::instance().init();
    ThreadPool::instance().init();

    Server server("127.0.0.1", 8848);
    server.start_server();

    return 0;
}
