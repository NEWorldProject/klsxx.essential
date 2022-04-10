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

#include "kls/hal/Perf.h"

#if __has_include(<Windows.h>)
#include "kls/hal/System.h"
#define HAS_WIN32_CLOCK 1
#elif __has_include(<mach/mach_time.h>)
#define HAVE_MACH_ABSOLUTE_TIME 1
#include <mach/mach_time.h>
#elif __has_include(<time.h>)
#include <ctime>
#if defined(CLOCK_MONOTONIC)
#define HAVE_CLOCK_MONOTONIC 1
#endif
#endif

namespace {
#ifndef HAS_WIN32_CLOCK
    constexpr int64_t tccSecondsToNanoSeconds = 1000000000;
#endif

#if HAVE_MACH_ABSOLUTE_TIME
    std::optional<int64_t> compute_frequency() noexcept {
        mach_timebase_info_data_t base{};
        mach_timebase_info(&base);
        if (base.denom == 0) return std::nullopt;
        return (tccSecondsToNanoSeconds * int64_t(base.denom)) / int64_t(base.numer);
    }

    auto mach_frequency = compute_frequency();
#endif
}

namespace kls::hal::performance {
    std::optional<int64_t> counter() noexcept {
#if HAVE_MACH_ABSOLUTE_TIME
        return mach_absolute_time();
#elif HAVE_CLOCK_MONOTONIC
        struct timespec ts {};
        int result = clock_gettime(CLOCK_MONOTONIC, &ts);
        if (result != 0) return std::nullopt;
        return (int64_t(ts.tv_sec) * tccSecondsToNanoSeconds) + int64_t(ts.tv_nsec);
#elif HAS_WIN32_CLOCK
        LARGE_INTEGER result{};
        if (QueryPerformanceCounter(&result)) return result.QuadPart; else return std::nullopt;
#else
#error "NO PROPPER CLOCK SUPPORT"
#endif
    }

    std::optional<int64_t> frequency() noexcept {
#if HAVE_MACH_ABSOLUTE_TIME
        return mach_frequency;
#elif HAVE_CLOCK_MONOTONIC
        return tccSecondsToNanoSeconds;
#elif HAS_WIN32_CLOCK
        LARGE_INTEGER result{};
        if (QueryPerformanceFrequency(&result)) return result.QuadPart; else return std::nullopt;
#else
#error "NO PROPPER CLOCK SUPPORT"
#endif
    }
}