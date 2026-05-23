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

#include "area.h"
#include "direction.h"
#include "slice.h"

static char shape_cave[] =
"  XXXXXXX  "
"XXX.....XXX"
"X.........X"
"X....0....X"
"XXX.....XXX"
"  XXX1XXX  "
;

struct shapei {
	apos		size, origin;
	const char* content;
};
static shapei shapes[] = {
	{{11, 6}, {6, 4}, shape_cave},
};
static_assert(lenghtof(shapes) == ShapeCave + 1, "Invalid shapes count");

static apos translate(apos origin, apos c, apos v, directionn d) {
	switch(d) {
	case North: return c.to(origin.x + v.x, origin.y + v.y);
	case South: return c.to(origin.x + v.x, -origin.y - v.y);
	case West: return c.to(-origin.y - v.y, v.x);
	case East: return c.to(v.y, -origin.x - v.x);
	default: return c;
	}
}

void area_set(short unsigned m, shapen shape, directionn d, char symbol, tilen value) {
	auto& e = shapes[shape];
	auto s = e.size.x * e.size.y;
	if(!s)
		return;
	apos c = m;
	for(short unsigned i = 0; i < s; i++) {
		if(e.content[i] != symbol)
			continue;
		auto m1 = translate(e.origin, c, apos(i % e.size.x, i / e.size.x), d);
		if(m1)
			area_set(m1, value);
	}
}

void area_set(short unsigned m, shapen shape, directionn d, char symbol, featuren value) {
	auto& e = shapes[shape];
	auto s = e.size.x * e.size.y;
	if(!s)
		return;
	apos c = m;
	for(short unsigned i = 0; i < s; i++) {
		if(e.content[i] != symbol)
			continue;
		auto m1 = translate(e.origin, c, apos(i % e.size.x, i / e.size.x), d);
		if(m1)
			area_set(m1, value);
	}
}

void area_set(short unsigned m, shapen e, directionn d, tilen floor, tilen wall, tilen door) {
	area_set(m, e, d, 'X', wall);
	area_set(m, e, d, '.', floor);
	area_set(m, e, d, '0', floor);
	area_set(m, e, d, '1', door);
}