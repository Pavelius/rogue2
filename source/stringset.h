#pragma once

#include "slice.h"

struct stringset {
	const char*			id;
	slice<const char*>	strings;
	int					value = -1;
};

class pushstring {
	int id, value;
	int find(const char* id);
public:
	pushstring(const char* id, int value = -1);
	~pushstring() { set(value); }
	void set(int value);
};