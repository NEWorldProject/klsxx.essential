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

#include <atomic>
#include "kls/temp/Temp.h"

namespace {
    constexpr uintptr_t block_size = 4u << 20u; // 4MiB

    struct header final {
        std::atomic_int32_t flying{0};
    };

    header *fetch() noexcept {
        return std::construct_at(reinterpret_cast<header *>(kls::essential::rent_4m_block()));
    }

    void release(header *const blk) noexcept {
        kls::essential::return_4m_block(reinterpret_cast<uintptr_t>(blk));
    }

    [[nodiscard]] constexpr uintptr_t max_align(const uintptr_t size) noexcept {
        constexpr auto mask = alignof(std::max_align_t) - 1;
        constexpr auto rev = ~mask;
        if (size & mask) return (size & rev) + alignof(std::max_align_t); else return size;
    }

    class allocation final {
        static constexpr auto alloc_start = max_align(sizeof(header));
        header *current{};
        uintptr_t head{};
        int32_t count{};
    public:
        [[nodiscard]] bool flush(header *&out) const noexcept {
            if (current) {
                out = current;
                return (current->flying.fetch_add(count, std::memory_order_seq_cst) == -count);
            }
            return false;
        }

        void reset(header *const other) noexcept {
            current = other, head = alloc_start, count = 0;
        }

        [[nodiscard]] void *allocate(const uintptr_t size) noexcept {
            const auto aligned = max_align(size);
            if (const auto expected = head + aligned; expected < block_size) {
                ++count;
                const auto res = reinterpret_cast<uintptr_t>(current) + head;
                head = expected;
                return reinterpret_cast<void *>(res);
            }
            return nullptr;
        }
    };

    constexpr uintptr_t temp_max_span = 1u << 18u;

    [[nodiscard]] void *allocate_impl(const uintptr_t size) noexcept {
        struct local final {
            allocation alloc{};

            local() noexcept { reset(); }

            ~local() noexcept { reset(nullptr); }

            void reset(header *const next = fetch()) noexcept {
                if (header *last = nullptr; alloc.flush(last)) {
                    if (last) release(last);
                }
                alloc.reset(next);
            }
        };
        static const thread_local auto o = std::make_unique<local>();
        for (;;) if (const auto ret = o->alloc.allocate(size); ret) return ret; else o->reset();
    }

    void deallocate_impl(void *const mem) noexcept {
        static constexpr uintptr_t rev = 0b11'1111'1111'1111'1111'1111;
        static constexpr uintptr_t mask = ~rev;
        if (mem == nullptr) return;
        const auto base = reinterpret_cast<uintptr_t>(mem) & mask;
        const auto header = reinterpret_cast<struct header *>(base);
        if (header->flying.fetch_sub(1, std::memory_order_seq_cst) == 1) release(header);
    }
}

namespace kls::temp {
    pmr::MemoryResource *resource() noexcept {
        struct Resource : pmr::MemoryResource {
            Resource() noexcept: MemoryResource(
                    reinterpret_cast<FnAllocate>(&Resource::allocate_self),
                    reinterpret_cast<FnDeallocate>(&Resource::deallocate_self)
            ) {}

            void *allocate_self(size_t bytes, size_t alignment) { // NOLINT
                if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__ || bytes > temp_max_span)
                    return ::operator new(bytes, std::align_val_t{alignment});
                return allocate_impl(bytes);
            }

            void deallocate_self(void *p, size_t bytes, size_t alignment) { // NOLINT
                if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__ || bytes > temp_max_span)
                    return ::operator delete(p, bytes, std::align_val_t{alignment});
                return deallocate_impl(p);
            }
        };
        static Resource resource{};
        return &resource;
    }
}