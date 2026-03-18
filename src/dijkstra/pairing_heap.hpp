#pragma once

#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename Key, typename Value, typename Compare = std::less<Key>>
class PairingHeap {
public:
    struct Node {
        Key key;
        Value value;
        Node* child;
        Node* sibling;
        Node* parent;

        Node(const Key& k, const Value& v)
            : key(k), value(v), child(nullptr), sibling(nullptr), parent(nullptr) {}
    };

    using Handle = Node*;

    PairingHeap() : root_(nullptr), size_(0), comp_(Compare()) {}

    ~PairingHeap() {
        destroy(root_);
    }

    bool empty() const {
        return root_ == nullptr;
    }

    std::size_t size() const {
        return size_;
    }

    std::pair<Key, Value> top() const {
        if (!root_) {
            throw std::runtime_error("PairingHeap is empty");
        }
        return {root_->key, root_->value};
    }

    Handle push(const Key& key, const Value& value) {
        Node* node = new Node(key, value);
        root_ = meld(root_, node);
        ++size_;
        return node;
    }

    void pop() {
        if (!root_) {
            throw std::runtime_error("PairingHeap is empty");
        }

        Node* old_root = root_;

        if (!root_->child) {
            root_ = nullptr;
        } else {
            root_ = two_pass_meld(root_->child);
            if (root_) {
                root_->parent = nullptr;
            }
        }

        delete old_root;
        --size_;
    }

    void decrease_key(Handle node, const Key& new_key) {
        if (!node) {
            throw std::runtime_error("Invalid handle in decrease_key");
        }

        if (!comp_(new_key, node->key) && !equal_keys(new_key, node->key)) {
            throw std::runtime_error("New key is not smaller than current key");
        }

        node->key = new_key;

        if (node == root_) {
            return;
        }

        if (node->parent && comp_(node->key, node->parent->key)) {
            cut(node);
            root_ = meld(root_, node);
        }
    }

private:
    Node* root_;
    std::size_t size_;
    Compare comp_;

    bool equal_keys(const Key& a, const Key& b) const {
        return !comp_(a, b) && !comp_(b, a);
    }

    void destroy(Node* node) {
        if (!node) {
            return;
        }

        Node* child = node->child;
        while (child) {
            Node* next = child->sibling;
            destroy(child);
            child = next;
        }

        delete node;
    }

    Node* meld(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;

        if (comp_(b->key, a->key)) {
            std::swap(a, b);
        }

        b->parent = a;
        b->sibling = a->child;
        a->child = b;

        return a;
    }

    void cut(Node* node) {
        Node* parent = node->parent;
        if (!parent) {
            return;
        }

        if (parent->child == node) {
            parent->child = node->sibling;
        } else {
            Node* prev = parent->child;
            while (prev && prev->sibling != node) {
                prev = prev->sibling;
            }
            if (prev) {
                prev->sibling = node->sibling;
            }
        }

        node->parent = nullptr;
        node->sibling = nullptr;
    }

    Node* two_pass_meld(Node* first_sibling) {
        if (!first_sibling) {
            return nullptr;
        }

        std::vector<Node*> trees;
        Node* curr = first_sibling;

        while (curr) {
            Node* a = curr;
            Node* b = curr->sibling;

            curr = nullptr;
            if (b) {
                curr = b->sibling;
            }

            a->parent = nullptr;
            a->sibling = nullptr;

            if (b) {
                b->parent = nullptr;
                b->sibling = nullptr;
                trees.push_back(meld(a, b));
            } else {
                trees.push_back(a);
            }
        }

        Node* result = trees.back();
        for (int i = static_cast<int>(trees.size()) - 2; i >= 0; --i) {
            result = meld(trees[i], result);
        }

        return result;
    }
};