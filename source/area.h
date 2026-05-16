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

#pragma once

typedef bool(*fnareais)(short unsigned i);

enum directionn : unsigned char;

enum rendern : unsigned char {
	RenderShadow, RenderFeature, RenderWall, RenderCreature,
};
enum landscapen : unsigned char {
	Plains, Forest, DeepForest, Swamp, Hills, Mountains,
};
enum blocktypen : short unsigned {
	NotCalculatedMovement = 0xFFF0, Blocked = 0xFFFF,
};
enum areafn : unsigned char {
	Explored, Visible, Hidden, Darkened, Blooded, Iced, Webbed
};
enum tilen : unsigned char {
	NoTile,
	WoodenFloor, Cave, Dungeon, Grass, GrassCorupted, Rock, Sand, Snow, Lava,
	Water, DarkWater, DeepWater,
	WallCave, WallBuilding, WallDungeon, WallFire, WallIce
};
enum featuren : unsigned char {
	NoFeature,
	Tree, TreePalm, DeadTree,
	ThornBush, FootMud, FootHill, Grave,
	Statue, HiveHole, Hive, Hole,
	Plant, Herbs,
	AltarGood, AltarNeutral, AltarEvil,
	Pit, AcidTrap,
	Door, OpenedDoor, HiddenDoor, LockedDoor, StuckDoor,
	StairsUp, StairsDown,
	GreenPool, RedPool,
	GatePortal, OpenedGatePortal,
	Chest, LockedChest, ChestOpen
};

const short mps = 64;
const int mst = 260;

extern unsigned short current_area;

//extern unsigned char area_flags[mps * mps];
extern unsigned char area_params[mps * mps];
extern tilen area_tiles[mps * mps];
extern featuren area_features[mps * mps];

extern unsigned short movement_rate[mps * mps];

inline short unsigned m2i(unsigned char x, unsigned char y) { return y * mps + x; }
inline bool left_side(short unsigned i) { return (i % mps) == 0; }
inline bool right_side(short unsigned i) { return (i % mps) == (mps - 1); }
inline bool up_side(short unsigned i) { return (i / mps) == 0; }
inline bool down_side(short unsigned i) { return (i / mps) == (mps - 1); }

struct apos {
	unsigned char x, y;
	constexpr apos() : x(0xFF), y(0xFF) {}
	constexpr apos(int x, int y) : x((unsigned char)x), y((unsigned char)y) {}
	constexpr explicit operator bool() const { return x < mps && y < mps; }
	constexpr operator unsigned short() const { return y * mps + x; }
	constexpr bool operator==(apos v) const { return y == v.y && x == v.x; }
	constexpr bool operator!=(apos v) const { return y != v.y || x != v.x; }
	constexpr apos to(unsigned char dx, unsigned char dy) { return {x + dx, y + dy}; }
};
struct aframe {
	unsigned char start, count;
	constexpr explicit operator bool() const { return count > 0; }
	short unsigned get(unsigned char r) const { return start + r % count; }
	short unsigned next() const { return start + count; }
};
struct abox : apos {
	unsigned char w, h;
	constexpr abox() : apos(), w(0), h(0) {}
	constexpr abox(int x, int y, int w, int h) : apos(x, y), w((unsigned char)w), h((unsigned char)h) {}
	constexpr bool have(apos v) const { return v.x >= x && v.x < (x + w) && v.y >= y && v.y < (y + h); }
	constexpr void correct() { if((x + w) > mps) x = mps - w; if((y + h) > mps) y = mps - h; }
	constexpr void clip() { if((x + w) > mps) w = mps - x; if((y + h) > mps) h = mps - y; }
	constexpr apos ru() const { return apos(x + w - 1, y); }
	constexpr apos rd() const { return apos(x + w - 1, y + h - 1); }
	constexpr apos ld() const { return apos(x, y + h - 1); }
};
struct areai {
	short unsigned	index; // Position on world map
	landscapen		type;
	constexpr explicit operator bool() const { return index != 0xFFFF; }
};

void area_change(tilen t1, tilen t2);
void area_clear();
bool area_is(short unsigned i, areafn f);
bool area_iswall(short unsigned i);
void area_los(short unsigned i, int r, fnareais test);
void area_hor(short unsigned i, tilen v, short unsigned count);
void area_remove(short unsigned i, areafn f);
void area_set(short unsigned i, areafn f);
void area_set(short unsigned i, tilen v);
void area_set(short unsigned i, featuren v);
void area_set(const abox& m, tilen v);
void area_set(const abox& m, areafn v);
void area_ver(short unsigned i, tilen v, short unsigned count);
void block_features();
void block_tiles();
void block_walls();
void block_zero();
void clear_path();
bool is_wall(tilen v);
void update_los();