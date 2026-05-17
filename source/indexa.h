#pragma once

#include "adat.h"

enum areafn : unsigned char;

typedef bool(*fnareais)(short unsigned i);

struct indexa : adat<short unsigned, 512> {
	void match(areafn v, bool keep = true);
	void match(fnareais allow, bool keep = true);
	void select(short unsigned i, int radius);
	void select(short unsigned i, int radius, fnareais allow, bool keep);
	void select(fnareais allow, bool keep);
	void shuffle();
	void sort(short unsigned i);
};
extern indexa indecies;
