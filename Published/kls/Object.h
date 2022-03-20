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
#include <utility>
#include <type_traits>

namespace kls {
	struct Object {};

    struct NonCopyable: Object {
        NonCopyable() = default;
        NonCopyable(const NonCopyable &) = delete;
        NonCopyable &operator=(const NonCopyable &) = delete;
    };

    struct NonMovable: Object {
        NonMovable() = default;
        NonMovable(NonMovable &&) = delete;
        NonMovable &operator=(NonMovable &&) = delete;
    };

    struct AddressSensitive: Object {
        AddressSensitive() = default;
        AddressSensitive(AddressSensitive &&) = delete;
        AddressSensitive(const AddressSensitive &) = delete;
        AddressSensitive &operator=(AddressSensitive &&) = delete;
        AddressSensitive &operator=(const AddressSensitive &) = delete;
    };

	struct PmrBase : Object { virtual ~PmrBase() noexcept = default; };

    template <class T>
    union Storage {
        T value;
        Storage() noexcept {} //NOLINT
        ~Storage() {} //NOLINT
    };

    template <class T>
    T& delegate_to_move_construct(T* ths, T&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
        return *(std::destroy_at(ths), std::construct_at(ths, std::forward<T>(other)));
    }
}
