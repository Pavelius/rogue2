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
#include "bsdata.h"
#include "creature.h"
#include "direction.h"
#include "game.h"
#include "math.h"
#include "rand.h"
#include "slice.h"

unsigned short current_area;
sitei* last_site;

static unsigned char area_flags[mps * mps];
tilen area_tiles[mps * mps];
featuren area_features[mps * mps];
unsigned char area_params[mps * mps];
short unsigned movement_rate[mps * mps];

static short unsigned stack[128 * 128];
static short unsigned* push_counter;
static short unsigned* pop_counter;
static const directionn orientations_7b7[49] = {
	NorthWest, NorthWest, North, North, North, NorthEast, NorthEast,
	NorthWest, NorthWest, NorthWest, North, NorthEast, NorthEast, NorthEast,
	West, West, NorthWest, North, NorthEast, East, East,
	West, West, West, North, East, East, East,
	West, West, SouthWest, South, SouthEast, East, East,
	SouthWest, SouthWest, SouthWest, South, SouthEast, SouthEast, SouthEast,
	SouthWest, SouthWest, South, South, South, SouthEast, SouthEast,
};
static directionn all_directions[] = {West, East, North, South, NorthWest, NorthEast, SouthWest, SouthEast};

static void addwave(short unsigned v) {
	*push_counter++ = v;
	if(push_counter >= stack + sizeof(stack) / sizeof(stack[0]))
		push_counter = stack;
}

static short unsigned getwave() {
	auto index = *pop_counter++;
	if(pop_counter >= stack + sizeof(stack) / sizeof(stack[0]))
		pop_counter = stack;
	return index;
}

bool is_wall(tilen v) {
	return v == NoTile || v >= WallCave;
}

bool is_trap(featuren v) {
	return v == AcidTrap;
}

void block_features(bool ignore_water) {
	for(auto i = 0; i < mps * mps; i++) {
		if(!is_free(area_features[i], ignore_water))
			movement_rate[i] = Blocked;
	}
}

void block_creatures(const creature* ignore) {
	update_creatures();
	for(auto p : creatures) {
		if(ignore == p)
			continue;
		movement_rate[p->index] = Blocked;
	}
}

void block_tiles(tilen v) {
	for(auto i = 0; i < mps * mps; i++) {
		if(area_tiles[i] == v)
			movement_rate[i] = Blocked;
	}
}

void block_tiles(bool ignore_water) {
	for(auto i = 0; i < mps * mps; i++) {
		if(!is_free(area_tiles[i], ignore_water))
			movement_rate[i] = Blocked;
	}
}

void block_zero() {
	for(auto i = 0; i < mps * mps; i++) {
		if(movement_rate[i] >= NotCalculatedMovement)
			movement_rate[i] = Blocked;
	}
}

void area_change(tilen t1, tilen t2) {
	for(auto i = 0; i < mps * mps; i++) {
		if(area_tiles[i] == t1)
			area_tiles[i] = t2;
	}
}

void area_clear() {
	memset(area_tiles, 0, sizeof(area_tiles));
	memset(area_features, 0, sizeof(area_features));
	memset(area_flags, 0, sizeof(area_flags));
	for(auto& e : area_params)
		e = (unsigned char)(rand() % 256);
}

void clear_path() {
	for(auto& e : movement_rate)
		e = NotCalculatedMovement;
	push_counter = stack;
	pop_counter = stack;
}

static void make_wave() {
	while(pop_counter != push_counter) {
		auto m = getwave();
		auto cost = movement_rate[m] + 1;
		for(auto d : all_directions) {
			auto m1 = to(m, d, Blocked);
			if(m1 == Blocked)
				continue;
			auto c1 = movement_rate[m1];
			if(c1 == Blocked || c1 <= cost)
				continue;
			movement_rate[m1] = cost;
			addwave(m1);
		}
	}
}

void make_wave(short unsigned start, short unsigned goal) {
	movement_rate[start] = 0;
	movement_rate[goal] = 0;
	addwave(start);
	make_wave();
	block_zero();
}

directionn move_lower(short unsigned start, short unsigned goal) {
	auto cost = 0xFFFF;
	auto result = Center;
	if(goal == start)
		return Center;
	for(auto d : all_directions) {
		auto i1 = to(start, d, Blocked);
		if(i1 == Blocked)
			continue;
		if(i1 == start)
			return d;
		auto c1 = movement_rate[i1];
		if(c1 >= cost)
			continue;
		result = d;
		cost = c1;
	}
	return result;
}

directionn move_greater(short unsigned start, short unsigned goal) {
	auto cost = 0;
	auto result = Center;
	if(goal == start)
		return Center;
	for(auto d : all_directions) {
		auto i1 = to(start, d, Blocked);
		if(i1 == Blocked)
			continue;
		if(i1 == start)
			return d;
		auto c1 = movement_rate[i1];
		if(c1 == Blocked || c1 <= cost)
			continue;
		result = d;
		cost = c1;
	}
	return result;
}

bool area_is(short unsigned i, areafn f) {
	return (area_flags[i] & (1 << f)) != 0;
}

bool area_iswall(short unsigned i) {
	return is_wall(area_tiles[i]);
}

void area_set(short unsigned m, tilen v) {
	area_tiles[m] = v;
	area_features[m] = NoFeature;
}

void area_set(short unsigned i, featuren v) {
	area_features[i] = v;
}

void area_set(const abox& m, tilen v) {
	auto y2 = m.y + m.h;
	for(auto y = m.y; y < y2; y++) {
		auto i2 = m2i(m.x + m.w, y);
		for(auto i = m2i(m.x, y); i < i2; i++)
			area_set(i, v);
	}
}

void area_hor(short unsigned i, tilen v, short unsigned count) {
	count += i;
	while(i < count)
		area_set(i++, v);
}

void area_ver(short unsigned i, tilen v, short unsigned count) {
	count = i + count * mps;
	while(i < count) {
		area_set(i, v);
		i += mps;
	}
}

void area_set(const abox& m, areafn v) {
	auto y2 = m.y + m.h;
	for(auto y = m.y; y < y2; y++) {
		auto i2 = m2i(m.x + m.w, y);
		for(auto i = m2i(m.x, y); i < i2; i++)
			area_set(i, v);
	}
}

void area_set(short unsigned i, areafn v) {
	area_flags[i] |= 1 << v;
}

void area_remove(short unsigned i, areafn v) {
	area_flags[i] &= ~(1 << v);
}

void area_block(short unsigned m, unsigned short v) {
	movement_rate[m] = v;
}

int area_range(short unsigned i1, short unsigned i2) {
	auto x1 = i1 % mps;
	auto y1 = i1 / mps;
	auto x2 = i2 % mps;
	auto y2 = i2 / mps;
	return imax(iabs(x1 - x2), iabs(y1 - y2));
}

short unsigned to(short unsigned m, directionn d, short unsigned me) {
	switch(d) {
	case North: return up_side(m) ? me : (m - mps);
	case NorthWest: return (up_side(m) || left_side(m)) ? me : (m - mps - 1);
	case NorthEast: return (up_side(m) || right_side(m)) ? me : (m - mps + 1);
	case West: return left_side(m) ? me : (m - 1);
	case East: return right_side(m) ? me : (m + 1);
	case South: return down_side(m) ? me : (m + mps);
	case SouthWest: return (down_side(m) || left_side(m)) ? me : (m + mps - 1);
	case SouthEast: return (down_side(m) || right_side(m)) ? me : (m + mps + 1);
	default: return me;
	}
}

static bool line_los(int x0, int y0, int x1, int y1, fnareais test) {
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	for(;;) {
		if(x0 >= 0 && x0 < mps && y0 >= 0 && y0 < mps) {
			if(!test(m2i(x0, y0)))
				return false;
		}
		if(x0 == x1 && y0 == y1)
			return true;
		e2 = err;
		if(e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if(e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

void area_los(short unsigned i, int r, fnareais test) {
	point m(i % mps, i / mps);
	for(auto x = m.x - r; x <= m.x + r; x++) {
		line_los(m.x, m.y, x, m.y - r, test);
		line_los(m.x, m.y, x, m.y + r, test);
	}
	for(auto y = m.y - r; y <= m.y + r; y++) {
		line_los(m.x, m.y, m.x - r, y, test);
		line_los(m.x, m.y, m.x + r, y, test);
	}
}

bool is_free(tilen v, bool ignore_water) {
	switch(v) {
	case Water:
	case DeepWater:
	case DarkWater:
		if(ignore_water)
			break;
		return false;
	case WallBuilding:
	case WallCave:
	case WallDungeon:
	case WallFire:
	case WallIce:
		return false;
	case NoTile:
		return false;
	default:
		break;
	}
	return true;
}

bool is_free(featuren v, bool ignore_doors) {
	switch(v) {
	case TreePalm: case Tree: case DeadTree:
	case Grave:
	case Statue:
		return false;
	case Door: case LockedDoor: case StuckDoor:
		if(ignore_doors)
			break;
		return false;
	case StairsUp: case StairsDown:
		return false;
	default:
		break;
	}
	return true;
}

int get_move_cost(featuren v) {
	switch(v) {
	case FootHill: return 150;
	case FootMud: return 200;
	default: return 100;
	}
}