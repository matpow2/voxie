/*
Copyright (c) 2013 Mathias Kaerlev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef VOXIE_TYPES_H
#define VOXIE_TYPES_H

#include <vector>
#include <string>

#if defined(_MSC_VER)

// Define _W64 macros to mark types changing their size, like intptr_t.
#ifndef _W64
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

#ifdef _WIN64
   typedef signed __int64 intptr_t;
   typedef unsigned __int64 uintptr_t;
#else
   typedef _W64 signed int intptr_t;
   typedef _W64 unsigned int uintptr_t;
#endif // _WIN64

typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;

#include <unordered_map>
#include <unordered_set>
#define fast_map std::tr1::unordered_map
#define fast_set std::tr1::unordered_set

#else

#include <stdint.h>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#define fast_map std::tr1::unordered_map
#define fast_set std::tr1::unordered_set

#endif // _MSC_VER

typedef std::vector<std::string> StringList;

#endif // VOXIE_TYPES_H
