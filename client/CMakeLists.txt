# A standalone client for testing server functionality.

add_executable(client client.cpp)
target_link_libraries(client PRIVATE nlohmann_json::nlohmann_json)