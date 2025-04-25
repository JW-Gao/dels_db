#pragma once
#include <cstddef>
#include <emmintrin.h>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <optional>
#include <variant>
#include <memory>
#include <thread>
#include <immintrin.h>

// write bit位
unsigned REQUIRED_BIT = (1 << 31);


// read guard, RAII
using ReadGuard = std::shared_lock<std::shared_mutex>;

// write guard, RAII
using WriteGuard = std::unique_lock<std::shared_mutex>;

using AtomicUsize = std::atomic<size_t>;



struct Protector {

  using NoneType = AtomicUsize *;
  // enum
  using VariantType= std::variant<ReadGuard, WriteGuard, NoneType>;


  explicit Protector(WriteGuard wg) : inner_(std::move(wg)) {}

  explicit Protector(ReadGuard rg) : inner_(std::move(rg)) {}

  explicit Protector(NoneType shared_cnt) : inner_(shared_cnt) {}

  // bool is_write() const { return std::holds_alternative<WriteGuard>(inner_); }
  // bool is_read() const { return std::}


  ~Protector() {
    if (std::holds_alternative<NoneType>(inner_)) {
      std::get<NoneType>(inner_)->fetch_sub(1, std::memory_order_release);
    }
  }

  VariantType inner_;
};

class ConcurrencyControl {

  std::atomic<size_t> active_;

  std::shared_mutex mu_;

  std::atomic<bool> upgrade_done_;

  // 一次升级，永久有效
  void enable() {
    size_t active = active_.fetch_or(REQUIRED_BIT, std::memory_order::seq_cst);
    
    // 第一个设置set bit的
    if (active < REQUIRED_BIT) {
      while (active_.load(std::memory_order_acquire) != REQUIRED_BIT) {
        // spin loop
        _mm_pause();
      }
      upgrade_done_.store(true, std::memory_order_release);
    }
  }
public:
  ConcurrencyControl() = default;

  Protector read() {
    // typename Protector::ProtectorType type = Protector::ProtectorType::None;
    uint shard = active_.fetch_add(1, std::memory_order_release);

    if (shard > REQUIRED_BIT) {
      active_.fetch_sub(1, std::memory_order_release);
      return Protector(ReadGuard(mu_));
    }

    return Protector(&active_);
  }

  Protector write() {
    this->enable();
    while (!upgrade_done_.load(std::memory_order_acquire)) {
      // spin loop
      _mm_pause();
    }
    return Protector(WriteGuard(mu_));
  }
};

// test
