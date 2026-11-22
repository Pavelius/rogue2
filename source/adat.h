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
	limitations under the License.77
*/

#pragma once

#include "slice.h"

template<class T, size_t count_max = 128>
struct adat {
	typedef T data_type;
	size_t count = 0;
	T data[count_max];
	constexpr adat() = default;
	constexpr adat(T v) : adat() { data[count++] = v; }
	template<typename... Ts> constexpr adat(T v, Ts... args) : adat(args...) { data[count++] = v; }
	constexpr const T& operator[](size_t index) const { return data[index]; }
	constexpr T& operator[](size_t index) { return data[index]; }
	explicit operator bool() const { return count != 0; }
	constexpr operator slice<T>() { return slice<T>(data, count); }
	constexpr size_t lenght() const { return count; }
	constexpr size_t maximum() const { return count_max; }
	T* add() { if(count < count_max) return data + (count++); return data; }
	T* addz() { for(auto& e : *this) if(!e) return &e; return add(); }
	T* begin() { return data; }
	T* end() { return data + count; }
	const T* begin() const { return data; }
	const T* end() const { return data + count; }
	const T* endof() const { return data + count_max; }
	size_t find(const T t) const { for(auto& e : *this) if(e == t) return &e - data; return -1; }
	size_t indexof(const void* e) const { if(e >= data && e < data + count) return (T*)e - data; return -1; }
	void add(const T& e) { if(count < count_max) data[count++] = e; }
	void addu(const T& e) { if(!have(e)) add(e); }
	void clear() { count = 0; }
	bool have(const T t) const { return find(t) != -1; }
	bool have(const void* e) const { return e >= data && e < (data + count); }
	void remove(size_t index, int remove_count = 1) { if(index < size_t(count - 1)) memcpy(data + index, data + index + 1, sizeof(data[0]) * (count - index - 1)); count--; }
	void remove(const T t) { remove(find(t), 1); }
	void sort(fncompare proc) { qsort(data, count, sizeof(data[0]), proc); }
	void top(unsigned v) { if(count > v) count = v; }
};