cmake_minimum_required(VERSION 3.20.0)

project(myserver
    VERSION 0.0.1
    DESCRIPTION "Learn C++ network programming"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 REQUIRED)

add_subdirectory(src)