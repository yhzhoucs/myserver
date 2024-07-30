# A standalone client for testing server functionality.

add_executable(client main.cpp client.cpp client.h)
target_link_libraries(client PRIVATE nlohmann_json::nlohmann_json)