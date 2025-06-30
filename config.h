#pragma once
#include "pagecache/def_types.h"
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

enum Mode {
  LowSpace,
  HighThroughput,
};

enum Error {

};
struct Inner {
   size_t cache_capacity;
  
   uint64_t flush_every_ms;
  
   size_t segment_size;
  
   std::string path;
  
   bool create_new;
   Mode mode;
   bool temporary;
   bool use_compression;
  
   uint32_t compression_factor;
  
   uint64_t idgen_persist_interval;
  
   uint64_t snapshot_after_ops;
  
  std::pair<int,int> version; // for mvcc ?? 
  std::string tmp_path;
  // (crate) global_error: Arc<Atomic<Error>>,
  std::shared_ptr<std::atomic<Error>> global_error;

  Inner(const Inner &another) {
    
  }
};

class Config {
  std::shared_ptr<Inner> inner_;

  bool validate() const {
    return inner_
  }
public:
  Config(const char *path) {
    inner_ = std::make_shared<Inner>();
    inner_->path = std::string(path);
  }

  Db *open() {

  }
};

class RunningConfig {

};