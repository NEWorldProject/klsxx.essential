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

#include <utility>
#include <optional>
#include "kls/Object.h"

namespace kls::essential {
    template<class Fn>
    class Final {
    public:
        explicit Final(Fn &&v) : m_fn(std::forward<Fn>(v)) {}

        explicit Final(const Fn &v) : m_fn(std::forward<Fn>(v)) {}

        ~Final() noexcept { m_fn(); }
    private:
        std::decay_t<Fn> m_fn;
    };

    template<class T, class Fn> requires requires { std::is_trivial_v<T>; }
    class RAII : public NonCopyable {
    public:
        explicit RAII(T v, Fn &&fn) : m_v{std::in_place, v, std::forward<Fn>(fn)} {}
        explicit RAII(T v, const Fn &fn) : m_v{std::in_place, v, std::forward<Fn>(fn)} {}
        RAII(RAII&& o) noexcept : m_v{std::move(o.m_v)} { o.m_v = std::nullopt; }
        RAII& operator=(RAII&& o) noexcept { return delegate_to_move_construct(this, std::move(o)); } //NOLINT

        T reset() noexcept {
            auto res = m_v->v;
            m_v = std::nullopt;
            return res;
        }

        auto get() const noexcept { return m_v->v; }

        ~RAII() noexcept { if (m_v) m_v->fn(m_v->v); }
    private:
        struct Inner {
            T v;
            std::decay_t<Fn> fn;

            explicit Inner(T v, Fn &&fn) : v(v), fn(std::forward<Fn>(fn)) {}
            explicit Inner(T v, const Fn &fn) : v(v), fn(std::forward<Fn>(fn)) {}
        };

        std::optional<Inner> m_v;
    };
}
