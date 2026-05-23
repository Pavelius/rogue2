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

#include "array.h"

#ifdef _MSC_VER
#define	BSDATATMPL
#else
#define	BSDATATMPL template<>
#endif

#define	FO(T,R) ((size_t)&((T*)0)->R)

#define BSDATA(e) BSDATATMPL e bsdata<e>::elements[]
#define BSDATAD(e) BSDATATMPL array bsdata<e>::source(sizeof(e));
#define BSDATAE(e) BSDATATMPL array bsdata<e>::source(bsdata<e>::elements, sizeof(bsdata<e>::elements[0]), 0, lenghtof(bsdata<e>::elements));
#define BSDATAF(e) BSDATATMPL array bsdata<e>::source(bsdata<e>::elements, sizeof(bsdata<e>::elements[0]), lenghtof(bsdata<e>::elements), lenghtof(bsdata<e>::elements));
#define BSDATAC(e, c) BSDATATMPL e bsdata<e>::elements[c] = {}; BSDATAE(e)
#define NOBSDATA(e) template<> struct bsdata<e> : bsdata<int> {};

template<typename T>
struct bsdata {
	static T			elements[];
	static array		source;
	static constexpr array*	source_ptr = &source;
	static T*			add() { return (T*)source.add(); }
	static T*			addz() { for(auto& e : bsdata<T>()) if(!e) return &e; return add(); }
	static T*			find(const char* id) { return (T*)source.findv(id, 0); }
	static constexpr bool have(const void* p) { return source.have(p); }
	static constexpr bool haveb(const void* p) { return source.haveb(p); }
	static constexpr T&	get(int i) { return begin()[i]; }
	static constexpr T*	get(const void* p) { return have(p) ? (T*)source.ptr(source.indexof(p)) : 0; }
	static constexpr T*	begin() { return (T*)source.data; }
	static constexpr T*	end() { return (T*)source.data + source.getcount(); }
};
template<> struct bsdata<int> { static constexpr array* source_ptr = 0; };
NOBSDATA(unsigned)
NOBSDATA(short)
NOBSDATA(unsigned short)
NOBSDATA(char)
NOBSDATA(unsigned char)
NOBSDATA(const char*)
NOBSDATA(bool)
