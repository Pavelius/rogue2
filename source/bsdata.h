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

template<typename T>
struct bsdata {
	static T elements[]; // External initialize
	static unsigned count; // Count of active elements
	static const unsigned count_max; // Need to be constant value (if present)
	static unsigned indexof(const void* p) { return have(p) ? ((unsigned char*)p - (unsigned char*)elements) / sizeof(T) : -1; }
	static T* add() { return (count < count_max) ? elements + (count++) : 0; }
	static T* addz() { for(auto& e : bsdata<T>()) if(!e) return &e; return add(); }
	static T* get(const void* p) { return have(p) ? elements + indexof(p) : 0; }
	static T* begin() { return elements; }
	static T* end() { return elements + count; }
	static void clear() { count = 0; }
	static bool have(const void* p) { return p >= elements && p < elements + count; }
	static void shrinkall() { auto ps = elements, pe = end(); for(auto pb = elements; pb < pe; pb++) if(*pb) *ps++ = *pb; count = ps - elements; }
	static void shrink() { while(count > 0) { auto& e = elements[count - 1]; if(e) break; count--; } }
};

template<typename T> short unsigned getbsi(const T* v) { return (short unsigned)(v - bsdata<T>::elements); }

#define BSDATAD1(T) template<> const unsigned bsdata<T>::count_max = sizeof(bsdata<T>::elements) / sizeof(bsdata<T>::elements[0]);
#define BSDATAD2(T, N) template<> unsigned bsdata<T>::count = N;

#define BSDATA(T) template<> T bsdata<T>::elements[]
#define BSDATAF(T) BSDATAD1(T); BSDATAD2(T, sizeof(bsdata<T>::elements) / sizeof(bsdata<T>::elements[0]))
#define BSDATAC(T, N) template<> T bsdata<T>::elements[N] = {}; BSDATAD1(T); BSDATAD2(T, 0);