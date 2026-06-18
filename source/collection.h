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

#include "adat.h"
#include "slice.h"

typedef unsigned char (*fncgroup)(unsigned char v);
typedef bool (*fncfilter)(unsigned char v);

struct collection : adat<unsigned char, 256> {
	void group(fncgroup proc);
	void distinct();
	void match(fncfilter proc, bool keep);
	void select(unsigned char v1, unsigned char v2);
	void select(unsigned char v1, unsigned char v2, fncfilter proc, bool keep);
	void shuffle();
	void sort(fncompare proc) { qsort(data, count, sizeof(data[0]), proc); }
	void top(int number);
	template<typename T> slice<T> records() const { return slice<T>((T*)data, (T*)data+count); }
	unsigned char pick();
	unsigned char picklast();
	unsigned char random() const;
};
extern collection records;