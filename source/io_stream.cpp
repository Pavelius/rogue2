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
#include "slice.h"

io::stream& io::stream::operator<<(const int n) {
	char temp[32]; stringbuilder sb(temp);
	sb.add("%1i", n);
	return *this << temp;
}

io::stream&	io::stream::operator<<(const char* t) {
	if(t && t[0])
		write(t, zlen(t));
	return *this;
}

unsigned char io::stream::get() {
	unsigned char r = 0;
	read(&r, 1);
	return r;
}

unsigned short io::stream::getLE16() {
	unsigned char u2 = get();
	unsigned char u1 = get();
	return (u2 << 8) | u1;
}

unsigned io::stream::getLE32() {
	unsigned char u4 = get();
	unsigned char u3 = get();
	unsigned char u2 = get();
	unsigned char u1 = get();
	return (u4 << 24) | (u3 << 16) | (u2 << 8) | u1;
}

unsigned io::stream::get32() {
	unsigned v; read(&v, sizeof(v));
	return v;
}

short unsigned io::stream::get16() {
	short unsigned v; read(&v, sizeof(v));
	return v;
}

void* loadb(const char* url, int* size, int additional) {
	void* p = 0;
	if(size)
		*size = 0;
	if(!url || url[0] == 0)
		return 0;
	io::file file(url, StreamRead);
	if(!file)
		return 0;
	int s = file.seek(0, SeekEnd) + additional;
	file.seek(0, SeekSet);
	p = new char[s];
	memset(p, 0, s);
	file.read(p, s);
	if(size)
		*size = s;
	return p;
}

char* loadt(const char* url, int* size) {
	return (char*)loadb(url, size, 1);
}

const char* szext(const char* path) {
	for(const char* r = zend((char*)path); r > path; r--) {
		if(*r == '.')
			return r + 1;
		else if(*r == '\\' || *r == '/')
			return 0;
	}
	return 0;
}

const char* szfname(const char* path) {
	for(const char* r = zend((char*)path); r > path; r--) {
		if(*r == '\\' || *r == '/')
			return r + 1;
	}
	return path;
}

char* szfnamewe(char* result, const char* name) {
	stringbuilder sb(result, result + 260); sb.clear();
	sb.addv(szfname(name), 0);
	char* p = (char*)szext(result);
	if(p && p != result)
		p[-1] = 0;
	return result;
}

char* szurl(char* result, const char* base_url, const char* folder, const char* name, const char* ext, const char* suffix) {
	stringbuilder sb(result, result + 260); sb.clear();
	if(base_url)
		sb.add(base_url);
	if(folder) {
		if(sb)
			sb.add("/");
		sb.add(folder);
	}
	if(name) {
		if(sb)
			sb.add("/");
		sb.add(name);
	}
	if(suffix)
		sb.add(suffix);
	if(ext) {
		if(sb)
			sb.add(".");
		sb.add(ext);
	}
	return result;
}