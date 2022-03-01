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

#include <cstdint>
#include "kls/essential/Memory.h"

namespace kls::temp {
    namespace detail {
        void free(void* mem) noexcept;

        [[nodiscard]] void* allocate(uintptr_t size) noexcept;

        constexpr uintptr_t temp_max_span = 1u << 18u;

        static constexpr uintptr_t block_size = 4u << 20u; // 4MiB
    }

    template<class T>
    struct allocator {
    private:
        static constexpr uintptr_t align() noexcept {
            const auto trim = sizeof(T) / alignof(T) * alignof(T);
            return trim != sizeof(T) ? trim + alignof(T) : sizeof(T);
        }

        // Helper Const
        static constexpr uintptr_t alignment = alignof(T);
        static constexpr uintptr_t aligned_size = align();
        inline static std::allocator<T> default_alloc{};
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using is_always_equal = std::true_type;
        using propagate_on_container_copy_assignment = std::false_type;
        using propagate_on_container_move_assignment = std::false_type;
        using propagate_on_container_swap = std::false_type;

        constexpr allocator() noexcept = default;

        allocator(allocator&&) noexcept = default;

        allocator& operator=(allocator&&) noexcept = default;

        allocator(const allocator&) noexcept = default;

        allocator& operator=(const allocator&) noexcept = default;

        template<class U>
        explicit constexpr allocator(const allocator<U>&) noexcept {}

        T* address(T& x) const noexcept { return std::addressof(x); }

        const T* address(const T& x) const noexcept { return std::addressof(x); }

        [[nodiscard]] T* allocate(const std::size_t n) {
            if constexpr (alignment <= alignof(std::max_align_t)) {
                const auto size = n > 1 ? aligned_size * n : sizeof(T);
                if (size <= detail::temp_max_span) { return reinterpret_cast<T*>(detail::allocate(size)); }
            }
            return default_alloc.allocate(n);
        }

        void deallocate(T* p, const std::size_t n) noexcept {
            if constexpr (alignment <= alignof(std::max_align_t)) {
                if ((n > 1 ? aligned_size * n : sizeof(T)) <= detail::temp_max_span) {
                    return detail::free(reinterpret_cast<void*>(const_cast<std::remove_cv_t<T> *>(p)));
                }
            }
            default_alloc.deallocate(p, n);
        }

        [[nodiscard]] bool operator==(const allocator& r) const noexcept { return true; }

        [[nodiscard]] bool operator!=(const allocator& r) const noexcept { return false; }
    };

    namespace detail {
        template<class T>
        inline auto get_alloc() noexcept { return allocator<T>(); }

        template<class T>
        struct destruct_a {
            void operator()(T* ptr) noexcept {
                auto alloc = detail::get_alloc<T>();
                essential::allocator_delete(alloc, ptr);
            }
        };

        template<class T>
        struct destruct_b {
            size_t size;

            using Elem = std::remove_extent_t<T>;

            constexpr destruct_b(size_t size) noexcept : size(size) {}

            void operator()(Elem* ptr) noexcept {
                for (size_t i = 0; i < size; ++i) std::destroy_at(ptr + i);
                detail::get_alloc<Elem>().deallocate(ptr, size);
            }
        };
    }

    template<class T>
    using unique_ptr = std::unique_ptr<T,
        std::conditional_t<
        !std::is_array_v<T>,
        detail::destruct_a<T>,
        detail::destruct_b<T>
        >
    >;

    template<class T, class... Ts, std::enable_if_t<!std::is_array_v<T>, int> = 0>
    unique_ptr<T> make_unique(Ts &&... args) {
        auto alloc = detail::get_alloc<T>();
        return unique_ptr<T>(essential::allocator_new<T>(alloc, std::forward<Ts>(args)...));
    }

    template<class T, std::enable_if_t<std::is_array_v<T>&& std::extent_v<T> == 0, int> = 0>
    unique_ptr<T> make_unique(const size_t size) {
        using Elem = std::remove_extent_t<T>;
        const auto mem = detail::get_alloc<Elem>().allocate(size);
        for (size_t i = 0; i < size; ++i) std::construct_at(mem + i);
        return unique_ptr<T>(mem, detail::destruct_b<T>{size});
    }

    template<class T, class... Ts, std::enable_if_t<std::extent_v<T> != 0, int> = 0>
    void make_unique(Ts &&...) = delete;
}
