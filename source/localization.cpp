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

#include "io_stream.h"
#include "stringbuilder.h"

namespace {
struct locfile {
	char		signature[4];
	unsigned	count;
	const char	text[1];
};
}

extern slice<const char*> locale_data[];

static locfile* loc;

static int total_size() {
	auto n = 0;
	for(auto p = locale_data; *p; p++)
		n += p->count;
	return n;
}

static void clear_locale() {
	if(loc)
		delete[] (char*)loc;
	for(auto p = locale_data; *p; p++) {
		for(auto i = 0; i < p->count; i++)
			p->data[i] = 0;
	}
}

void read_locale(const char* url) {
	clear_locale();
	loc = (locfile*)loadb(url);
	if(!loc)
		return;
	auto pm = loc->text;
	for(auto p = locale_data; *p; p++) {
		for(auto i = 0; i < p->count; i++) {
			p->data[i] = pm;
			pm = zend(pm); pm++;
		}
	}
}

void write_locale(const char* url) {
	locfile header = {"LOC", total_size()};
	io::file file(url, StreamWrite);
	if(!file)
		return;
	file.write(&header, sizeof(header) - sizeof(header.text));
	for(auto p = locale_data; *p; p++) {
		for(auto i = 0; i < p->count; i++) {
			auto pm = p->data[i];
			auto sz = zlen(pm) + 1;
			file.write(pm, sz);
		}
	}
}