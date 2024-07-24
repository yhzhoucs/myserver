cmake_minimum_required(VERSION 3.20.0)

project(myserver
    VERSION 0.0.1
    DESCRIPTION "Learn C++ network programming"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

option(ENABLE_TEST "Build unit tests using catch2" ON)

add_subdirectory(src)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND ENABLE_TEST)
    include(CTest)
    add_subdirectory(test)
endif ()