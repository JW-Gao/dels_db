#include <memory>
#include <utility>
#include <optional>


// 垃圾回收怎么处理呢
template <typename K, typename V>
class PersistentMap {
    struct Node {
        const K key;
        const V value;
        const std::shared_ptr<const Node> left;
        const std::shared_ptr<const Node> right;
        const int height;

        Node(K k, V v, std::shared_ptr<const Node> l, std::shared_ptr<const Node> r, int h)
            : key(std::move(k)), value(std::move(v)), left(std::move(l)),
              right(std::move(r)), height(h) {}
    };

    std::shared_ptr<const Node> root;

    // 私有构造函数
    explicit PersistentMap(std::shared_ptr<const Node> root) : root(std::move(root)) {}

    static int balanceFactor(const std::shared_ptr<const Node>& n) {
        return (n->left ? n->left->height : 0) - (n->right ? n->right->height : 0);
    }

    static std::shared_ptr<const Node> rotateLeft(std::shared_ptr<const Node> n) {
        auto newRoot = n->right;
        auto newRight = newRoot->left;
        return std::make_shared<const Node>(
            newRoot->key, newRoot->value,
            std::make_shared<const Node>(n->key, n->value, n->left, newRight, n->height),
            newRoot->right,
            std::max(n->height, newRight ? newRight->height + 1 : 0) + 1
        );
    }

    // 其他平衡操作类似...

public:
    PersistentMap() = default;

    bool contains(const K& key) const {
        auto node = root;
        while (node) {
            if (key < node->key) node = node->left;
            else if (node->key < key) node = node->right;
            else return true;
        }
        return false;
    }

    std::optional<V> find(const K& key) const {
        auto node = root;
        while (node) {
            if (key < node->key) node = node->left;
            else if (node->key < key) node = node->right;
            else return node->value;
        }
        return std::nullopt;
    }

    PersistentMap insert(K key, V value) const {
        auto newRoot = insert(root, std::move(key), std::move(value));
        return PersistentMap(newRoot);
    }

private:
    static std::shared_ptr<const Node> insert(
        const std::shared_ptr<const Node>& node, K key, V value) 
    {
        if (!node) {
            return std::make_shared<const Node>(
                std::move(key), std::move(value), nullptr, nullptr, 1);
        }

        if (key < node->key) {
            auto newLeft = insert(node->left, std::move(key), std::move(value));
            return balance(std::make_shared<const Node>(
                node->key, node->value, newLeft, node->right, node->height));
        }
        
        if (node->key < key) {
            auto newRight = insert(node->right, std::move(key), std::move(value));
            return balance(std::make_shared<const Node>(
                node->key, node->value, node->left, newRight, node->height));
        }

        // 已存在则更新值
        return std::make_shared<const Node>(
            node->key, std::move(value), node->left, node->right, node->height);
    }

    // AVL 平衡实现
    static std::shared_ptr<const Node> balance(std::shared_ptr<const Node> node) {
        int bf = balanceFactor(node);
        if (bf > 1) {
            if (balanceFactor(node->left) < 0) {
                node = std::make_shared<const Node>(
                    node->key, node->value, rotateLeft(node->left), node->right, node->height);
            }
            return rotateRight(node);
        }
        if (bf < -1) {
            if (balanceFactor(node->right) > 0) {
                node = std::make_shared<const Node>(
                    node->key, node->value, node->left, rotateRight(node->right), node->height);
            }
            return rotateLeft(node);
        }
        return node;
    }
};

// test