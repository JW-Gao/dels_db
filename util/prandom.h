#pragma once
#include <cstdint>

// Crandom
class Crandom {
public:
    // using result_type = uint32_t;

    /* 1.  public 常量：next() 的最大返回值                       */
    static constexpr uint32_t kMaxNext = 0x0fffffffu;

    /* 2.  构造函数：允许用户指定种子，缺省给一个固定常量         */
    explicit Crandom(uint32_t seed = 0xdeadbeefu) : state_(seed ? seed : 0xdeadbeefu) {}

    /* 3.  返回 [0, kMaxNext] 的均匀随机数                       */
    inline uint32_t next() {
        // xorshift32
        uint32_t x = 0;
        do {
            x = state_;
            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 5;
            state_ = x;
        } while (x >= kMaxNext); // 拒绝大于等于 kMaxNext 的结果
        return x;
    }

private:
    /* 4.  32-bit 内部状态                                       */
    uint32_t state_;
};