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
#include "kls/pmr/Automatic.h"

namespace kls::temp {
    pmr::MemoryResource *resource() noexcept;

    template<class T>
    struct allocator: pmr::PolymorphicAllocator<T> {
        allocator() noexcept: pmr::PolymorphicAllocator<T>(resource()) {}
        template<class U>
        allocator(const allocator<U> &o) noexcept: pmr::PolymorphicAllocator<T>(o.resource()) {} // NOLINT
    };

    template<class T, class... Ts, std::enable_if_t<!std::is_array_v<T>, int> = 0>
    decltype(auto) make_unique(Ts &&... args) {
        return pmr::make_unique<T>(resource(), std::forward<Ts>(args)...);
    }

    template<class T, std::enable_if_t<std::is_array_v<T>&& std::extent_v<T> == 0, int> = 0>
    decltype(auto) make_unique(const size_t size) {
        return pmr::make_unique<T>(resource(), size);
    }

    template<class T, class... Ts, std::enable_if_t<std::extent_v<T> != 0, int> = 0>
    void make_unique(Ts &&...) = delete;
}
