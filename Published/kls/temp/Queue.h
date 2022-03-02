#pragma once

#include <cstdint>
#include <atomic>
#include "Temp.h"

namespace kls::temp {
    template<class T>
    class Queue {
        struct Node;

        static constexpr uintptr_t Items = (4096 - sizeof(Node*)) / sizeof(T);

        struct Node {
            constexpr Node() noexcept : Next(nullptr), Data() {}

            Node* Next;
            T Data[Items];
        };

        inline static allocator<Node> Alloc;
    public:
        void Push(const T& data) noexcept {
            if (!mBeg) {
                mBeg.Off = mEnd.Off = 0;
                const auto n = essential::allocator_new<Node>(Alloc);
                mBeg.Blk.store(n);
                mEnd.Blk.store(n);
                n->Data[mEnd.Off++] = data;
            }
            else {
                const auto endBlk = mEnd.Blk.load();
                endBlk->Data[mEnd.Off++] = data;
                if (mEnd.Off == Items) {
                    mEnd.Off = 0;
                    const auto n = essential::allocator_new<Node>(Alloc);
                    endBlk->Next = n;
                    mEnd.Blk.store(n);
                }
            }
        }

        T Pop() noexcept {
            if (mBeg) {
                const auto curBlk = mBeg.Blk.load();
                const auto ret = curBlk->Data[mBeg.Off++];
                if (mBeg != mEnd) {
                    if (mBeg.Off == Items) {
                        mBeg.Off = 0;
                        mBeg.Blk.store(curBlk->Next);
                        essential::allocator_delete(Alloc, curBlk);
                    }
                }
                if (mBeg == mEnd) {
                    if (mBeg.Off == mEnd.Off) {
                        mBeg.Blk.store(nullptr);
                        mEnd.Blk.store(nullptr);
                        essential::allocator_delete(Alloc, curBlk);
                    }
                }
                return ret;
            }
            return T{};
        }

        [[nodiscard]] bool Empty() const noexcept { return !mBeg.Blk; }

    private:
        struct Iterator {
            constexpr Iterator() noexcept : Blk(nullptr), Off(0) {}

            explicit operator bool() const noexcept { return bool(Blk.load()); }

            bool operator!=(const Iterator& r) const noexcept { return (Blk != r.Blk) || (Off != r.Off); }

            bool operator==(const Iterator& r) const noexcept { return (Blk == r.Blk) && (Off == r.Off); }

            std::atomic<Node*> Blk;
            uintptr_t Off;
        };

        Iterator mBeg{}, mEnd{};
    };
}
