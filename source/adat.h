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

typedef int(*fncompare)(const void*, const void*);

extern "C" void* memmove(void* destination, const void* source, size_t size) noexcept(true);
extern "C" void	qsort(void* base, size_t num, size_t size, fncompare proc) noexcept(true);

template<class T, size_t count_max = 128>
struct adat {
	typedef T data_type;
	size_t count = 0;
	T data[count_max] = {};
	constexpr adat() = default;
	template<typename... Ts> constexpr adat(Ts... args) {
		static_assert(sizeof...(args) <= count_max, "Too many adat initializer values");
		((data[count++] = static_cast<T>(args)), ...); }
	constexpr const T& operator[](size_t index) const { return data[index]; }
	constexpr T& operator[](size_t index) { return data[index]; }
	explicit operator bool() const { return count != 0; }
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
	bool have(const void* e) const { return e >= data && e < (data + count); }
	void remove(size_t index) { if(index >= count) return; if(index < count - 1) memmove(data + index, data + index + 1, sizeof(data[0]) * (count - index - 1)); count--; }
	void sort(fncompare proc) { qsort(data, count, sizeof(data[0]), proc); }
	void top(unsigned v) { if(count > v) count = v; }
};