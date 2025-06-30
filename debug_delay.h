#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <thread>
#include <algorithm>
#include <string>
#include <mutex>
#include <iostream>
#include <climits>

uint32_t get_env_u32(const char* name, uint32_t default_val) {
    const char* val = std::getenv(name);
    if (!val) return default_val;
    try {
        return static_cast<uint32_t>(std::stoul(val));
    } catch (...) {
        std::cerr << "Environment variable " << name << " must be a non-negative integer\n";
        std::exit(1);
    }
}

// 懒加载（线程安全单例实现）
uint32_t get_INTENSITY() {
    static std::once_flag flag;
    static uint32_t value;
    std::call_once(flag, []() {
        value = get_env_u32("SLED_LOCK_FREE_DELAY_INTENSITY", 100);
    });
    return value;
}

uint32_t get_CRASH_CHANCE() {
    static std::once_flag flag;
    static uint32_t value;
    std::call_once(flag, []() {
        value = get_env_u32("SLED_CRASH_CHANCE", 0);
    });
    return value;
}

// 线程本地延迟计数
thread_local size_t LOCAL_DELAYS = 0;

// 全局延迟计数
static std::atomic<size_t> GLOBAL_DELAYS{0};

// Xorshift32 + Lemire (fast mod)
uint32_t random_u32(uint32_t n) {
    thread_local uint32_t x = 1406868647U;
    // Xorshift32
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    // Lemire's fast modulo
    return static_cast<uint32_t>((uint64_t(x) * uint64_t(n)) >> 32);
}

void debug_delay() {
    // 省略 miri_optimizations 相关特性（一般 C++ 不用）
    
    size_t global_delays = GLOBAL_DELAYS.fetch_add(1, std::memory_order_relaxed);

    size_t old = LOCAL_DELAYS;
    size_t new_val = std::max(global_delays + 1, old + 1);
    LOCAL_DELAYS = new_val;

    if (get_CRASH_CHANCE() > 0 && random_u32(get_CRASH_CHANCE()) == 0) {
        std::exit(9);
    }

    if (global_delays == old) {
        // 没有其它线程调用，直接返回
        return;
    }

    if (random_u32(1000) == 1) {
        uint32_t duration = random_u32(get_INTENSITY());
        std::this_thread::sleep_for(std::chrono::microseconds(duration));
    }

    if (random_u32(2) == 0) {
        std::this_thread::yield();
    }
}