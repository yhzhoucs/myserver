#include <iostream>
#include <db.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    using namespace myserver;
    DB db("myserver.db");
    sleep(5);
    return 0;
}
