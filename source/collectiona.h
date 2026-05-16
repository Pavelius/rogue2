#include "adat.h"

typedef void* (*fnvgroup)(void* v);
typedef bool (*fnvfilter)(const void* v);

struct collectiona : adat<void*, 256> {
	void group(fnvgroup proc);
	void distinct();
	void match(fnvfilter proc, bool keep);
	void shuffle();
	void top(int number);
	template<typename T> slice<T*> records() const { return slice<T*>((T**)data, (T**)data + count); }
	void* random() const;
};
extern collectiona targets;