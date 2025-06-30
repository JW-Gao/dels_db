#pragma once
#include "common_def.h"
#include <iostream>
#include <memory>
#include <type_traits>

template <typename T>
class alignas(CACHE_LINE_FETCH_ALIGN) cache_padded_t {
    static_assert(std::is_default_constructible<T>::value, "T must be default constructible");

public:
    // 默认构造函数
    cache_padded_t() = default;
    
    template <typename... Args>
    explicit cache_padded_t(Args&&... args)
        : value_(std::forward<Args>(args)...) {}
    // 构造函数，接收一个值
    // explicit cache_padded_t(const T& value) : value_(value) {}

    // // 移动构造函数
    // explicit cache_padded_t(T&& value) noexcept : value_(std::move(value)) {}

    // 拷贝构造函数
    cache_padded_t(const cache_padded_t& other) = default;

    // 移动构造函数
    cache_padded_t(cache_padded_t&& other) noexcept = default;

    // // 拷贝赋值运算符
    // cache_padded_t& operator=(const cache_padded_t& other) = default;

    // // 移动赋值运算符
    // cache_padded_t& operator=(cache_padded_t&& other) noexcept = default;

    // 获取内部值（常量引用）
    const T& value() const noexcept { return value_; }

    // 获取内部值（非常量引用）
    T& value() noexcept { return value_; }

    // 隐式转换为 T 的常量引用
    operator const T&() const noexcept { return value_; }

    // 隐式转换为 T 的非常量引用
    operator T&() noexcept { return value_; }

    // 重载函数调用运算符，返回内部值的引用
    T& operator()() noexcept { return value_; }
    const T& operator()() const noexcept { return value_; }

private:
    T value_; // 实际存储的值
};
