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

#include <memory>
#include <cstddef>
#include "kls/Object.h"
#include "kls/Macros.h"

namespace kls::pmr {
    class MemoryResource : public PmrBase {
    public:
        using FnAllocate = void *(MemoryResource::*)(size_t bytes, size_t alignment);
        using FnDeallocate = void (MemoryResource::*)(void *p, size_t bytes, size_t alignment);
        using FnIsEqual = bool (MemoryResource::*)(const MemoryResource &other) const noexcept;

        [[nodiscard]] KLS_FORCE_INLINE void *allocate(size_t bytes, size_t align = alignof(std::max_align_t)) {
            return (*this.*mFnAllocate)(bytes, align);
        }

        KLS_FORCE_INLINE void deallocate(void *p, size_t bytes, size_t align = alignof(std::max_align_t)) {
            return (*this.*mFnDeallocate)(p, bytes, align);
        }

        [[nodiscard]] KLS_FORCE_INLINE bool is_equal(const MemoryResource &other) const noexcept {
            if (&other == this) return true;
            if (mFnIsEqual) return (*this.*mFnIsEqual)(other);
            return (typeid(other) == typeid(this));
        }
    protected:
        MemoryResource(FnAllocate alloc, FnDeallocate dealloc, FnIsEqual equal = nullptr) noexcept
                : mFnAllocate(alloc), mFnDeallocate(dealloc), mFnIsEqual(equal) {}
    private:
        FnAllocate mFnAllocate;
        FnDeallocate mFnDeallocate;
        FnIsEqual mFnIsEqual;
    };

    [[nodiscard]] inline bool operator==(const MemoryResource &l, const MemoryResource &r) noexcept {
        return l.is_equal(r);
    }

    MemoryResource *default_resource() noexcept;
}