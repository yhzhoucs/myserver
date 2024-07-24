Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.6.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(test_units test_units.cpp)
target_link_libraries(test_units PRIVATE Catch2::Catch2WithMain myserver::myserverlib)
add_test(NAME "test_units" COMMAND test_units)