#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include "utils.h"
#include "log.h"
#include <thread>
#include <iostream>
#include <functional>
#include <ranges>
#include <fstream>
#include <nlohmann/json.hpp>

TEST_CASE("BlockingQueue works fine", "[unit][blocking-queue]") {
    using namespace myserver::utils;
    BlockingQueue<int> bq;
    constexpr int producer_number = 5;
    
    int finished_number{};
    std::mutex mutex;
 
    bool bmp[producer_number * 10]{};
    std::mutex bmp_mutex;

    std::vector<std::thread> producers;
    for (int i{}; i < producer_number; ++i) {
        producers.emplace_back([&bq, i, &finished_number, &mutex]() {
            for (int j{}; j < 10; ++j) {
                bq.push(i * 10 + j);
            }
            {
                std::lock_guard guard(mutex);
                ++finished_number;
            }
        });
    }

    std::thread consumer([&bq, &finished_number, &mutex, &producer_number, &bmp, &bmp_mutex]() {
        bool finish = false;
        {
            std::lock_guard guard(mutex);
            finish = finished_number == producer_number;
        }
        while (!finish || !bq.empty()) {
            int i = bq.pop();
            {
                std::lock_guard guard(bmp_mutex);
                bmp[i] = true;
            }
            {
                std::lock_guard guard(mutex);
                finish = finished_number == producer_number;
            }
        }
    });

    consumer.join();
    for (auto &t : producers) {
        t.join();
    }

    REQUIRE(std::ranges::all_of(bmp, std::identity{}));
}

TEST_CASE("logging system works fine", "[unit][log]") {
    using namespace myserver;
    Log::instance().init(LOG_STORE_PATH, 1000, 1024, true);
    log_info("hello, %s", "info");
    log_debug("hello, %s", "debug");
    log_error("hello, %s", "error");
    log_warn("hello, %s", "warn");
    using namespace std::literals;
    std::this_thread::sleep_for(2s);
    Log::instance().terminate();
    auto log_path = Log::instance().log_file_path();
    std::ifstream in(log_path, std::ios::in | std::ios::ate);
    int size = in.tellg();
    in.seekg(std::ios::beg);
    char *all = new char[size + 1];
    in.read(all, size);
    in.close();
    all[size] = '\n';
    std::string s(all);
    delete[] all;
    REQUIRE_THAT(s, Catch::Matchers::ContainsSubstring("hello, info"));
    REQUIRE_THAT(s, Catch::Matchers::ContainsSubstring("hello, debug"));
    REQUIRE_THAT(s, Catch::Matchers::ContainsSubstring("hello, error"));
    REQUIRE_THAT(s, Catch::Matchers::ContainsSubstring("hello, warn"));
}

TEST_CASE("json library usage", "[learn]") {
    using json = nlohmann::json;
}