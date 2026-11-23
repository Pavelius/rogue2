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

#define	FO(T,R) ((unsigned)&((T*)0)->R)

template<typename T>
struct bsdata {
	static T elements[]; // External initialize
	static unsigned count; // Count of elements
	static const unsigned count_max; // Need to be constant value
	static constexpr void clear() { count = 0; }
	static constexpr bool have(const void* p) { return p >= elements && p < elements + count; }
	static constexpr unsigned indexof(const void* p) { return have(p) ? ((unsigned char*)p - (unsigned char*)elements) / sizeof(T) : -1; }
	static constexpr T* add() { return (count < count_max) ? elements + (count++) : 0; }
	static constexpr T* addz() { for(auto& e : bsdata<T>()) if(!e) return &e; return add(); }
	static constexpr T*	get(const void* p) { return have(p) ? elements + indexof(p) : 0; }
	static constexpr T*	begin() { return elements; }
	static constexpr T*	end() { return elements + count; }
};

#define BSDATAD1(T) template<> const unsigned bsdata<T>::count_max = sizeof(bsdata<T>::elements) / sizeof(bsdata<T>::elements[0]);
#define BSDATAD2(T, N) template<> unsigned bsdata<T>::count = N;

#define BSDATA(T) template<> T bsdata<T>::elements[]
#define BSDATAF(T) BSDATAD1(T); BSDATAD2(T, sizeof(bsdata<T>::elements) / sizeof(bsdata<T>::elements[0]))
#define BSDATAC(T, N) template<> T bsdata<T>::elements[N] = {}; BSDATAD1(T); BSDATAD2(T, 0);