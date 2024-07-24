set(MYSERVERLIB_SOURCE
    db.cpp)

add_library(myserverlib STATIC ${MYSERVERLIB_SOURCE})
add_library(myserver::myserverlib ALIAS myserverlib)
target_include_directories(myserverlib PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include)

# Add sqlite3 support. Ensure you have sqlite3 and its library 
# installed in your machine.
find_package(SQLite3 REQUIRED)
target_link_libraries(myserverlib PUBLIC SQLite::SQLite3)

add_executable(myserver main.cpp)
target_link_libraries(myserver PRIVATE myserver::myserverlib)