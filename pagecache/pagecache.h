#pragma once
#include <vector>

namespace PageCache {
  struct Page {
    int page_id;
    
    std::vector<CacheInfo> cache_info;
  }

  struct Config {

  };

  struct NOde {
    OrdMap map;

    unsigned char *info;
  }


  class PageCache {
    Config *conf_;
    PageCacheInner *inner;
  public:
    Page allocate_page(const Node &node);

    Page link(int page_id, const Page &page, Node delta);
  };
}