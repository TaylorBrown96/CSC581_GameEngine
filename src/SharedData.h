#pragma once
#include <atomic>
#include <mutex>
#include <string>
#include <condition_variable>

struct SharedData {
    std::atomic<size_t> nextJobIndex{0};
};
