#pragma once
#include "common_def.h"
#include "prandom.h"
#include <deque>
struct node_t;
struct reclaim_t {
    // Placeholder for reclaim context, can be extended with more fields if needed.
    std::deque<node_t *> nodes;
    reclaim_t() = default;
    bool empty() const { return nodes.empty(); }
    
};
struct thread_context_t final {
    Crandom rnd; // Random number generator for this thread.
    tcs_t slots[128]; // Thread local slots, for epoch reclaim.
    reclaim_t reclaim; // Reclaim context for cooperative deletion.
    bool reclaim_in_use{false}; // Flag to indicate if reclaim container is in use.

    thread_context_t() = default;
};