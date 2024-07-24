#include <iostream>
#include <format>

int main(int argc, char const *argv[]) {
    using namespace std::literals;
    auto constexpr format = "{}-{}"sv;
    std::cout << std::format(format, 1, 2) << std::endl;
    return 0;
}
