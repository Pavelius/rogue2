#include "area.h"
#include "indexa.h"
#include "rand.h"

static unsigned short compare_index;

indexa indecies;

static int compare_distace(const void* v1, const void* v2) {
	auto p1 = *((short unsigned*)v1);
	auto p2 = *((short unsigned*)v2);
	auto d1 = area_range(p1, compare_index);
	auto d2 = area_range(p2, compare_index);
	return d1 - d2;
}

void indexa::sort(short unsigned i) {
	compare_index = i;
	qsort(data, count, sizeof(data[0]), compare_distace);
}

void indexa::select(short unsigned i, int range) {
	auto pb = data;
	auto pe = endof();
	auto cy = i / mps, cx = i % mps;
	for(auto y = cy - range; y <= cy + range; y++) {
		if(y < 0 || y >= mps)
			continue;
		for(auto x = cx - range; x <= cx + range; x++) {
			if(x < 0 || x >= mps)
				continue;
			auto i = y * mps + x;
			if(!area_features[i])
				continue;
			if(pb < pe)
				*pb++ = i;
		}
	}
	count = pb - data;
}

void indexa::select(short unsigned i, int radius, fnareais allow, bool keep) {
	auto pb = data;
	auto pe = endof();
	auto cy = i / mps, cx = i % mps;
	for(auto y = cy - radius; y <= cy + radius; y++) {
		if(y < 0 || y >= mps)
			continue;
		for(auto x = cx - radius; x <= cx + radius; x++) {
			if(x < 0 || x >= mps)
				continue;
			auto i = y * mps + x;
			if(allow(i) != keep)
				continue;
			if(pb < pe)
				*pb++ = i;
		}
	}
	count = pb - data;
}

void indexa::select(fnareais allow, bool keep) {
	auto ps = data;
	for(short unsigned i = 0; i < mps * mps; i++) {
		if(allow(i) != keep)
			continue;
		*ps++ = i;
	}
	count = ps - data;
}

void indexa::match(areafn v, bool keep) {
	auto ps = data;
	for(auto i : *this) {
		if(area_is(i, v) != keep)
			continue;
		*ps++ = i;
	}
	count = ps - data;
}

void indexa::match(fnareais allow, bool keep) {
	auto ps = data;
	for(auto i : *this) {
		if(allow(i) != keep)
			continue;
		*ps++ = i;
	}
	count = ps - data;
}

void indexa::shuffle() {
	zshuffle(data, count);
}