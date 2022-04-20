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

#ifdef _MSC_VER
#define KLS_FORCE_INLINE [[msvc::forceinline]]
#else
#define KLS_FORCE_INLINE [[gnu::always_inline]]
#endif

#ifdef _MSC_VER
#define KLS_ALLOCATE __declspec(allocator)
#else
#define KLS_ALLOCATE
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#    ifdef __GNUC__
#        define KLS_EXPORT __attribute__ ((dllexport))
#        define KLS_IMPORT __attribute__ ((dllimport))
#    else
#        define KLS_EXPORT __declspec(dllexport)
#        define KLS_IMPORT __declspec(dllimport)
#    endif
#else
#    define KLS_EXPORT __attribute__ ((visibility ("default")))
#    define KLS_IMPORT
#endif

#if defined(_MSC_VER)
#pragma warning(disable: 4251)
#ifndef _ENABLE_ATOMIC_ALIGNMENT_FIX
#    define _ENABLE_ATOMIC_ALIGNMENT_FIX
#endif
#ifndef _ENABLE_EXTENDED_ALIGNED_STORAGE
#    define _ENABLE_EXTENDED_ALIGNED_STORAGE
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#endif
#endif