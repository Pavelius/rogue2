#include "collectiona.h"
#include "math.h"
#include "rand.h"
#include "stringbuilder.h"

collectiona targets;

static bool exist(void** pb, void** pe, void* v) {
	while(pb < pe) {
		if(*pb == v)
			return true;
		pb++;
	}
	return false;
}

void collectiona::distinct() {
	auto ps = data;
	for(auto p : *this) {
		if(!exist(data, ps, p))
			*ps++ = p;
	}
	count = ps - data;
}

void collectiona::group(fnvgroup proc) {
	for(auto& e : *this)
		e = proc(e);
	distinct();
}

void collectiona::match(fnvfilter proc, bool keep) {
	auto ps = data;
	for(auto v : *this) {
		if(proc(v) != keep)
			continue;
		*ps++ = v;
	}
	count = ps - data;
}

void* collectiona::random() const {
	if(!count)
		return 0;
	return data[rand() % count];
}

void collectiona::shuffle() {
	zshuffle(data, count);
}

void collectiona::top(int number) {
	if(count > (unsigned)number)
		count = number;
}