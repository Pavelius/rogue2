///////////////////////////////////////////////////////////////////////////
// 
//  Copyright 2026 by Pavel Chistyakov
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http ://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#pragma once

#include "slice.h"

template<class T, unsigned count_max = 128>
struct adat {
	typedef T data_type;
	unsigned count;
	T data[count_max];
	adat() : count(0) {}
	constexpr const T& operator[](unsigned index) const { return data[index]; }
	constexpr T& operator[](unsigned index) { return data[index]; }
	explicit operator bool() const { return count != 0; }
	constexpr operator slice<T> () { return slice<T>(data, count);}
	T* add() { if(count < count_max) return data + (count++); return data; }
	T* addz() { for(auto& e : *this) if(!e) return &e; return add(); }
	void add(const T& e) { if(count < count_max) data[count++] = e; }
	void addu(const T& e) { if(!have(e)) add(e); }
	T* begin() { return data; }
	const T* begin() const { return data; }
	void clear() { count = 0; }
	T* end() { return data + count; }
	const T* end() const { return data + count; }
	const T* endof() const { return data + count_max; }
	int	find(const T t) const { for(auto& e : *this) if(e == t) return &e - data; return -1; }
	unsigned getcount() const { return count; }
	unsigned getmaximum() const { return count_max; }
	int	indexof(const void* e) const { if(e >= data && e < data + count) return (T*)e - data; return -1; }
	bool have(const T t) const { for(auto& e : *this) if(e == t) return true; return false; }
	bool have(const void* element) const { return element >= data && element < (data + count); }
	void remove(int index, int remove_count = 1) { if(index < 0) return; if(index<int(count - 1)) memcpy(data + index, data + index + 1, sizeof(data[0]) * (count - index - 1)); count--; }
	void remove(const T t) { remove(find(t), 1); }
	void top(unsigned v) { if(count > v) count = v; }
};
