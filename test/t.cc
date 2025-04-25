#include <atomic>
#include <iostream>

using namespace std;

struct Node {
  void *p;
  int v;
};

atomic<Node> an;
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

template<typename T>
class Shared : public Pointable<T> {
public:

}; // class Shared
int main() {
  // an.store(Node{nullptr, 0}, memory_order_release);
  // if (an.)
  uintptr_t sz = Shared<Node>::ALIGN;
  uintptr_t isz = Shared<int>::ALIGN;
  cout << sz << endl;
  cout << isz << endl;
  return 0;
}