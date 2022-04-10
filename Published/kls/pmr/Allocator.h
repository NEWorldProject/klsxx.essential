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

#include "Resource.h"

namespace kls::pmr {
    template<class T>
    class PolymorphicAllocator {
    public:
        template<class>
        friend class PolymorphicAllocator;

        using value_type = T;

        PolymorphicAllocator() noexcept = default;
        PolymorphicAllocator(const PolymorphicAllocator &) = default;
        PolymorphicAllocator &operator=(const PolymorphicAllocator &) = default;
        PolymorphicAllocator(PolymorphicAllocator &&)  noexcept = default;
        PolymorphicAllocator &operator=(PolymorphicAllocator &&)  noexcept = default;

        PolymorphicAllocator(MemoryResource *const resource) noexcept: mResource{resource} {} // NOLINT

        template<class U>
        PolymorphicAllocator(const PolymorphicAllocator<U> &o) noexcept: mResource{o.mResource} {} // NOLINT

        [[nodiscard]] KLS_ALLOCATE T *allocate(const size_t count) {
            // get space for count objects of type T from _Resource
            void *const v = mResource->allocate(count * sizeof(T), alignof(T));
            return static_cast<T *>(v);
        }

        void deallocate(T *const ptr, const size_t count) noexcept {
            // return space for count objects of type T to _Resource
            // No need to verify that size_t can represent the size of T[count].
            mResource->deallocate(ptr, count * sizeof(T), alignof(T));
        }

        [[nodiscard]] KLS_ALLOCATE void *allocate_bytes(
                const size_t bytes, const size_t align = alignof(max_align_t)) {
            return mResource->allocate(bytes, align);
        }

        void deallocate_bytes(void *const ptr, const size_t bytes, const size_t align = alignof(max_align_t)) noexcept {
            mResource->deallocate(ptr, bytes, align);
        }

        template<class U>
        [[nodiscard]] KLS_ALLOCATE U *allocate_object(const size_t count = 1) {
            void *const v = allocate_bytes(count * sizeof(T), alignof(U));
            return static_cast<U *>(v);
        }

        template<class U>
        void deallocate_object(U *const ptr, const size_t count = 1) noexcept {
            deallocate_bytes(ptr, count * sizeof(U), alignof(U));
        }

        template<class U, class... Types>
        [[nodiscard]] KLS_ALLOCATE U *new_object(Types &&... args) {
            U *const ptr = allocate_object<U>();
            try {
                construct(ptr, std::forward<Types>(args)...);
            }
            catch (...) {
                deallocate_object(ptr);
                throw;
            }
            return ptr;
        }

        template<class U>
        void delete_object(U *const ptr) noexcept {
            std::destroy_at(*ptr);
            deallocate_object(ptr);
        }

        template<class U, class... Ts>
        void construct(U *const ptr, Ts &&... args) {
            // propagate allocator *this if uses_allocator_v<_Uty, PolymorphicAllocator>
            std::uninitialized_construct_using_allocator(ptr, *this, std::forward<Ts>(args)...);
        }

        [[nodiscard]] MemoryResource *resource() const noexcept { return mResource; }
    private:
        MemoryResource *mResource = default_resource();
    };

    template<class T1, class T2>
    [[nodiscard]] bool operator==(const PolymorphicAllocator<T1> &l, const PolymorphicAllocator<T2> &r) noexcept {
        // PolymorphicAllocators with the same resource are compatible
        return *l.resource() == *r.resource();
    }
}