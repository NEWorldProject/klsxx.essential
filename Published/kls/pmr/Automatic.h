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

#include "Allocator.h"
#include "kls/essential/Memory.h"

namespace kls::pmr {
    namespace detail {
        template<class T>
        inline auto get_alloc(MemoryResource *mem) noexcept { return PolymorphicAllocator<T>(mem); }

        template<class T>
        struct destroy_one {
            PolymorphicAllocator<T> allocator{};
            destroy_one() = default;
            explicit destroy_one(MemoryResource *res) noexcept: allocator(res) {}
            void operator()(T *ptr) noexcept { essential::allocator_delete(allocator, ptr); }
        };

        template<class T>
        struct destroy_many {
            using Elem = std::remove_extent_t<T>;
            size_t size{};
            PolymorphicAllocator<Elem> allocator{};
            destroy_many() = default;
            explicit destroy_many(size_t size, MemoryResource *res) noexcept: size(size), allocator(res) {}
            void operator()(Elem *ptr) noexcept {
                for (size_t i = 0; i < size; ++i) std::destroy_at(ptr + i);
                allocator.deallocate(ptr, size);
            }
        };
    }

    template<class T>
    using unique_ptr = std::unique_ptr<T,
            std::conditional_t<!std::is_array_v<T>, detail::destroy_one<T>, detail::destroy_many<T>>
    >;

    template<class T, class... Ts, std::enable_if_t<!std::is_array_v<T>, int> = 0>
    unique_ptr<T> make_unique(MemoryResource *resource, Ts &&... args) {
        auto alloc = detail::get_alloc<T>(resource);
        return unique_ptr<T>(
                essential::allocator_new<T>(alloc, std::forward<Ts>(args)...),
                detail::destroy_one<T>{resource}
        );
    }

    template<class T, std::enable_if_t<std::is_array_v<T> && std::extent_v<T> == 0, int> = 0>
    unique_ptr<T> make_unique(MemoryResource *resource, const size_t size) {
        using Elem = std::remove_extent_t<T>;
        const auto mem = detail::get_alloc<Elem>(resource).allocate(size);
        for (size_t i = 0; i < size; ++i) std::construct_at(mem + i);
        return unique_ptr<T>(mem, detail::destroy_many<T>{size, resource});
    }

    template<class T, class... Ts, std::enable_if_t<std::extent_v<T> != 0, int> = 0>
    void make_unique(Ts &&...) = delete;
}
