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

#include <new>
#include "kls/pmr/Resource.h"

namespace kls::pmr {
    MemoryResource *default_resource() noexcept {
        struct Resource : MemoryResource {
            Resource() noexcept: MemoryResource(
                    reinterpret_cast<FnAllocate>(&Resource::allocate_self),
                    reinterpret_cast<FnDeallocate>(&Resource::deallocate_self)
            ) {}

            void *allocate_self(size_t bytes, size_t alignment) { // NOLINT
                if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
                    return ::operator new (bytes, std::align_val_t{alignment});
                return ::operator new(bytes);
            }

            void deallocate_self(void *p, size_t bytes, size_t alignment) { // NOLINT
                if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
                    return ::operator delete (p, bytes, std::align_val_t{alignment});
                ::operator delete(p, bytes);
            }
        };
        static Resource resource{};
        return &resource;
    }
}