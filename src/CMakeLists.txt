add_library(db STATIC db.cpp)
add_library(myserver::db ALIAS db)
target_include_directories(db PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include)
target_link_libraries(db PUBLIC SQLite::SQLite3)

add_executable(myserver main.cpp)
target_link_libraries(myserver PRIVATE myserver::db)