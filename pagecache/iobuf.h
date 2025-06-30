#pragma once

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <atomic>
#include <cassert>

// Lsn, LogOffset, PageId
#include "def_types.h"
#include "../util/cache_padded.h"
#include "../header.h"
#include "../slice.h"
#include "constant.h"
#include "logger.h"


// 8KB 对齐的缓冲区
struct AlignedBuf {
  unsigned char *ptr;
  size_t len;


public:
  AlignedBuf(size_t len) : len(len) {
    constexpr size_t alignment = 8192; // 8KB 对齐
    size_t aligned_size = (len + alignment - 1) & ~(alignment - 1);
    ptr = static_cast<unsigned char*>(std::aligned_alloc(alignment, aligned_size));
  }

  // 禁止拷贝，允许移动
  AlignedBuf(const AlignedBuf&) = delete;
  AlignedBuf& operator=(const AlignedBuf&) = delete;
  AlignedBuf(AlignedBuf&& other) noexcept : ptr(other.ptr), len(other.len) {
      other.ptr = nullptr;
      other.len = 0;
  }

  ~AlignedBuf() {
    if (ptr) {
      std::free(ptr);
    }
  }
};



// IoBuf,
// 一个AlignedBuf 可能包含多个不同的IoBuf,每个IoBuf都有不同的base
class IoBuf {
  std::shared_ptr<std::atomic<AlignedBuf *>> buf_;
  cache_padded_t<std::atomic<uint64_t>> header_;
  size_t base_; // 当前IoBuf在AlignedBuf中的偏移量
  bool from_tip_;
  Lsn stored_max_stable_lsn_;
public:
  LogOffset offset_;
  Lsn lsn_;
  size_t capacity_;



private:

  // 一个新的segment初始化的时候会调用这个函数
  // 在数据恢复的时候，会读取buffer 头部的segment header
  void store_segment_header(Header last, Lsn lsn, Lsn max_stable_lsn) {
    assert(capacity_ >= SEG_HEADER_LEN);
    unsigned char *buf_ptr = buf_->load()->ptr;
    stored_max_stable_lsn_ = max_stable_lsn;
    lsn_ = lsn;
    SegmentHeader header = SegmentHeader {lsn, max_stable_lsn, true};
    unsigned char seg_header_buf[SEG_HEADER_LEN] = {0};

    header.to_char(seg_header_buf);
    std::memcpy(buf_ptr, seg_header_buf, SEG_HEADER_LEN);

    // 这里有问题
    auto last_salt = HeaderUtil::salt(last);
    auto new_salt = HeaderUtil::bump_salt(last_salt);
    auto bumped = HeaderUtil::bump_offset(new_salt, SEG_HEADER_LEN);
    this->set_header(bumped);
  }
public:
  SliceMut get_mut_range(size_t at, size_t len) { // at是偏移量，len是长度
    unsigned char *buf_ptr = buf_->load()->ptr;
    size_t buf_len = buf_->load()->len;
    assert(at + len <= buf_len);
    return SliceMut(buf_ptr + this->base_ + at, len);
  }

  void set_header(Header v) {
    header_().store(v, std::memory_order_release);
  }

};