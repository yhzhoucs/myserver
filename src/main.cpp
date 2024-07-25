#include <iostream>
#include <format>
#include "db.h"
#include "log.h"

int main(int argc, char const *argv[]) {
    using namespace myserver;

    Log::instance().init(LOG_STORE_PATH, 1000, 1024, true);
    DB::instance().init(DATABASE_PATH);
    UserCache::instance().init();

    std::this_thread::sleep_for(std::chrono::seconds(2));
    myserver::Log::instance().terminate();
    return 0;
}
