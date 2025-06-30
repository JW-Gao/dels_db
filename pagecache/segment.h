#pragma once

/*
SegmentAccountant 是底层存储文件（segment）等大小块的分配器。

它必须维护以下关键安全属性：

A. 当某个 segment 里包含页面的最新稳定状态时，绝不能覆盖该 segment。

B. 当活跃线程可能持有指向这些 segment 的 LogOffset 时，绝不能覆盖这些 segment。

让事情变得复杂的是，PageCache 只知道自己何时把页面写入了 IO 缓冲区，但它并不追踪这些 IO 缓冲区何时被稳定化（直到实现写合并为止）。

为了保证上述安全性，采取了如下措施：

延迟重用已有的 segment，确保只有当写入该 segment 以及更早 segment 的所有数据都已经写入磁盘并 fsync 后，才会将该 segment 设为非活跃状态（可重用）。

我们在 IoBufs::write_to_log 中使用了 epoch::Guard::defer()，它能保证在未来要重用某个 segment 时，会延迟 segment 的失效，直到所有可能访问该 segment 的线程都已结束。

还有另一个问题：由于 IO 缓冲区可能是乱序写入的，如何恢复 segment 就成了难题。如果最近写入的 segment 丢失了数据，我们必须小心地保证日志的“线性化”特性。为此，我们必须能够检测“撕裂的 segment”（即因崩溃未能完全写入的 segment）。

但如果我们在写入前一个 segment 之前就把后一个 segment 写完了，然后发生了崩溃怎么办？我们必须保持线性化，因此当前一个 segment 在崩溃中丢失时，绝不能恢复后一个 segment。

这个问题的解决方法是引入“不稳定尾部”的概念：在恢复时，这些 segment 必须作为恢复出来的、LSN（日志序号）最高、且连续的 segment 出现。只要 segment 还属于这个“不稳定尾部”，就禁止重用。只有当后续更高的 segment 在自己的头部写下比我们更高的“稳定连续 lsn”时，我们才允许重用这些 segment
*/

#include <mutex>
#include <map>
#include <set>
#include <cassert>

#include "def_types.h"
#include "snapshot.h"
#include "pagecache.h"
#include "../config.h"

enum SegmentOpType {
  Link,
  Replace,
};

class SegmentOp {
public:
  SegmentOpType type_;

  PageId page_id_; // 这个操作涉及的页面ID

  // for link
  CacheInfo cache_;

  // for replace
  std::vector<CacheInfo> old_caches_;
  CacheInfo new_cache_; // 替换后的新缓存信息

public:
  

};

enum SegmentState {
  SegFree,
  SegActive,
  SegInactive,
  SegDraining, // 正在清理的状态
};

// segmeng and its inheritance

// 基类
// 基类
class Segment {
public:
    virtual ~Segment() = default;

    // 状态判别
    virtual bool is_free()    const { return false; }
    virtual bool is_active()  const { return false; }
    virtual bool is_inactive()const { return false; }
    virtual bool is_draining()const { return false; }
    
    virtual SegmentState seg_state() const = 0;

    // 状态转换接口（返回新状态！）
    virtual std::unique_ptr<Segment> free_to_active(Lsn)                        { throw std::logic_error("Not implemented"); }
    virtual std::pair<std::unique_ptr<Segment>, std::unordered_set<Lsn>> active_to_inactive(Lsn, const RunningConfig&) { throw std::logic_error("Not implemented"); }
    virtual std::pair<std::unique_ptr<Segment>, std::unordered_set<PageId>> inactive_to_draining(Lsn)      { throw std::logic_error("Not implemented"); }
    virtual std::pair<std::unique_ptr<Segment>, Lsn> draining_to_free(Lsn)      { throw std::logic_error("Not implemented"); }

    virtual void defer_free_lsn(Lsn) { throw std::logic_error("Not implemented"); }
    virtual void recovery_ensure_initialized(Lsn) { throw std::logic_error("Not implemented"); }
    virtual Lsn lsn() const { throw std::logic_error("Not implemented"); }
    virtual void insert_pid(PageId, Lsn) { throw std::logic_error("Not implemented"); }
    virtual void remove_pid(PageId, Lsn) { throw std::logic_error("Not implemented"); }
    virtual void remove_heap_item(HeapId, const RunningConfig&) { throw std::logic_error("Not implemented"); }
    virtual bool can_free() const { return false; }
};


// 声明
class ActiveSegment;

// ---- Free 状态 ----
class FreeSegment : public Segment {
public:
    std::optional<Lsn> previous_lsn;

    FreeSegment(std::optional<Lsn> prev = std::nullopt) : previous_lsn(prev) {}

    bool is_free() const override { return true; }


    SegmentState seg_state() const override { return SegFree; }

    std::unique_ptr<Segment> free_to_active(Lsn new_lsn) override;

    void recovery_ensure_initialized(Lsn lsn) override {
        // 在外部调用 free_to_active/replace即可
        throw std::logic_error("recovery_ensure_initialized should be handled externally for Free");
    }
};


// Active 状态
// ---- Active 状态 ----
class ActiveSegment : public Segment {
public:
    Lsn lsn_;
    std::unordered_set<PageId> deferred_replaced_pids;
    std::unordered_set<PageId> pids;
    Lsn latest_replacement_lsn = 0;
    std::unordered_set<Lsn> can_free_upon_deactivation;
    // std::unordered_set<HeapId> deferred_heap_removals;

    ActiveSegment(Lsn lsnv)
        : lsn_(lsnv) {}

    bool is_active() const override { return true; }


    Lsn lsn() const override { return lsn_; }


    SegmentState seg_state() const override { return SegActive; }


    void insert_pid(PageId pid, Lsn lsn) override {
        assert(lsn == lsn_);
        pids.insert(pid);
    }

    void defer_free_lsn(Lsn lsn) override {
        can_free_upon_deactivation.insert(lsn);
    }

    void remove_pid(PageId pid, Lsn replacement_lsn) override {
        assert(lsn_ <= replacement_lsn);
        if (replacement_lsn != lsn_) {
            deferred_replaced_pids.insert(pid);
        }
        if (replacement_lsn > latest_replacement_lsn) {
            latest_replacement_lsn = replacement_lsn;
        }
    }

    // void remove_heap_item(HeapId heap_id, const RunningConfig&) override {
    //     deferred_heap_removals.insert(heap_id);
    // }

    std::pair<std::unique_ptr<Segment>, std::unordered_set<Lsn>>
    active_to_inactive(Lsn to_lsn, const RunningConfig& config) override {
        assert(to_lsn >= lsn_);
        // 处理延迟heap移除
        // for (auto heap_id : deferred_heap_removals)
        //     config.heap.free(heap_id);

        size_t max_pids = pids.size();

        // 复制并移除deferred_replaced_pids
        std::unordered_set<PageId> pids_copy = pids;
        for (auto pid : deferred_replaced_pids) {
            pids_copy.erase(pid);
        }

        auto inact = std::make_unique<InactiveSegment>(
            lsn_,
            std::move(pids_copy),
            max_pids,
            deferred_replaced_pids.size(),
            latest_replacement_lsn
        );
        std::unordered_set<Lsn> can_free = std::move(can_free_upon_deactivation);
        // 返回新状态和can_free集合
        return {std::move(inact), std::move(can_free)};
    }
};

// ---- Inactive 状态 ----
class InactiveSegment : public Segment {
public:
    Lsn lsn_;
    std::unordered_set<PageId> pids;
    size_t max_pids;
    size_t replaced_pids;
    Lsn latest_replacement_lsn;

    InactiveSegment(Lsn lsnv, std::unordered_set<PageId>&& p, size_t maxp, size_t rep, Lsn latest)
        : lsn_(lsnv), pids(std::move(p)), max_pids(maxp), replaced_pids(rep), latest_replacement_lsn(latest) {}

    bool is_inactive() const override { return true; }
    Lsn lsn() const override { return lsn_; }
    SegmentState seg_state() const override { return SegInactive; }


    void remove_pid(PageId pid, Lsn replacement_lsn) override {
        assert(lsn_ <= replacement_lsn);
        if (replacement_lsn != lsn_) {
            auto it = pids.find(pid);
            if (it != pids.end()) {
                pids.erase(it);
                replaced_pids += 1;
            }
        }
        if (replacement_lsn > latest_replacement_lsn) {
            latest_replacement_lsn = replacement_lsn;
        }
    }

    void remove_heap_item(HeapId heap_id, const RunningConfig& config) override {
        config.heap.free(heap_id);
    }

    std::pair<std::unique_ptr<Segment>, std::unordered_set<PageId>>
    inactive_to_draining(Lsn to_lsn) override {
        assert(to_lsn >= lsn_);
        std::unordered_set<PageId> pids_moved = std::move(pids);
        auto draining = std::make_unique<DrainingSegment>(
            lsn_,
            max_pids,
            replaced_pids,
            latest_replacement_lsn
        );
        return {std::move(draining), std::move(pids_moved)};
    }
};



// ---- Draining 状态 ----
class DrainingSegment : public Segment {
public:
    Lsn lsn_;
    size_t max_pids;
    size_t replaced_pids;
    Lsn latest_replacement_lsn;

    DrainingSegment(Lsn lsnv, size_t maxp, size_t rep, Lsn latest)
        : lsn_(lsnv), max_pids(maxp), replaced_pids(rep), latest_replacement_lsn(latest) {}

    bool is_draining() const override { return true; }
    Lsn lsn() const override { return lsn_; }
    SegmentState seg_state() const override { return SegDraining; }


    void remove_pid(PageId /*pid*/, Lsn replacement_lsn) override {
        assert(lsn_ <= replacement_lsn);
        if (replacement_lsn != lsn_) {
            replaced_pids += 1;
        }
        if (replacement_lsn > latest_replacement_lsn) {
            latest_replacement_lsn = replacement_lsn;
        }
    }

    void remove_heap_item(HeapId heap_id, const RunningConfig& config) override {
        config.heap.free(heap_id);
    }

    std::pair<std::unique_ptr<Segment>, Lsn>
    draining_to_free(Lsn to_lsn) override {
        assert(to_lsn >= lsn_);
        Lsn replacement_lsn = latest_replacement_lsn;
        auto free = std::make_unique<FreeSegment>(lsn_);
        return {std::move(free), replacement_lsn};
    }

    bool can_free() const override {
        return replaced_pids == max_pids;
    }
};

// ---- 工厂函数 ----
inline std::unique_ptr<Segment> make_free_segment() {
    return std::make_unique<FreeSegment>();
}


// 状态转换函数
std::unique_ptr<Segment> FreeSegment::free_to_active(Lsn new_lsn) {
    assert(!previous_lsn.has_value() || new_lsn > previous_lsn.value());
    return std::make_unique<ActiveSegment>(new_lsn);
}



struct SegmentCleanerInner {
  std::mutex mutex_;
  std::map<LogOffset, std::set<PageId>> inner_;
};

using SegmentCleaner = std::shared_ptr<SegmentCleanerInner>;


