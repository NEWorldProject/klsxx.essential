/*
* Copyright (c) 2022 DWVoid and Infinideastudio Team
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include <cstddef>

namespace kls::essential {
    class MemoryAVL {
        struct Node {
            Node* left;
            Node* right;
            Node* parent;
            intptr_t height;

            [[nodiscard]] uintptr_t key() const noexcept { return reinterpret_cast<uintptr_t>(this); }

            [[nodiscard]] uintptr_t value() const noexcept { return key(); }

            [[nodiscard]] intptr_t left_height() const noexcept { return (left ? left->height : 0u); }

            [[nodiscard]] intptr_t right_height() const noexcept { return (right ? right->height : 0u); }

            [[nodiscard]] auto heights() const noexcept { return std::pair(left_height(), right_height()); }

            auto& select(const bool is_left) noexcept { if (is_left) return left; else return right; }

            void set_left(Node* const node) noexcept { if ((left = node)) node->parent = this; }

            void set_right(Node* const node) noexcept { if ((right = node)) node->parent = this; }

            void replace(Node* const find, Node* const value) noexcept {
                if (left == find) set_left(value); else if (right == find) set_right(value);
            }

            bool fix_height() noexcept {
                if (const auto new_height = std::max(left_height(), right_height()) + 1; height != new_height)
                    return (height = new_height, true);
                else return false;
            }

            auto reset(Node* const np) noexcept { return (left = right = nullptr, parent = np, height = 1, this); }
        };
    public:
        void push(uintptr_t location) noexcept { add(reinterpret_cast<Node*>(location)); }
        
        [[nodiscard]] bool pop_back_if(uintptr_t location) noexcept {
            if (max == nullptr) return false;
            if (max->value() != location) return false;
            delete_leaf(max);
            return true;
        }

        [[nodiscard]] uintptr_t pop_front() noexcept {
            if (min == nullptr) return 0ull;
            const auto ret = min->value();
            delete_leaf(min);
            return ret;
        }

    private:
        Node* root = nullptr;
        Node* min = nullptr;
        Node* max = nullptr;

        void add(Node* const node) noexcept {
            if (!root) return (min = max = root = node->reset(nullptr), void());
            const auto n_key = node->key();
            for (auto current = root;;) {
                const auto a_key = current->key();
                const auto target_left = n_key < a_key;
                if (auto& target = current->select(target_left); !target) {
                    target = node->reset(current);
                    if (target_left && current == min) min = node;
                    if (!target_left && current == max) max = node;
                    node_fix_up(current);
                    break;
                }
                else current = target;
            }
        }

        bool single_rotate_left(Node* const n_left) noexcept {
            const auto parent = n_left->parent;
            const auto n_root = n_left->right;
            const auto shifted = n_root->left;
            n_root->set_left(n_left);
            n_left->set_right(shifted);
            if (parent) parent->replace(n_left, n_root); else (root = n_root, n_root->parent = nullptr);
            n_left->fix_height();
            return n_root->fix_height();
        }

        bool single_rotate_right(Node* const n_right) noexcept {
            const auto parent = n_right->parent;
            const auto n_root = n_right->left;
            const auto shifted = n_root->right;
            n_root->set_right(n_right);
            n_right->set_left(shifted);
            if (parent) parent->replace(n_right, n_root); else (root = n_root, n_root->parent = nullptr);
            n_right->fix_height();
            return n_root->fix_height();
        }

        bool double_rotate_left_right(Node* const avl) noexcept {
            single_rotate_left(avl->left);
            return single_rotate_right(avl);
        }

        bool double_rotate_right_left(Node* const avl) noexcept {
            single_rotate_right(avl->right);
            return single_rotate_left(avl);
        }

        bool try_balance(Node* const avl) noexcept {
            const auto [lh, rh] = avl->heights();
            if (lh - rh >= 2) {
                const auto [llh, lrh] = avl->left->heights();
                if (llh > lrh) return single_rotate_right(avl); else return double_rotate_left_right(avl);
            }
            else if (rh - lh >= 2) {
                const auto [rlh, rrh] = avl->right->heights();
                if (rrh > rlh) return single_rotate_left(avl); else return double_rotate_right_left(avl);
            }
            else return avl->fix_height();
        }

        void node_fix_up(Node* node) {
            while (node) {
                const auto parent = node->parent;
                if (!try_balance(node)) return;
                node = parent;
            }
        }

        void delete_leaf(Node* const node) noexcept {
            const auto parent = node->parent;
            // update min-max tags
            if (node == min) { if (node->right) min = node->right; else min = parent; }
            if (node == max) { if (node->left) max = node->left; else max = parent; }
            // remove the node from tree
            const auto child = (node->left ? node->left : node->right);
            if (parent) {
                parent->replace(node, child);
                node_fix_up(parent);
            }
            else if ((root = child)) child->parent = nullptr;
        }
    };
}
