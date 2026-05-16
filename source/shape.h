#pragma once

#include "point.h"

enum shapen : unsigned char {
	ShapeCave,
};

struct shapei {
	point		size;
	const char* content;
};