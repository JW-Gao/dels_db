#pragma once
#include "def_types.h"
#include "heap.h"

// 首先完成只支持inline 形式的外存储日志结构
struct DiskPtr {
  bool inline_flag = false; // whether heap allocation
  union {
    LogOffset offset; // used for non-heap allocation
    HeapId heap_id; // used for heap allocation
  };
  static DiskPtr new_inline(LogOffset offset) {
    DiskPtr ptr;
    ptr.inline_flag = true;
    ptr.offset = offset;
    return ptr;
  }

  bool is_inline() const {
    return inline_flag;
  }

  LogOffset lid() const {
    if (inline_flag) {
      return offset;
    } else {
      return heap_id.location;
    }
  }
};



