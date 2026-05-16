#include "collection.h"
#include "math.h"
#include "rand.h"
#include "stringbuilder.h"

collection records;

static bool exist(unsigned char* pb, unsigned char* pe, unsigned char v) {
	while(pb < pe) {
		if(*pb == v)
			return true;
		pb++;
	}
	return false;
}

void collection::distinct() {
	auto ps = data;
	for(auto p : *this) {
		if(!exist(data, ps, p))
			*ps++ = p;
	}
	count = ps - data;
}

void collection::group(fncgroup proc) {
	for(auto& e : *this)
		e = proc(e);
	distinct();
}

void collection::select(unsigned char v1, unsigned char v2) {
	auto ps = data;
	auto pe = endof();
	for(auto v = v1; v != v2; v++) {
		if(ps >= pe)
			break;
		*ps++ = v;
	}
	count = ps - data;
}

void collection::select(unsigned char v1, unsigned char v2, fncfilter proc, bool keep) {
	if(!proc) {
		select(v1, v2);
		return;
	}
	auto ps = data;
	auto pe = endof();
	for(auto v = v1; v!=v2; v++) {
		if(ps >= pe)
			break;
		if(proc(v) != keep)
			continue;
		*ps++ = v;
	}
	count = ps - data;
}

void collection::match(fncfilter proc, bool keep) {
	auto ps = data;
	for(auto v : *this) {
		if(proc(v) != keep)
			continue;
		*ps++ = v;
	}
	count = ps - data;
}

unsigned char collection::random() const {
	if(!count)
		return 0;
	return data[rand() % count];
}

void collection::shuffle() {
	zshuffle(data, count);
}

unsigned char collection::pick() {
	auto result = (count > 0) ? data[0] : 0;
	remove(0, 1);
	return result;
}

unsigned char collection::picklast() {
	if(!count)
		return 0;
	return data[--count];
}

void collection::top(int number) {
	if(count > (unsigned)number)
		count = number;
}