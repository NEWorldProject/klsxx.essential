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

#include <bit>
#include "Memory.h"

#if defined(_MSC_VER) && (!defined(__clang__) || defined(__c2__))
# include <cstdlib>
# define KLS_ENDIAN_INTRINSIC_BYTE_SWAP_2(x) _byteswap_ushort(x)
# define KLS_ENDIAN_INTRINSIC_BYTE_SWAP_4(x) _byteswap_ulong(x)
# define KLS_ENDIAN_INTRINSIC_BYTE_SWAP_8(x) _byteswap_uint64(x)
#elif defined(__clang__) || defined(__GNUC__ )
# define KLS_ENDIAN_INTRINSIC_BYTE_SWAP_2(x) __builtin_bswap16(x)
# define KLS_ENDIAN_INTRINSIC_BYTE_SWAP_4(x) __builtin_bswap32(x)
# define KLS_ENDIAN_INTRINSIC_BYTE_SWAP_8(x) __builtin_bswap64(x)
#elif defined(__linux__)
# include <byteswap.h>
# define KLS_ENDIAN_INTRINSIC_BYTE_SWAP_2(x) bswap_16(x)
# define KLS_ENDIAN_INTRINSIC_BYTE_SWAP_4(x) bswap_32(x)
# define KLS_ENDIAN_INTRINSIC_BYTE_SWAP_8(x) bswap_64(x)
#else
# define KLS_ENDIAN_NO_INTRINSICS
#endif

namespace kls::essential {
    static_assert(
            std::endian::native == std::endian::little || std::endian::native == std::endian::big,
            "Mixed platform endian is not supported"
    );

    template<class T>
    requires std::is_arithmetic_v<T>
    void byte_swap(T &v) noexcept {
#ifndef KLS_ENDIAN_NO_INTRINSICS
        if constexpr(sizeof(T) == 2) return void(v = KLS_ENDIAN_INTRINSIC_BYTE_SWAP_2(v));
        if constexpr(sizeof(T) == 4) return void(v = KLS_ENDIAN_INTRINSIC_BYTE_SWAP_4(v));
        if constexpr(sizeof(T) == 8) return void(v = KLS_ENDIAN_INTRINSIC_BYTE_SWAP_8(v));
#endif
        T temp = v;
        char *src = static_cast<char *>(&temp), *dst = static_cast<char *>(&v) + sizeof(T) - 1;
        for (int i = 0; i < sizeof(T); ++i) dst[-i] = src[i];
    }

    template<std::endian E>
    class Access {
    public:
        explicit Access(Span<> span) noexcept: m_span(static_span_cast<char>(span)) {}

        template<class T>
        requires std::is_arithmetic_v<T>
        void put(int offset, T v) noexcept {
            if constexpr(sizeof(T) == 1) {
                *pointer(offset) = char(v);
            } else {
                if constexpr(std::endian::native != E) byte_swap(v);
                std::memcpy(pointer(offset), &v, sizeof(T));
            }
        }

        template<class T>
        requires std::is_arithmetic_v<T>
        [[nodiscard]] T get(int offset) const noexcept {
            if constexpr(sizeof(T) == 1) {
                return T(*pointer(offset));
            } else {
                T result{};
                std::memcpy(&result, pointer(offset), sizeof(T));
                if constexpr(std::endian::native != E) byte_swap(result);
                return result;
            }
        }

        auto bytes(int offset, int size) const noexcept { return Span<char>{pointer(offset), size}; }
        auto size() const noexcept { return m_span.size(); }
    private:
        Span<char> m_span;

        [[nodiscard]] char *pointer(int index) noexcept { return m_span.data() + index; }
        [[nodiscard]] const char *pointer(int index) const noexcept { return m_span.data() + index; }
    };

    template<std::endian E>
    class SpanReader {
    public:
        explicit SpanReader(Span<> span) noexcept: m_access(span) {}

        template<class T>
        requires std::is_arithmetic_v<T>
        [[nodiscard]] T get() noexcept {
            auto res = m_access.template get<T>(m_offset);
            m_offset += sizeof(T);
            return res;
        }

        template<class T>
        requires std::is_arithmetic_v<T>
        [[nodiscard]] bool check(int count = 1) const noexcept {
            return (m_offset + sizeof(T) * count) <= m_access.size();
        }

        auto bytes(int size) noexcept {
            auto span = m_access.bytes(m_offset, size);
            return (m_offset += size, span);
        }
    private:
        Access<E> m_access;
        int m_offset{0};
    };

    template<std::endian E>
    class SpanWriter {
    public:
        explicit SpanWriter(Span<> span) noexcept: m_access(span) {}

        template<class T>
        requires std::is_arithmetic_v<T>
        void put(T v) noexcept {
            m_access.put(m_offset, v);
            m_offset += sizeof(T);
        }

        template<class T>
        requires std::is_arithmetic_v<T>
        [[nodiscard]] bool check(int count = 1) const noexcept {
            return (m_offset + sizeof(T) * count) <= m_access.size();
        }

        auto bytes(int size) noexcept {
            auto span = m_access.bytes(m_offset, size);
            return (m_offset += size, span);
        }
    private:
        Access<E> m_access;
        int m_offset{0};
    };
}