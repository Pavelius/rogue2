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