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

#include "answers.h"
#include "pushvalue.h"
#include "rand.h"

const char* answers::header;
const char* answers::string;

bool answers::interactive = true;

const answers* answers::last;

answers an;

static bool allow(const answers& an, size_t max_width) {
	for(auto& e : an) {
		if(zlen(e.text) > max_width)
			return false;
	}
	return true;
}

static int getcolumns(const answers& an) {
	auto count = an.getcount();
	if(!count)
		return 1;
	auto result = 1;
	if(count > 3 && (count % 3) == 0 && allow(an, 13))
		result = 3;
	else if((count % 2) == 0)
		result = 2;
	return result;
}

unsigned anhotkey(int index) {
	static char hotkeys[] = {
		'1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
		'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
	};
	if((size_t)index > sizeof(hotkeys) / sizeof(hotkeys[0]) - 1)
		index = sizeof(hotkeys) / sizeof(hotkeys[0]) - 1;
	return hotkeys[index];
}

int answers::compare(const void* v1, const void* v2) {
	return szcmp(((answers::element*)v1)->text, ((answers::element*)v2)->text);
}

void answers::addv(long value, const char* text, const char* format) {
	auto p = elements.add();
	p->value = value;
	p->text = sc.get();
	sc.addv(text, format);
	sc.addsz();
}

void answers::add(long value, const char* name, ...) {
	XVA_FORMAT(name);
	addv(value, name, format_param);
}

int	answers::totalweight() const {
	auto n = 0;
	for(auto& e : elements)
		n += e.weight;
	return n;
}

void answers::sort() {
	qsort(elements.data, elements.count, sizeof(elements.data[0]), compare);
}

long answers::random() const {
	if(!elements.count)
		return 0;
	return elements.data[rand() % elements.count].value;
}

long answers::randomweight() const {
	if(!elements.count)
		return 0;
	auto n = totalweight();
	if(!n)
		return 0;
	auto m = rand() % n;
	for(auto& e : elements) {
		m -= e.weight;
		if(m <= 0)
			return e.value;
	}
	return 0;
}

const char* answers::getname(long v) {
	for(auto& e : elements) {
		if(e.value == v)
			return e.text;
	}
	return 0;
}

void answers::clear() {
	elements.clear();
	sc.clear();
}

const answers::element* answers::find(long value) const {
	for(auto& e : elements) {
		if(e.value==value)
			return &e;
	}
	return 0;
}

const char* find_separator(const char* pb) {
	auto p = pb;
	while(*p) {
		if(*p == '-' && p[1] == '+' && p[2] == '-' && (p[3] == 10 || p[3] == 13) && p > pb && (p[-1] == 10 || p[-1] == 13))
			return p;
		p++;
	}
	return 0;
}