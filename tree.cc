#include "tree.h"
#include "pagecache/pagecache.h"


struct View {
  NodeView node_view;
  PageId pid;
};