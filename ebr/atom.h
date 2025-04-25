#pragma once
#include <atomic>
#include <memory>
#include <cstdlib>
#include <type_traits>

constexpr std::memory_order strongest_failure_ordering(std::memory_order ord) {
  std::memory_order ans;
  switch (ord) {
  case std::memory_order_relaxed:
  case std::memory_order_release:
    ans = std::memory_order_relaxed;
    break;
  case std::memory_order_acquire:
  case std::memory_order_acq_rel:
    ans = std::memory_order_acquire;
    break;
  default:
    ans = std::memory_order_seq_cst;
    break;
  }
  return ans;
}

// 基础抽象类模板
template <typename T>
class Pointable {
public:
    // 对齐要求（需由具体特化实现）
    static constexpr size_t ALIGN = alignof(T);

    // 初始化器类型（可根据需要特化）
    using Init = typename std::decay<T>::type;

    // 初始化对象，返回地址
    static uintptr_t init(Init init) {
        // 分配对齐内存
        void* ptr = aligned_alloc(ALIGN, sizeof(T));
        if (!ptr) throw std::bad_alloc();
        
        // placement new 构造对象
        new (ptr) T(std::forward<Init>(init));
        return reinterpret_cast<uintptr_t>(ptr);
    }


    // 删除拷贝和移动
    Pointable() = delete;
    Pointable(const Pointable&) = delete;
    Pointable& operator=(const Pointable&) = delete;
};

template<typename T, typename P>
struct CompareAndSetError {
  Shared<T> current;
  P new_v;
};

#include <atomic>
#include <memory>
#include <cstdint>
#include <stdexcept>

// 假设 Pointable 已实现（根据之前代码）
template <typename T>
class Pointable {
public:
    static uintptr_t init(typename T::Init init);
    static T& deref(uintptr_t ptr);
    static void drop(uintptr_t ptr);
};

// 标记位处理（假设低2位用于标记）
constexpr uintptr_t TAG_MASK = 0b11;
constexpr size_t POINTER_ALIGN = 4; // 确保指针地址对齐到4字节

template <typename T>
constexpr uintptr_t pack_ptr(T* ptr, uintptr_t tag = 0) {
    return reinterpret_cast<uintptr_t>(ptr) | (tag & TAG_MASK);
}

template <typename T>
T* unpack_ptr(uintptr_t packed) {
    return reinterpret_cast<T*>(packed & ~TAG_MASK);
}

template <typename T>
uintptr_t get_tag(uintptr_t packed) {
    return packed & TAG_MASK;
}

// 所有权管理包装器
template <typename T>
class Owned {
    uintptr_t packed_;
public:
    explicit Owned(T* ptr) : packed_(pack_ptr<T>(ptr)) {}
    
    uintptr_t into_usize() const { return packed_; }
    
    ~Owned() {
        if (packed_ != 0) {
            Pointable<T>::drop(packed_);
        }
    }
};

template <typename T>
class Shared {
    uintptr_t packed_;
public:
    explicit Shared(uintptr_t packed) : packed_(packed) {}
    
    T& deref() const { return Pointable<T>::deref(packed_); }
    uintptr_t into_usize() const { return packed_; }
};


