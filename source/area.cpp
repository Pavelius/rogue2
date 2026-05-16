#include "area.h"
#include "bsdata.h"
#include "creature.h"
#include "direction.h"
#include "game.h"
#include "math.h"
#include "rand.h"
#include "slice.h"

unsigned short current_area;

unsigned char area_flags[mps * mps];
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
static void addwave(short unsigned v) {
	*push_counter++ = v;
	if(push_counter >= stack + sizeof(stack) / sizeof(stack[0]))
		push_counter = stack;
}

static point getwave() {
	auto index = *pop_counter++;
	if(pop_counter >= stack + sizeof(stack) / sizeof(stack[0]))
		pop_counter = stack;
	return index;
}

static bool is_impassable(featuren v) {
	switch(v) {
	case Statue:
	case Tree: case TreePalm:
		return true;
	default:
		return false;
	}
}

bool is_wall(tilen v) {
	return v == NoTile || v >= WallCave;
}

void block_features() {
	for(auto i = 0; i < mps * mps; i++) {
		if(is_impassable(area_features[i]))
			movement_rate[i] = Blocked;
	}
}

void block_tiles(tilen v) {
	for(auto i = 0; i < mps * mps; i++) {
		if(area_tiles[i] == v)
			movement_rate[i] = Blocked;
	}
}

void block_walls() {
	for(auto i = 0; i < mps * mps; i++) {
		if(is_wall(area_tiles[i]))
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

void area_block(short unsigned m, unsigned short v) {
	movement_rate[m] = v;
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