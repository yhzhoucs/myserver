#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include "blocking_queue.h"
#include <thread>
#include <iostream>
#include <functional>
#include <ranges>

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