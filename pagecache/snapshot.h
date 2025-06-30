#pragma once
#include <vector>

#include "disk_pointer.h"
#include "def_types.h"


struct CacheInfoWithoutTs {
  Lsn lsn;
  DiskPtr disk_ptr;
};

enum PageStateType {
  Present, // 页面存在
  Free, // 页面空闲
  Uninitialized, // 页面未初始化
};

class PageState {
public:
  PageStateType type_;
  CacheInfoWithoutTs base_; // 基准页信息
  std::vector<CacheInfoWithoutTs> frags_; // 同一page的多个fragment 信息, 如果是页面状态是free的，那么只有base_是有效的,frags_是空的
public:

};