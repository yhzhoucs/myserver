set(MYSERVERLIB_SOURCE
        db.cpp
        log.cpp
        tcp_connection.cpp
        arcade.cpp)

add_library(myserverlib STATIC ${MYSERVERLIB_SOURCE})
add_library(myserver::myserverlib ALIAS myserverlib)
target_include_directories(myserverlib PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include)
target_compile_definitions(myserverlib PRIVATE ENABLE_LOG)

# Add sqlite3 support. Ensure you have sqlite3 and its library 
# installed in your machine.
find_package(SQLite3 REQUIRED)
# Add uuid support. Ensure you have libuuid installed.
find_package(LibUUID REQUIRED)
target_link_libraries(myserverlib PUBLIC SQLite::SQLite3 LibUUID::LibUUID)
# Add json support. Dealing with server<->client message in json.
include(FetchContent)
FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)
target_link_libraries(myserverlib PUBLIC nlohmann_json::nlohmann_json)

add_executable(myserver main.cpp)
target_link_libraries(myserver PRIVATE myserver::myserverlib)
target_compile_definitions(myserver PRIVATE LOG_STORE_PATH="${PROJECT_BINARY_DIR}/logs/")
target_compile_definitions(myserver PRIVATE DATABASE_PATH="${PROJECT_SOURCE_DIR}/myserver.db")