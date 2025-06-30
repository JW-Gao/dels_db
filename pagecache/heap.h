#pragma once
#include "def_types.h"
#include <tuple>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <memory>
#include "../util/concurrent_stack.h"


using SlabId = uint8_t;
using SlabIdx = uint32_t;

constexpr uint64_t MIN_TRAILING_ZEROS = 15; // 32 KB, 32 * 1024
constexpr uint64_t MIN_SZ = 1ULL << MIN_TRAILING_ZEROS;


// 下面是一些辅助函数

// next_power_of_two
inline uint64_t next_power_of_two(uint64_t n) {
    if (n == 0) return 1;
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}
// slab_id_to_size
inline uint64_t slab_id_to_size(SlabId slab_id) {
    return 1ULL << (MIN_TRAILING_ZEROS + slab_id);
}

// size_to_slab_id
inline SlabId size_to_slab_id(uint64_t size) {
  uint64_t normalized_size = std::max(MIN_SZ, next_power_of_two(size));
  uint64_t rebased_size = normalized_size >> MIN_TRAILING_ZEROS;

  uint32_t tz = static_cast<uint32_t>(rebased_size == 0 ? 0 : __builtin_ctzll(rebased_size));
  return static_cast<SlabId>(tz);
}


// slab_size
inline uint64_t slab_size(uint64_t size) {
    return slab_id_to_size(size_to_slab_id(size));
}

struct HeapId {
  uint64_t location;
  Lsn original_lsn;

  std::tuple<SlabId, SlabIdx, Lsn> decompose() const {
    constexpr uint64_t IDX_MASK = (1ULL << 32) - 1;
    // slab_id 是 location 的高32位中第一个1的下标
    // trailing_zeros 返回的是第一个1的位置
    uint32_t tz = static_cast<uint32_t>((location >> 32) == 0 ? 32 : __builtin_ctzll(location >> 32));
    SlabId slab_id = static_cast<SlabId>(tz);
    SlabIdx slab_idx = static_cast<SlabIdx>(location & IDX_MASK);
    return std::make_tuple(slab_id, slab_idx, original_lsn);
  }
      static HeapId compose(SlabId slab_id, SlabIdx slab_idx, Lsn original_lsn) {
        uint64_t slab = 1ULL << (32 + slab_id);
        uint64_t heap_id = slab | static_cast<uint64_t>(slab_idx);
        return HeapId{heap_id, original_lsn};
    }

    uint64_t offset() const {
        auto [slab_id, idx, _] = decompose();
        return slab_id_to_size(slab_id) * static_cast<uint64_t>(idx);
    }

    uint64_t slab_size() const {
        auto [slab_id, _, __] = decompose();
        return slab_id_to_size(slab_id);
    }
};


