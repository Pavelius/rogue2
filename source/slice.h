/*
	Copyright 2026 by Pavel Chistyakov

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#pragma once

#ifdef __GNUC__
using size_t = decltype(sizeof(0));
#endif

#define	lenghtof(C) (sizeof(C)/sizeof(C[0]))

typedef int(*fncompare)(const void*, const void*);
typedef int(*fngetnum)(const void* object);

typedef bool(*fnallow)(const void* object, int index);
typedef void(*fncommand)(void* object);
typedef bool(*fncondition)();
typedef void(*fnevent)(); // Callback function of any command executing
typedef bool(*fnvisible)(const void* object);
typedef const char*(*fngetname)(const void* object);

extern "C" int atexit(void(*)());
extern "C" void exit(int exit_code) noexcept(true);
extern "C" void* bsearch(const void* key, const void* base, size_t num, size_t size, fncompare proc) noexcept(true);
extern "C" void* memchr(const void* ptr, int value, long unsigned num) noexcept(true);
extern "C" void* memcpy(void* destination, const void* source, size_t size) noexcept(true);
extern "C" void* memmove(void* destination, const void* source, size_t size) noexcept(true);
extern "C" void* memset(void* destination, int value, size_t size) noexcept(true);
extern "C" void	qsort(void* base, size_t num, size_t size, fncompare proc) noexcept(true);
extern "C" void* malloc(long unsigned size);
extern "C" void* realloc(void *ptr, long unsigned size);
extern "C" void	free(void* pointer);

extern "C" int memcmp(const void* p1, const void* p2, size_t size) noexcept(true);

template<class T>
struct slice {
	T* data;
	size_t count;
	typedef T data_type;
	constexpr slice() : data(0), count(0) {}
	template<size_t N> constexpr slice(T(&v)[N]) : data(v), count(N) {}
	constexpr slice(T* data, unsigned count) : data(data), count(count) {}
	constexpr slice(T* p1, const T* p2) : data(p1), count(p2 - p1) {}
	constexpr explicit operator bool() const { return count != 0; }
	constexpr T* begin() const { return data; }
	constexpr T* end() const { return data + count; }
	constexpr T& operator[](int n) const { return data[n]; }
	constexpr unsigned size() const { return count; }
};