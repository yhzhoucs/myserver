# A standalone client for testing server functionality.

cmake_minimum_required(VERSION 3.20.0)
project(client
    VERSION 0.0.1
    DESCRIPTION "A simple client written in C++ for my server"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(client main.cpp client.cpp client.h)

# Add json support. Dealing with server<->client message in json.
include(FetchContent)
FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)
target_link_libraries(client PRIVATE nlohmann_json::nlohmann_json)