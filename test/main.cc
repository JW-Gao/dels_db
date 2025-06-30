// #include "test_epoch.h"
// #include "test_skiplist.h"
#include "../util/cache_padded.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

int main() {
    // CconcurrentSkipListTest::concurrent_exchange_test();
    std::unique_ptr<int> ptr = std::make_unique<int>(42);
    ptr = std::make_unique<int>(100); // 原先的指针将被释放
    std::cout << "Value: " << *ptr << std::endl; // 输出
    return 0;
}