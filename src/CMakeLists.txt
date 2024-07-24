set(MYSERVERLIB_SOURCE
        db.cpp
        log.cpp)

add_library(myserverlib STATIC ${MYSERVERLIB_SOURCE})
add_library(myserver::myserverlib ALIAS myserverlib)
target_include_directories(myserverlib PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include)
target_compile_definitions(myserverlib PRIVATE ENABLE_LOG)

# Add sqlite3 support. Ensure you have sqlite3 and its library 
# installed in your machine.
find_package(SQLite3 REQUIRED)
target_link_libraries(myserverlib PUBLIC SQLite::SQLite3)

target_link_libraries(myserverlib PUBLIC /lib/x86_64-linux-gnu/libuuid.so)

add_executable(myserver main.cpp)
target_link_libraries(myserver PRIVATE myserver::myserverlib)
target_compile_definitions(myserver PRIVATE LOG_STORE_PATH="${PROJECT_BINARY_DIR}/logs/")