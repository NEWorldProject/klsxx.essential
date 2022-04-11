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

#include <atomic>
#include <utility>
#include <concepts>
#include <type_traits>

namespace kls {
    template<class T>
    concept TrivialManipulableData = requires {
        std::is_trivially_copy_assignable_v<T>;
        std::is_trivially_copy_constructible_v<T>;
        std::is_trivially_move_assignable_v<T>;
        std::is_trivially_move_constructible_v<T>;
        std::is_trivially_destructible_v<T>;
    };

    namespace detail {
        class HandleControl {
            using BaseDestruct = void (*)(void *base, void *data) noexcept;
        public:
            void acquire() noexcept { mCount.fetch_add(1); }

            void release(void *data) noexcept {
                if (const auto result = mCount.fetch_sub(1); result == 1) {
                    mBaseDestruct(mBasePointer, data);
                }
            }

            template<class T, class Fn>
            requires requires(T &h, Fn fn) {
                { fn(h) };
                noexcept(fn(h));
            }
            static HandleControl *make(Fn &&destruct);
        private:
            void *mBasePointer{};
            BaseDestruct mBaseDestruct{};
            std::atomic_int mCount{1};
        };
    }

    template<class T> requires TrivialManipulableData<T>
    class Handle {
    public:
        friend struct HandleAccess;
        friend class kls::detail::HandleControl;

        template<class Fn, class ...Ts>
        explicit Handle(Fn &&destruct, Ts &&... args):
                mControl(detail::HandleControl::make<T, Fn>(std::forward<Fn>(destruct))),
                mValue(std::forward<Ts>(args)...) {}
        Handle(const Handle &o) noexcept = default;
        Handle &operator=(const Handle &o) noexcept = default;
        Handle(Handle &&o) noexcept: mValue(o.mValue), mControl(o.mControl) { o.mControl = nullptr; }
        Handle &operator=(Handle &&o) noexcept {
            if (&o != this) mValue = o.mValue, mControl = std::exchange(o.mControl, nullptr);
            return *this;
        }
        ~Handle() = default;
    protected:
        T &value() noexcept { return mValue; }
        const T &value() const noexcept { return mValue; }
    private:
        T mValue;
        detail::HandleControl *mControl;
        void acquire() noexcept { mControl->acquire(); }
        void release() noexcept { mControl->release(this); }
    };

    struct HandleAccess {
        template<class U>
        static U duplicate(U &h) noexcept { return h.acquire(), U{h}; }
        template<class T>
        static void close(Handle<T> &h) noexcept { if (h.mControl) h.release(); }
    };

    template<class H>
    class SafeHandle {
    public:
        explicit SafeHandle(H&& h) noexcept: mHandle(std::move(h)) {}
        SafeHandle(SafeHandle &&) noexcept = default;
        SafeHandle &operator=(SafeHandle &&) noexcept = default;
        SafeHandle(const SafeHandle &o) noexcept: mHandle(HandleAccess::duplicate(o.mHandle)) {}
        SafeHandle &operator=(const SafeHandle &o) noexcept {
            if (&o != this) {
                HandleAccess::close(mHandle);
                mHandle = HandleAccess::duplicate(o.mHandle);
            }
            return *this;
        }
        ~SafeHandle() { HandleAccess::close(mHandle); }
        H release() { return H(std::move(mHandle)); }
        H& operator*() noexcept { return mHandle; }
        const H& operator*() const noexcept { return mHandle; }
        H* operator->() noexcept { return &mHandle; }
        const H* operator->() const noexcept { return &mHandle; }
    private:
        H mHandle;
    };

    namespace detail {
        template<class T, class Fn>
        requires requires(T &h, Fn fn) {
            { fn(h) };
            noexcept(fn(h));
        }
        inline HandleControl *HandleControl::make(Fn &&destruct) {
            using FnConcrete = std::decay_t<Fn>;
            struct Base {
                FnConcrete callable;
                HandleControl control;
            };
            Base *const base = new Base{
                    .callable = FnConcrete(std::forward<Fn>(destruct)),
                    .control = HandleControl{}
            };
            base->control.mBasePointer = base;
            base->control.mBaseDestruct = [](void *base, void *data) noexcept {
                const auto obj = static_cast<Base *>(base);
                obj->callable(static_cast<Handle<T> *>(data)->value());
                delete obj;
            };
            return &base->control;
        }
    }
}