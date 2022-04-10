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
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <concepts>
#include <algorithm>
#include <type_traits>

namespace kls {
    /// <summary>
    /// Represents a non-owning contiguous span of memory that is capable of holding 'size' entries of 'T' typed data
    /// with the exception of T = void
    ///
    /// This data-type is implicitly convertable to Span<> to allow for a cleaner type erasure
    ///
    /// This data-type implements the std::ranges::contiguous_range concept and provides size() for information
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<class T = void>
    class Span {
    public:
        template<class Range>
        requires std::ranges::contiguous_range<Range>
        constexpr Span(Range &range) noexcept: Span{std::ranges::data(range), std::ranges::size(range)} {}
        template<class U>
        requires std::integral<U>
        constexpr Span(T *data, U size) noexcept : m_begin{data}, m_size(size) {}
        template<class U>
        requires std::integral<U>
        constexpr Span(const T *data, U size) noexcept : m_begin{const_cast<T *>(data)}, m_size(size) {}
        constexpr Span(Span &&) noexcept = default;
        constexpr Span(const Span &) noexcept = default;
        constexpr Span &operator=(Span &&) noexcept = default;
        constexpr Span &operator=(const Span &) noexcept = default;
        constexpr T *begin() noexcept { return m_begin; }
        constexpr T *end() noexcept { return m_begin + m_size; }
        constexpr const T *begin() const noexcept { return m_begin; }
        constexpr const T *end() const noexcept { return m_begin + m_size; }
        constexpr T *data() noexcept { return m_begin; }
        constexpr const T *data() const noexcept { return m_begin; }
        constexpr size_t size() const noexcept { return m_size; }
        [[nodiscard]] Span trim_front(size_t diff) const noexcept {
            if (diff > m_size) diff = m_size;
            return Span{m_begin + diff, m_size - diff};
        }
        [[nodiscard]] Span trim_back(size_t diff) const noexcept {
            if (diff > m_size) diff = m_size;
            return Span{m_begin, m_size - diff};
        }
        [[nodiscard]] Span keep_front(size_t size) const noexcept {
            if (size > m_size) size = m_size;
            return Span{m_begin, size};
        }
        [[nodiscard]] Span keep_back(size_t size) const noexcept {
            if (size > m_size) size = m_size;
            return Span{m_begin + m_size - size, size};
        }
        constexpr operator Span<void>() const noexcept;
    private:
        T *m_begin;
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
        template<class U>
        requires std::integral<U>
        constexpr Span(void *data, U size) noexcept : m_begin{data}, m_size(size) {}
        template<class U>
        requires std::integral<U>
        constexpr Span(const void *data, U size) noexcept : m_begin{const_cast<void *>(data)}, m_size(size) {}
        constexpr Span(Span &&) noexcept = default;
        constexpr Span(const Span &) noexcept = default;
        constexpr Span &operator=(Span &&) noexcept = default;
        constexpr Span &operator=(const Span &) noexcept = default;
        constexpr void *data() noexcept { return m_begin; }
        constexpr const void *data() const noexcept { return m_begin; }
        size_t size() const noexcept { return m_size; }
        [[nodiscard]] Span trim_front(size_t diff) const noexcept {
            if (diff > m_size) diff = m_size;
            return Span{static_cast<char *>(m_begin) + diff, m_size - diff};
        }
        [[nodiscard]] Span trim_back(size_t diff) const noexcept {
            if (diff > m_size) diff = m_size;
            return Span{static_cast<char *>(m_begin), m_size - diff};
        }
        [[nodiscard]] Span keep_front(size_t size) const noexcept {
            if (size > m_size) size = m_size;
            return Span{static_cast<char *>(m_begin), size};
        }
        [[nodiscard]] Span keep_back(size_t size) const noexcept {
            if (size > m_size) size = m_size;
            return Span{static_cast<char *>(m_begin) + m_size - size, size};
        }
    private:
        void *m_begin;
        size_t m_size;
    };

    template<class T>
    inline constexpr Span<T>::operator Span<void>() const noexcept { return Span<void>(m_begin, m_size * sizeof(T)); }

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

    template<class T>
    concept span_safely_copyable_type = requires {
        std::same_as<T, void> ||
        (std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>);
    };

    /// <summary>
    /// Copy the source span's items to the destination span using std::copy if both spans have the same size
    /// The function assumes all entries of the both given span contains valid elements
    /// </summary>
    /// <param name="src"> The source span </param>
    /// <param name="dest"> The destination span </param>
    /// <returns> If both spans have same size returns true, otherwise false</returns>
    template<class T>
    constexpr bool copy(Span<T> src, Span<T> dst) noexcept(span_safely_copyable_type<T>) {
        if (src.size() != dst.size()) return false;
        if constexpr(std::same_as<void, T>)
            std::copy(src.begin(), src.end(), dst.begin());
        else
            std::memmove(dst.data(), src.data(), dst.size());
        return true;
    }

    template<class T>
    concept span_safely_movable_type = requires {
        (std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>);
    };

    /// <summary>
    /// Move the source span's items to the destination span using std::move if both spans have the same size
    /// The function assumes all entries of the both given span contains valid elements
    /// </summary>
    /// <param name="src"> The source span </param>
    /// <param name="dest"> The destination span </param>
    /// <returns> If both spans have same size returns true, otherwise false</returns>
    template<class T>
    constexpr bool move(Span<T> src, Span<T> dst) noexcept(span_safely_movable_type<T>) {
        if (src.size() != dst.size()) return false;
        std::move(src.begin(), src.end(), dst.begin());
        return true;
    }

    /// <summary>
    /// Relocates the source span's items to the destination span
    /// The implementation uses move constructor and destructs the old item immediately
    /// The function assumes all entries of the destination span is valid items and source span contains no objects
    /// </summary>
    /// <param name="src"> The source span </param>
    /// <param name="dest"> The destination span </param>
    /// <returns> If both spans have same size returns true, otherwise false</returns>
    template<class T> requires requires() {
        std::is_nothrow_move_constructible_v<T>;
        std::is_nothrow_destructible_v<T>;
    }
    constexpr bool relocate(Span<T> src, Span<T> dst) noexcept {
        if (src.size() != dst.size()) return false;
        const auto size = dst.size();
        for (size_t i = 0; i < size; ++i) {
            T* o_src = src.data() + i, o_dst = dst.data() + i;
            std::construct_at(o_dst, std::move(*o_src));
            std::destroy_at(o_src);
        }
        return true;
    }
}