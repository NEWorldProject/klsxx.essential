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

#include <ranges>
#include <memory>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <type_traits>

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
    /// Return the $MiB memory block obtained form calling rent_4m_block() back to the library's memory management system
    /// </summary>
    /// <param name="block"> The exact value returned from the relavent rent_4m_block() </param>
    /// <returns> None </returns>
    void return_4m_block(uintptr_t block) noexcept;

    /// <summary>
    /// Represents a non-owning contiguous span of memory that is capable of holding 'size' entries of 'T' typed data
    /// with the exception of T = void
    /// 
    /// This data-type is implicitly convertable to Span<> to allow for a cleaner type erasure
    /// 
    /// This data-type implements the std::ranges::contiguous_range concept and provides size() for information
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template <class T = void>
    class Span {
    public:
        template <class Range> requires std::ranges::contiguous_range<Range>
        constexpr Span(Range& range) noexcept: m_begin(std::ranges::data(range)), m_size(std::ranges::size(range)) {}
        template <class U> requires std::integral<U>
        constexpr Span(T* data, U size) noexcept : m_begin{ data }, m_size(size) {}
        template <class U> requires std::integral<U>
        constexpr Span(const T* data, U size) noexcept : m_begin{ const_cast<T*>(data) }, m_size(size) {}
        constexpr Span(Span&&) noexcept = default;
        constexpr Span(const Span&) noexcept = default;
        constexpr Span& operator=(Span&&) noexcept = default;
        constexpr Span& operator=(const Span&) noexcept = default;
        constexpr T* begin() noexcept { return m_begin; }
        constexpr T* end() noexcept { return m_begin + m_size; }
        constexpr const T* begin() const noexcept { return m_begin; }
        constexpr const T* end() const noexcept { return m_begin + m_size; }
        constexpr T* data() noexcept { return m_begin; }
        constexpr const T* data() const noexcept { return m_begin; }
        constexpr size_t size() const noexcept { return m_size; }
        constexpr operator Span<>() const noexcept;
    private:
        T* m_begin;
        size_t m_size;
    };

    /// <summary>
    /// Represents a non-owning contiguous span of memory consists of 'size' basic machine addressable units
    /// 
    /// This data-type implements data() and provides size() for information
    /// 
    /// This data-type does not implement the range concept. Use *_span_cast to bring it to a Span<T> to access
    /// </summary>
    template<>
    class Span<void> {
    public:
        template <class U> requires std::integral<U>
        constexpr Span(void* data, U size) noexcept : m_begin{ data }, m_size(size) {}
        template <class U> requires std::integral<U>
        constexpr Span(const void* data, U size) noexcept : m_begin{ const_cast<void*>(data) }, m_size(size) {}
        constexpr Span(Span&&) noexcept = default;
        constexpr Span(const Span&) noexcept = default;
        constexpr Span& operator=(Span&&) noexcept = default;
        constexpr Span& operator=(const Span&) noexcept = default;
        constexpr void* data() noexcept { return m_begin; }
        constexpr const void* data() const noexcept { return m_begin; }
        size_t size() const noexcept { return m_size; }
    private:
        void* m_begin;
        size_t m_size;
    };

    template<class T>
    inline constexpr Span<T>::operator Span<>() const noexcept { return Span(m_begin, m_size * sizeof(T)); }

    /// <summary>
    /// The function performs a static_cast from Span<> regardless of the data alignment
    /// </summary>
    /// <typeparam name="T"> The type to cast to </typeparam>
    /// <param name="o"> The Span object to cast </param>
    /// <returns> The casted span with size being the maximum amount of elements able to fit into o </returns>
    template<class T>
    inline constexpr Span<T> static_span_cast(Span<> o) noexcept {
        return Span<T>(static_cast<T *>(o.data()), o.size() / sizeof(T));
    }

    /// <summary>
    /// The function performs a reinterpret cast from Span<U> regardless of the data alignment 
    /// if U and T has the same size and is trivially copiable
    /// </summary>
    /// <typeparam name="T"> The type to cast to </typeparam>
    /// <typeparam name="T"> The type to cast from </typeparam>
    /// <param name="o"> The Span object to cast </param>
    /// <returns> The casted span </returns>
    template<class T, class U>
    requires requires {
        sizeof(T) == sizeof(U);
        std::is_trivially_copyable_v<T>;
        std::is_trivially_copyable_v<U>;
    }
    inline constexpr Span<T> reinterpret_span_cast(Span<U> o) noexcept {
        return Span<T>(reinterpret_cast<T *>(o.data()), o.size());
    }

    /// <summary>
    /// Copy the source span's items to the destination span using std::copy if both spans have the same size
    /// The function assumes all entries of the both given span contains valid elements
    /// </summary>
    /// <param name="src"> The source span </param>
    /// <param name="dest"> The destination span </param>
    /// <returns> If both spans have same size returns true, otherwise false</returns>
    template<class T>
    constexpr bool copy(Span<T> src, Span<T> dest) noexcept(std::is_nothrow_copy_assignable_v<T> &&
                                                            std::is_nothrow_copy_constructible_v<T>) {
        if (src.size() != dest.size()) return false;
        std::copy(src.begin(), src.end(), dest.begin());
        return true;
    }

    /// <summary>
    /// Move the source span's items to the destination span using std::move if both spans have the same size
    /// The function assumes all entries of the both given span contains valid elements
    /// </summary>
    /// <param name="src"> The source span </param>
    /// <param name="dest"> The destination span </param>
    /// <returns> If both spans have same size returns true, otherwise false</returns>
    template<class T>
    constexpr bool move(Span<T> src, Span<T> dest) noexcept(std::is_nothrow_move_assignable_v<T> &&
                                                            std::is_nothrow_move_constructible_v<T>) {
        if (src.size() != dest.size()) return false;
        std::move(src.begin(), src.end(), dest.begin());
        return true;
    }
}

#include <memory_resource>

namespace kls::pmr {
    namespace detail {
        template<class T>
        inline auto get_alloc(std::pmr::memory_resource *mem) noexcept {
            return std::pmr::polymorphic_allocator<T>(mem);
        }

        template<class T>
        struct destruct_a {
            std::pmr::memory_resource *resource{};

            void operator()(T *ptr) noexcept {
                auto alloc = detail::get_alloc<T>(resource);
                essential::allocator_delete(alloc, ptr);
            }
        };

        template<class T>
        struct destruct_b {
            size_t size{};
            std::pmr::memory_resource *resource{};

            using Elem = std::remove_extent_t<T>;

            explicit destruct_b(size_t size, std::pmr::memory_resource *res) noexcept: size(size), resource(res) {}

            void operator()(Elem *ptr) noexcept {
                for (size_t i = 0; i < size; ++i) std::destroy_at(ptr + i);
                detail::get_alloc<Elem>(resource).deallocate(ptr, size);
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
    unique_ptr<T> make_unique(std::pmr::memory_resource *resource, Ts &&... args) {
        auto alloc = detail::get_alloc<T>(resource);
        return unique_ptr<T>(
                essential::allocator_new<T>(alloc, std::forward<Ts>(args)...),
                detail::destruct_a<T>{resource}
        );
    }

    template<class T, std::enable_if_t<std::is_array_v<T> && std::extent_v<T> == 0, int> = 0>
    unique_ptr<T> make_unique(std::pmr::memory_resource *resource, const size_t size) {
        using Elem = std::remove_extent_t<T>;
        const auto mem = detail::get_alloc<Elem>(resource).allocate(size);
        for (size_t i = 0; i < size; ++i) std::construct_at(mem + i);
        return unique_ptr<T>(mem, detail::destruct_b<T>{size, resource});
    }

    template<class T, class... Ts, std::enable_if_t<std::extent_v<T> != 0, int> = 0>
    void make_unique(Ts &&...) = delete;
}
