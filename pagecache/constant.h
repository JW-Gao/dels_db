#pragma  once
#include "def_types.h"

// 
const size_t MAX_MSG_HEADER_LEN = 32;

// Log header length
const uint32_t SEG_HEADER_LEN = 20;

// #[allow(unused)]
const double MAX_SPACE_AMPLIFICATION = 10.0;

const PageId META_PID = 0;

const PageId COUNTER_PID = 1;

const PageId BATCH_MANIFEST_PID = UINT64_MAX - 666;


const size_t PAGE_CONSOLIDATION_THRESHOLD = 10;
const size_t SEGMENT_CLEANUP_THRESHOLD = 50;


// // Allows for around 1 trillion items to be stored
// // 2^37 * (assuming 50% node fill, 8 items per leaf)
// // and well below 1% of nodes being non-leaf nodes.
// #[cfg(target_pointer_width = "64")]

// pub(crate) const MAX_PID_BITS: usize = 37;
// const size_t MAX_PID_BITS = 37;

// const size_t MAX_BLOB = 1 << 37;

// 平台检测宏（通常由编译器预定义）
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
    #define TARGET_64BIT 1
#else
    #define TARGET_32BIT 1
#endif

// 页面ID位数配置
#if TARGET_64BIT
    const size_t MAX_PID_BITS = 37;
#elif TARGET_32BIT
    const size_t MAX_PID_BITS = 32;
#endif

// BLOB大小限制
#if TARGET_64BIT
    const size_t MAX_BLOB = 1UL << 37; // 128GB
#elif TARGET_32BIT
    const size_t MAX_BLOB = 1UL << 29; // 512MB
#endif