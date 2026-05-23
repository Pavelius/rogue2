#pragma once

#include "variant.h"

enum spelln : unsigned char;

struct enchanti {
	unsigned		stop;
	short unsigned	parent;
	variant			value;
	constexpr explicit operator bool() const { return stop != 0; }
	void clear();
};

void add_enchant(short unsigned parent, variant value, unsigned stop);
void remove_enchant(short unsigned parent);
void remove_enchant(short unsigned parent, variant value);
void update_enchantments(unsigned stop);