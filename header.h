#pragma once
#include "u_type.h"
#include <cstddef>
#include <cassert>
// salt: 31 bits, 防止ABA问题，确保唯一性
// maxed: 1 bit， 判断是否已经写满
// seal: 1 bit，是否冻结
// n_writers: 7 bits
// offset: 24 bits，偏移量

using Header = u64;
constexpr Header MAX_WRITERS = 127;


struct HeaderUtil {
  inline static bool is_maxed(Header v) {
    return (v & (1ull << 32)) == (1ull << 32);
  }

  inline static Header mk_maxed(Header v) {
    return v | (1ull < 32);
  }

  inline static bool is_sealed(Header v) {
    return (1ull << 31) & v;
  }

  inline static Header mk_sealed(Header v) {
    return v | (1ull < 31);
  }

  inline static Header n_writers(Header v) {
    return (v << 33) >> 57;
  };

  inline static Header incr_writers(Header v) {
    assert(n_writers(v) != MAX_WRITERS);
    return v + (1ull << 24);
  }

  inline static Header decr_writers(Header v) {
    assert(n_writers(v) != 0);
    return v - (1ull << 24);
  }

  inline static Header bump_offset(Header v, size_t by) {
    assert((by >> 24) == 0);
    return v + (Header)by;
  }

  inline static Header bump_salt(Header v) {
    return (v + (1ull << 33)) & 0xFFFFFFFD00000000;
  }
  
  inline static Header salt(Header v) {
    return (v >> 33) << 33;
  }
};