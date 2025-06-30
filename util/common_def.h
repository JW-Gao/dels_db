#pragma once

// for debug
#ifdef DEBUG
    #define dbg_assert(cond)                                                   \
        do {                                                                   \
            if (!(cond)) {                                                     \
                std::cerr << "Assertion failed: " << #cond                     \
                          << ", file " << __FILE__                             \
                          << ", line " << __LINE__ << std::endl;               \
                std::abort();                                                  \
            }                                                                  \
        } while (0)
#else
    #define dbg_assert(cond) ((void)0) // Do nothing in release mode
#endif
#include <cstdint>

#define LOG_ERROR(args) \
    do {                \
        printf("[ERROR] "); \
        printf args;        \
        printf("\n");       \
    } while(0)

#define LOG_INFO(args)  \
    do {                \
        printf("[INFO ] "); \
        printf args;        \
        printf("\n");       \
    } while(0)

#define LOG_TRACE(args) \
    do {                \
        printf("[TRACE] "); \
        printf args;        \
        printf("\n");       \
    } while(0)


// Define likely/unlikely macros for branch prediction
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)




#define NO_COPY_MOVE(ClassName)            \
    ClassName(const ClassName&) = delete; \
    ClassName(ClassName&&) = delete;      \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName& operator=(ClassName&&) = delete;


#define NO_COPY(ClassName)            \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete;





// for alignment
#define ALIGNED(x) __attribute__((aligned(x)))

// for prefetch
#define PREFETCH(addr, rw, locality) __builtin_prefetch((addr), (rw), (locality))

#define CACHE_LINE_FETCH_ALIGN 64

#define tcs_t int32_t
using tcs_e = tcs_t;
#define DEFAULT_TCS_VAL -1
#define DEFAULT_TCS (tcs_t)DEFAULT_TCS_VAL
