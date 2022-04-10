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
#include <cstring>
#include "kls/Span.h"

namespace kls::essential {
    template<class T, class Alloc, class... Types>
    T* allocator_construct(Alloc& alloc, T* ptr, Types&&... args) {
        using traits = std::allocator_traits<Alloc>;
        traits::construct(alloc, ptr, std::forward<Types>(args)...);
        return ptr;
    }

    template<class T, class Alloc>
    T* allocator_destruct(Alloc& alloc, T* ptr) {
        using traits = std::allocator_traits<Alloc>;
        traits::destroy(alloc, ptr);
        return ptr;
    }

    template<class T, class Alloc, class... Types>
    T* allocator_new(Alloc& alloc, Types&&... args) {
        using traits = std::allocator_traits<Alloc>;
        return allocator_construct(alloc, traits::allocate(alloc, 1), std::forward<Types>(args)...);
    }

    template<class T, class Alloc>
    void allocator_delete(Alloc& alloc, T* ptr) {
        using traits = std::allocator_traits<Alloc>;
        traits::deallocate(alloc, allocator_destruct(alloc, ptr), 1);
    }

    /// <summary>
    /// Obtains a 4MiB memory block that is alligned to 4MiB from the library's memory management system
    /// </summary>
    /// <returns> The staring address of the block in uintptr_t </returns>
    uintptr_t rent_4m_block() noexcept;

    /// <summary>
    /// Return the 4MiB memory block obtained form calling rent_4m_block() back to the library's memory management system
    /// </summary>
    /// <param name="block"> The exact value returned from the relavent rent_4m_block() </param>
    /// <returns> None </returns>
    void return_4m_block(uintptr_t block) noexcept;
}
