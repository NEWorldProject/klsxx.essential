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

#include <map>
#include <set>
#include <deque>
#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace kls::essential {
    template<template<class T> class allocator>
    struct AllocAliased {
        template<class T> using basic_string = std::basic_string<T, std::char_traits<T>, allocator < T>>;
        using string = basic_string<char>;
        using u16string = basic_string<char16_t>;
        using u32string = basic_string<char32_t>;

        template<class T> using basic_istringstream = std::basic_istringstream<T, std::char_traits<T>, allocator < T>>;
        template<class T> using basic_ostringstream = std::basic_ostringstream<T, std::char_traits<T>, allocator < T>>;
        template<class T> using basic_stringstream = std::basic_stringstream<T, std::char_traits<T>, allocator < T>>;

        using istringstream = basic_istringstream<char>;
        using u16istringstream = basic_istringstream<char16_t>;
        using u32istringstream = basic_istringstream<char32_t>;
        using ostringstream = basic_ostringstream<char>;
        using u16ostringstream = basic_ostringstream<char16_t>;
        using u32ostringstream = basic_ostringstream<char32_t>;
        using stringstream = basic_stringstream<char>;
        using u16stringstream = basic_stringstream<char16_t>;
        using u32stringstream = basic_stringstream<char32_t>;

        template<class T>
        using vector = std::vector<T, allocator < T>>;

        template<class T>
        using deque = std::deque<T, allocator < T>>;

        template<class T, class Pr = std::less<T>>
        using set = std::set<T, Pr, allocator < T>>;

        template<class T, class Pr = std::less<T>>
        using multiset = std::multiset<T, Pr, allocator < T>>;

        template<class T, class V, class Pr = std::less<T>>
        using map = std::map<T, V, Pr, allocator < std::pair<const T, V>>>;

        template<class T, class V, class Pr = std::less<T>>
        using multimap = std::multimap<T, V, Pr, allocator < std::pair<const T, V>>>;

        template<class T, class Hash = std::hash<T>, class Eq = std::equal_to<T>>
        using unordered_set = std::unordered_set<T, Hash, Eq, allocator < T>>;

        template<class T, class Hash = std::hash<T>, class Eq = std::equal_to<T>>
        using unordered_multiset = std::unordered_multiset<T, Hash, Eq, allocator < T>>;

        template<class T, class V, class Hash = std::hash<T>, class Eq = std::equal_to<T>>
        using unordered_map = std::unordered_map<T, V, Hash, Eq, allocator < std::pair<const T, V>>>;

        template<class T, class V, class Hash = std::hash<T>, class Eq = std::equal_to<T>>
        using unordered_multimap = std::unordered_multimap<T, V, Hash, Eq, allocator < std::pair<const T, V>>>;
    };
}
