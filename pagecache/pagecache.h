#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "def_types.h"
#include "disk_pointer.h"



struct CacheInfo {
  uint64_t ts;  // 时间戳，用于保证日志的线性关系
  Lsn lsn;
  DiskPtr pointer; // 指向日志中的位置，是对应
};

struct Page {
  int page_id;
  
  std::vector<CacheInfo> cache_info;
};

struct Config {

};

struct NOde {
  OrdMap map;

  unsigned char *info;
};


class PageCache {
  Config *conf_;
  PageCacheInner *inner;
public:
  Page allocate_page(const Node &node);

  Page link(int page_id, const Page &page, Node delta);
};
