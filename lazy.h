// #pragma once

// #include <atomic>
// #include <functional>
// #include <cassert>
// #include <thread>
// #include <utility>
// #include <new>
// #include <cstddef>

// template<typename T, typename F>
// class Lazy {
//     std::atomic<void*> value_;
//     std::atomic<bool> init_mu_;
//     F init_;

// public:
//     explicit Lazy(F&& init)
//         : value_(nullptr), init_mu_(false), init_(std::forward<F>(init)) {}

//     ~Lazy() {
//         void* ptr = value_.load(std::memory_order_acquire);
//         if (ptr) {
//             delete static_cast<T*>(ptr);
//         }
//     }

//     // 禁止拷贝
//     Lazy(const Lazy&) = delete;
//     Lazy& operator=(const Lazy&) = delete;

//     // 允许移动
//     Lazy(Lazy&&) = default;
//     Lazy& operator=(Lazy&&) = default;

//     T& get() const {
//         void* ptr = value_.load(std::memory_order_acquire);
//         if (ptr) {
//             return *static_cast<T*>(ptr);
//         }

//         // 用自旋锁保护初始化
//         while (init_mu_.exchange(true, std::memory_order_acq_rel)) {
//             // std::this_thread::yield(); // 或者
//             [[maybe_unused]] volatile int dummy = 0; // 空操作防止优化
//         }

//         // double-checked locking
//         ptr = value_.load(std::memory_order_acquire);
//         if (!ptr) {
//             T* value = new T(init_());
//             value_.store(static_cast<void*>(value), std::memory_order_release);
//             // 解锁
//             bool old = init_mu_.exchange(false, std::memory_order_release);
//             assert(old);
//             return *value;
//         } else {
//             bool old = init_mu_.exchange(false, std::memory_order_release);
//             assert(old);
//             return *static_cast<T*>(ptr);
//         }
//     }

//     T* operator->() const { return &get(); }
//     T& operator*() const { return get(); }
// };

