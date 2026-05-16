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

#include "answers.h"
#include "area.h"
#include "bsdata.h"
#include "creature.h"
#include "direction.h"
#include "draw.h"
#include "draw_object.h"
#include "game.h"
#include "io_stream.h"
#include "itemlay.h"
#include "keyname.h"
#include "message.h"
#include "pushvalue.h"
#include "screenshoot.h"
#include "resid.h"

const int tsx = 64;
const int tsy = 48;

const int panel_width = 130;
const int window_width = 608;
const int window_height = 376;
const int wears_offset = 80;

const int tick_time = 400;

typedef unsigned (*fnactionkey)(const void* drawable, int index);

struct tilei {
	unsigned char priority;
	aframe frame, decals;
};
static tilei tile_frames[] = {
	{0}, // NoTile borders(-1)
	{0, {0, 1}}, // WoodenFloor
	{0, {9, 3}}, // Cave
	{0, {12, 8}, {14, 3}}, // DungeonFloor
	{0, {1, 4}, {0, 8}}, // Grass minimap(60 176 67)
	{0, {5, 4}, {8, 6}}, // GrassCorupted minimap(66 63 48)
	{0, {44, 1}}, // Rock minimap(96 71 52)
	{0, {35, 4}}, // Sand
	{0, {31, 4}}, // Snow
	{0, {28, 3}}, // Lava
	{0, {40, 4}}, // Water
	{0, {45, 8}}, // DarkWater minimap(22 68 59)
	{0, {53, 8}}, // DeepWater minimap(29 80 120)
	{0, {0, 2}}, // WallCave tile(Cave)
	{0, {13, 1}}, // WallBuilding tile(WoodenFloor)
	{0, {25, 3}}, // WallDungeon tile(DungeonFloor)
	{0, {39, 5}}, // WallFire tile(Lava)
	{0, {55, 3}}, // WallIce tile(Snow)
};
static_assert(lenghtof(tile_frames) == (WallIce + 1), "Invalid tile frames data count");
static tilei flag_frames[] = {
	{0}, // Explored
	{0}, // Visible
	{0}, // Hidden
	{0}, // Darkened
	{0, {0, 1}}, // Blooded
	{0, {0, 1}}, // Iced
	{0, {0, 1}}, // Webbed
};
static_assert(lenghtof(flag_frames) == (Webbed + 1), "Invalid flag frames data count");
static tilei feature_frames[] = {
	{}, // NoFeature
	{10, {7, 4}, {11, 3}}, // Tree minimap(35 79 31 192)
	{10, {100, 6}}, // TreePalm minimap(35 79 31 192)
	{10, {45, 4}, {49, 3}}, // DeadTree
	{10, {97, 3}}, // ThornBush movedifficult(200)
	{5, {14, 1}}, // FootMud movedifficult(300)
	{5, {15, 1}}, // FootHill movedifficult(200)
	{10, {16, 2}}, // Grave
	{10, {43, 2}}, // Statue
	{5, {18, 1}}, // HiveHole
	{10, {19, 1}}, // Hive
	{10, {20, 1}}, // Hole
	{10, {21, 3}}, // Plant minimap(77 111 62 160) power(2)
	{6, {24, 3}}, // Herbs minimap(77 111 62 128)
	{6, {56, 1}}, // AltarGood
	{6, {57, 1}}, // AltarNeutral
	{6, {58, 1}}, // AltarEvil
	{6, {76, 1}}, // Pit
	{6, {28, 1}}, // AcidTrap
	{7, {39, 1}}, // Door minimap(102 70 18 128)
	{7, {41, 1}}, // OpenedDoor minimap(102 70 18 128)
	{7, {}}, // HiddenDoor
	{7, {39, 1}}, // LockedDoor minimap(102 70 18 128) activate_item(TinyKey) random_count(6)
	{7, {39, 1}}, // StuckDoor minimap(102 70 18 128)
	{4, {52, 1}}, // StairsUp
	{4, {53, 1}}, // StairsDown
	{3, {77, 1}}, // GreenPool
	{3, {78, 1}}, // RedPool
	{10, {54, 1}}, // GatePortal
	{10, {55, 1}}, // OpenedGatePortal
	{6, {106, 1}}, // Chest
	{6, {106, 1}}, // LockedChest random_count(6)
	{6, {107, 1}}, // ChestOpen
};
static_assert(lenghtof(feature_frames) == (ChestOpen + 1), "Invalid feature frames data count");
static point next_tile[] = {
	{0, -tsy}, {tsx, 0}, {0, tsy}, {-tsx, 0},
	{tsx, -tsy}, {tsx, tsy}, {-tsx, tsy}, {-tsx, -tsy},
	{0, 0}
};
static array console(1);
answers an;
bool show_floor_rect;

void set_dark_theme();
void show_manual();

static void strokedown() {
	pushrect push;
	auto push_fore = fore;
	fore = push_fore.mix(colors::text, 216);
	line(caret.x, caret.y + height - 1);
	line(caret.x + width - 1, caret.y);
	fore = push_fore.mix(colors::window, 128);
	line(caret.x, caret.y - height + 1);
	line(caret.x - width + 1, caret.y);
	fore = push_fore;
}

static void strokeup() {
	pushrect push;
	auto push_fore = fore;
	fore = push_fore.mix(colors::window, 128);
	line(caret.x, caret.y + height - 1);
	line(caret.x + width - 1, caret.y);
	fore = push_fore.mix(colors::text, 216);
	line(caret.x, caret.y - height + 1);
	line(caret.x - width + 1, caret.y);
	fore = push_fore;
}

static apos i2m(short unsigned v) {
	return {(unsigned char)(v % mps), (unsigned char)(v / mps)};
}

static abox camera_box() {
	auto w = (unsigned char)(getwidth() / tsx + 2);
	auto h = (unsigned char)(getheight() / tsy + 3); // Hight object must be visible
	auto x = (camera.x - tsx / 2) / tsx;
	auto y = (camera.y - tsy / 2) / tsy;
	if(x < 0)
		x = 0;
	if(y < 0)
		y = 0;
	abox r = {(unsigned char)x, (unsigned char)y, w, h};
	r.correct();
	return r;
}

point i2s(short unsigned i) {
	return point((i % mps) * tsx + tsx / 2, (i / mps) * tsy + tsy / 2);
}

static point down(point v) {
	return v + point(0, tsy);
}

static point right(point v) {
	return v + point(tsx, 0);
}

static point left(point v) {
	return v + point(-tsx, 0);
}

static point up(point v) {
	return v + point(0, -tsy);
}

static tilen get_base_tile(tilen v) {
	switch(v) {
	case WallDungeon: return Dungeon;
	case WallCave: return Cave;
	case WallFire: return Lava;
	case WallIce: return Snow;
	default: return v;
	}
}

static unsigned char get_frame(short unsigned m, areafn af) {
	unsigned char r = 0;
	unsigned char f = 1;
	if(!up_side(m) && area_is(m - mps, af))
		r |= f;
	f = f << 1;
	if(!down_side(m) && area_is(m + mps, af))
		r |= f;
	f = f << 1;
	if(!left_side(m) && area_is(m - 1, af))
		r |= f;
	f = f << 1;
	if(!right_side(m) && area_is(m + 1, af))
		r |= f;
	return r;
}

static unsigned char get_frame(short unsigned m, tilen tile) {
	auto r = 0;
	auto f = 1;
	if(!up_side(m) && get_base_tile(area_tiles[m - mps]) == tile)
		r |= f;
	f = f << 1;
	if(!down_side(m) && get_base_tile(area_tiles[m + mps]) == tile)
		r |= f;
	f = f << 1;
	if(!left_side(m) && get_base_tile(area_tiles[m - 1]) == tile)
		r |= f;
	f = f << 1;
	if(!right_side(m) && get_base_tile(area_tiles[m + 1]) == tile)
		r |= f;
	return r;
}

static void paint_shadow() {
	image(gres(ResShadows), ((drawobject*)last_object)->frame, 0);
}

static void paint_feature() {
	image(gres(ResFeatures), ((drawobject*)last_object)->frame, 0);
}

static void paint_wall() {
	image(gres(ResWalls), ((drawobject*)last_object)->frame, 0);
}

static int get_avatar(monstern race, bool female, int armor) {
	return race * 6 + (female ? 1 : 0) + armor * 2;
}

static int get_arms(itemn v) {
	switch(v) {
	case GreatAxe: return 0;
	case GreatMace: return 1;
	case WarHammer: return 3;
	case Spear: return 4;
	case Staff: return 6;
	case Axe: return 36;
	case ShortSword: return 56;
	case LongSword: return 59;
	case Mace: return 45;
	case Dagger: return 39;
	case Scimitar: return 48;
	default: return 61;
	}
}

static int get_acc_back(itemn v) {
	switch(v) {
	case LongBow: return 5;
	case ShortBow: return 7;
	case Crossbow: return 4;
	default: return -1;
	}
}

static int get_armor(itemn v) {
	switch(v) {
	case ChainMail: case ScaleMail: case PlateMail:
		return 2;
	case LeatherArmor: case StuddedArmor: case HideArmor:
		return 1;
	default:
		return 0;
	}
}

static void paint_wear(sprite* ps, int frame, unsigned feats) {
	if(frame != -1)
		image(ps, frame, feats);
}

static void paint_creature() {
	auto p = (creature*)last_object;
	auto feats = p->ismirror() ? ImageMirrorH : 0;
	if(p->ischaracter()) {
		auto pb = gres(ResPCBody);
		auto pa = gres(ResPCArms);
		auto pc = gres(ResPCAccessories);
		// Render inventory layers
		paint_wear(pc, get_acc_back(p->wears[RangedWeapon].type), feats);
		paint_wear(pc, get_acc_back(p->wears[Backward].type), feats);
		// Primary arm
		if(p->wears[MeleeWeapon].istwohanded())
			image(pa, 9, feats);
		else
			image(pa, get_arms(p->wears[MeleeWeapon].type), feats);
		// Torso and armor
		image(pb, get_avatar(p->type, p->isfemale(), get_armor(p->wears[Torso].type)), feats);
		// Secondanary arm
		if(p->wears[MeleeWeapon].istwohanded())
			image(pa, get_arms(p->wears[MeleeWeapon].type), feats);
		else
			image(pa, get_arms(p->wears[MeleeWeaponOffhand].type) - 36 + 10, feats);
	} else
		image(gres(ResMonsters), p->type, feats);
}

static void paint_items() {
	update_items();
	auto pi = gres(ResItems);
	auto mb = camera_box();
	for(auto p : items.records<itemlay>()) {
		if(!mb.have(i2m(p->index)))
			continue;
		if(!area_is(p->index, Explored))
			continue;
		auto f = area_features[p->index];
		if(f == Chest)
			continue;
		point pt = i2s(p->index) - camera;
		image(pt.x, pt.y, pi, p->type, 0);
	}
}

static void link_camera() {
	if(player)
		camera_set(player->position);
}

static void paint_overlapped(short unsigned m, tilen tile, tilen t0, int border) {
	if(tile == t0)
		return;
	auto f = get_frame(m, t0);
	if(!f)
		return;
	image(gres(ResBorders), border * 15 + (f - 1), 0);
}

static bool iswall(short unsigned i, directionn d) {
	auto i1 = to(i, d, i);
	if(!area_is(i1, Explored))
		return true;
	return area_iswall(i1);
}

static void fillwalls() {
	pushrect push;
	pushvalue push_fore(fore);
	caret.x -= tsx / 2; caret.y -= tsy / 2;
	width = tsx; height = tsy;
	fore = color(39, 45, 47);
	rectf();
}

static void filllos() {
	pushrect push;
	pushvalue push_fore(fore);
	pushvalue push_alpha(alpha, (unsigned char)64);
	caret.x -= tsx / 2; caret.y -= tsy / 2;
	width = tsx; height = tsy;
	fore = color(0, 0, 0);
	rectf();
}

static void fillfow() {
	pushrect push;
	pushvalue push_fore(fore);
	caret.x -= tsx / 2; caret.y -= tsy / 2;
	width = tsx; height = tsy;
	fore = color(11, 12, 11);
	rectf();
}

static void floorrect() {
	pushrect push;
	caret.x -= tsx / 2 - 1;
	caret.y -= tsx / 2 - 1;
	width = tsx - 2;
	height = tsy - 2;
	rectb();
}

static void fillfade(color cv, unsigned char av = 128) {
	pushvalue push_fore(fore);
	pushvalue push_alpha(alpha);
	fore = cv;
	alpha = av;
	rectf();
}

static void paint_wall(sprite* pw, point pt, unsigned short i, int bf, int bs) {
	auto pi = gres(ResWalls);
	auto bw = 0;
	auto wn = iswall(i, North);
	auto ws = iswall(i, South);
	auto we = iswall(i, East);
	auto ww = iswall(i, West);
	auto ss = false;
	auto sn = false;
	if(ws) {
		fillwalls();
		if(!ww) {
			if(iswall(i, SouthWest))
				image(pi, bs + 5, 0);
			else
				image(pi, bs + 1, 0);
		} else if(!iswall(i, SouthWest))
			image(pi, bs + 3, 0);
		if(!we) {
			if(iswall(i, SouthEast))
				image(pi, bs + 6, 0);
			else
				image(pi, bs + 2, 0);
		} else if(!iswall(i, SouthEast))
			image(pi, bs + 4, 0);
	} else {
		image(pi, bf, 0);
		if(!ww)
			image(pi, bs + 10, 0);
		if(!we)
			image(pi, bs + 9, 0);
		add_object(RenderShadow, down(pt), bw + 0, 6);
		sn = true;
	}
	auto pu = up(pt);
	if(!wn) {
		add_object(RenderWall, pu, bs + 0, 12);
		if(!we)
			add_object(RenderWall, pu, bs + 7, 13);
		if(!ww)
			add_object(RenderWall, pu, bs + 8, 13);
		add_object(RenderShadow, pu, bw + 1, 6);
		ss = true;
	}
	if(!ww) {
		add_object(RenderShadow, right(pt), bw + 2, 6);
		if(ss)
			add_object(RenderShadow, right(pu), bw + 6, 6);
		if(sn)
			add_object(RenderShadow, right(down(pt)), bw + 4, 6);
	}
	if(!we) {
		add_object(RenderShadow, left(pt), bw + 3, 6);
		if(ss)
			add_object(RenderShadow, left(pu), bw + 7, 6);
		if(sn)
			add_object(RenderShadow, left(down(pt)), bw + 5, 6);
	}
}

static bool is_between_walls(featuren v) {
	return v >= Door && v <= StuckDoor;
}

static void paint_floor() {
	pushrect push;
	auto pi = gres(ResFloor);
	auto pd = gres(ResDecals);
	auto mb = camera_box();
	auto y2 = mb.y + mb.h;
	for(auto y = mb.y; y < y2; y++) {
		auto i2 = m2i(mb.x + mb.w, y);
		for(auto i = m2i(mb.x, y); i < i2; i++) {
			if(!area_is(i, Explored))
				continue;
			auto t = area_tiles[i];
			if(is_wall(t))
				continue;
			auto pt = i2s(i);
			caret = pt - camera;
			auto r = area_params[i];
			auto& ei = tile_frames[t];
			if(ei.frame) {
				image(pi, ei.frame.get(r), 0);
				paint_overlapped(i, t, Cave, 1);
				paint_overlapped(i, t, Dungeon, 3);
				paint_overlapped(i, t, Grass, 0);
				paint_overlapped(i, t, GrassCorupted, 2);
				if(ei.decals)
					image(pd, ei.decals.get(r >> 3), 0);
			}
			for(auto f = Blooded; f <= Webbed; f = (areafn)(f + 1)) {
				if(!area_is(i, f))
					continue;
				if(flag_frames[f].frame)
					image(pd, flag_frames[f].frame.get(r), 0);
			}
			if(show_floor_rect)
				floorrect();
		}
	}
}

static void paint_features() {
	pushrect push;
	auto pw = gres(ResWalls);
	auto pf = gres(ResFeatures);
	auto mb = camera_box();
	auto y2 = mb.y + mb.h;
	for(auto y = mb.y; y < y2; y++) {
		auto i2 = m2i(mb.x + mb.w, y);
		for(auto i = m2i(mb.x, y); i < i2; i++) {
			if(!area_is(i, Explored))
				continue;
			auto pt = i2s(i);
			caret = pt - camera;
			auto r = area_params[i];
			auto t = area_tiles[i];
			if(is_wall(t))
				paint_wall(pw, pt, i, tile_frames[t].frame.get(r), tile_frames[t].frame.next());
			else {
				auto f = area_features[i];
				if(f && !area_is(i, Hidden)) {
					auto& ei = feature_frames[f];
					if(is_between_walls(f)) {
						if(iswall(i, East) && iswall(i, West))
							add_object(RenderFeature, pt, ei.frame.start, ei.priority);
						else if(iswall(i, North) && iswall(i, South))
							add_object(RenderFeature, pt, ei.frame.start + 1, ei.priority);
					} else {
						add_object(RenderFeature, pt, ei.frame.get(r), ei.priority);
						if(ei.decals)
							add_object(RenderFeature, pt, ei.decals.get(r >> 3), ei.priority + 10);
					}
				}
			}
		}
	}
}

static bool area_isb(short unsigned i, areafn v) {
	return (i==Blocked) || area_is(i, v);
}

static void paint_fow() {
	pushrect push;
	auto pi = gres(ResFow);
	auto mb = camera_box();
	auto y2 = mb.y + mb.h;
	height = tsy; width = tsx;
	for(auto y = mb.y; y < y2; y++) {
		auto i2 = m2i(mb.x + mb.w, y);
		for(auto i = m2i(mb.x, y); i < i2; i++) {
			auto pt = i2s(i);
			caret = pt - camera;
			if(!area_is(i, Explored))
				fillfow();
			else {
				auto f = get_frame(i, Explored) ^ 0x0F;
				if((f & 1) != 0)
					image(pi, 0, ImageMirrorV);
				if((f & 2) != 0)
					image(pi, 0, 0);
				if((f & 4) != 0)
					image(pi, 1, 0);
				if((f & 8) != 0)
					image(pi, 1, ImageMirrorH);
				if((f & (2 | 8)) == 0 && !area_isb(to(i, SouthEast), Explored))
					image(pi, 2, ImageMirrorH);
				if((f & (1 | 4)) == 0 && !area_isb(to(i, NorthWest), Explored))
					image(pi, 2, ImageMirrorV);
				if((f & (1 | 8)) == 0 && !area_isb(to(i, NorthEast), Explored))
					image(pi, 2, ImageMirrorV | ImageMirrorH);
				if((f & (2 | 4)) == 0 && !area_isb(to(i, SouthWest), Explored))
					image(pi, 2, 0);
			}
		}
	}
}

static void paint_los() {
	pushrect push;
	auto pi = gres(ResLos);
	auto mb = camera_box();
	auto y2 = mb.y + mb.h;
	height = tsy; width = tsx;
	for(auto y = mb.y; y < y2; y++) {
		auto i2 = m2i(mb.x + mb.w, y);
		for(auto i = m2i(mb.x, y); i < i2; i++) {
			if(!area_is(i, Explored))
				continue;
			auto pt = i2s(i);
			caret = pt - camera;
			if(!area_is(i, Visible))
				filllos();
			else {
				auto f = get_frame(i, Visible) ^ 0x0F;
				auto e = get_frame(i, Explored) ^ 0x0F;
				if(f != e) {
					// 1 - North, 2 - South, 4 - West, 8 - East
					switch(f) {
					case 1: image(pi, 0, ImageMirrorV); break;
					case 2: image(pi, 0, 0); break;
					case 1 + 2: image(pi, 0, ImageMirrorV); image(pi, 0, 0); break;
					case 4: image(pi, 1, 0); break;
					case 1 + 4: image(pi, 3, ImageMirrorV); break;
					case 2 + 4: image(pi, 3, 0); break;
					case 1 + 2 + 4: image(pi, 3, 0); image(pi, 0, ImageMirrorV); break;
					case 8: image(pi, 1, ImageMirrorH); break;
					case 1 + 8: image(pi, 3, ImageMirrorV | ImageMirrorH); break;
					case 2 + 8: image(pi, 3, ImageMirrorH); break;
					case 1 + 2 + 8: image(pi, 0, ImageMirrorV); image(pi, 3, ImageMirrorH); break;
					case 4 + 8: image(pi, 1, 0); image(pi, 1, ImageMirrorH); break;
					case 1 + 4 + 8: image(pi, 3, ImageMirrorV); image(pi, 1, ImageMirrorH); break;
					case 2 + 4 + 8: image(pi, 3, 0); image(pi, 1, ImageMirrorH); break;
					default: break;
					}
				}
				if((f & (2 | 8)) == 0 && !area_isb(to(i, SouthEast), Visible))
					image(pi, 2, ImageMirrorH);
				if((f & (1 | 4)) == 0 && !area_isb(to(i, NorthWest), Visible))
					image(pi, 2, ImageMirrorV);
				if((f & (1 | 8)) == 0 && !area_isb(to(i, NorthEast), Visible))
					image(pi, 2, ImageMirrorV | ImageMirrorH);
				if((f & (2 | 4)) == 0 && !area_isb(to(i, SouthWest), Visible))
					image(pi, 2, 0);
			}
		}
	}
}

static void fillbar(int value) {
	pushrect push;
	if(value > 0) {
		auto push_width = width;
		width = value;
		rectf();
		width = push_width - value;
		caret.x += value;
	}
	//	filldark();
}

static void bar(int value, int maximum, color m) {
	if(!maximum)
		return;
	auto push_fore = fore;
	fore = m;
	fillbar(value * width / maximum);
	strokeborder();
	fore = push_fore;
}

static void fillbarnd(int value) {
	pushrect push;
	if(value > 0) {
		setoffset(1, 1);
		auto push_width = width;
		width = value;
		rectf();
		width = push_width - value;
		caret.x += value;
	}
}

static void bar_shade(int value, int maximum, color m) {
	if(!maximum || !value)
		return;
	if(value > maximum)
		value = maximum;
	auto push_fore = fore;
	fore = m;
	fore.a = 128;
	fillbarnd(value * width / maximum);
	fore = push_fore;
}

static point get_top_position(const creature* p) {
	point result = {0, 0};
	if(p->ischaracter())
		result.y = -76;
	else {
		auto ps = gres(ResMonsters);
		auto& fr = ps->get(p->type);
		result.y -= fr.oy + 4 * 2;
	}
	return result;
}

static void paint_bars(const creature* player) {
	const int dy = 4;
	pushrect push;
	caret.y += get_top_position(player).y;
	caret.x -= tsx / 4;
	width = tsx / 2; height = 4;
	bar(player->hits, player->hits_maximum, colors::red);
	bar_shade(player->get(Poison), player->hits_maximum, colors::green);
	caret.y += dy - 1;
	bar(player->get(Mana), player->basic.abilities[Mana], colors::blue);
}

static void paint_health_bar() {
	if(last_object->render == RenderCreature) {
		auto p = (creature*)last_object;
		if(!area_is(p->index, Visible))
			return;
		if((human == p) || human->isenemy(p))
			paint_bars(p);
	}
}

//void visualeffect::paint(unsigned char param) const {
//	auto pi = gres(resid);
//	if(!pi)
//		return;
//	if(pi->cicles_offset) {
//		auto pc = pi->gcicle(frame);
//		if(pc) {
//			unsigned long current = getobjectstamp() - start_stamp;
//			auto tk = current * pc->count / mst;
//			if(tk < pc->count)
//				image(pi, pc->start + tk, feats);
//		}
//	} else
//		image(pi, param + frame, feats);
//}

static void fieldh(const char* format) {
	char temp[260]; stringbuilder sb(temp);
	sb.add("%1:", format);
	text(temp);
}

static void field(const char* id, int width, const char* format) {
	fieldh(id);
	caret.x += width;
	text(format);
	caret.x += width;
}

static void field(const char* id, int width, int value) {
	char temp[32]; stringbuilder sb(temp);
	sb.add("%1i", value);
	field(id, width, temp);
}

static void fillbuttonpress() {
	auto push_fore = fore;
	fore = colors::button.mix(colors::form, 96);
	rectf();
	fore = push_fore;
}

static void paint_button(const char* format, bool pressed) {
	auto push_caret = caret;
	height = texth();
	if(width == -1)
		width = textw(format) + 6;
	auto push_fore = fore;
	fore = colors::button;
	caret.y += 1;
	rectf();
	if(pressed)
		strokedown();
	else
		strokeup();
	caret.y -= 1;
	fore = push_fore;
	caret.x += (width - textw(format) + 1) / 2;
	if(pressed)
		caret.y += 1;
	text(format, -1, TextBold);
	caret = push_caret;
}

bool button(unsigned key, int format_width) {
	if(!key)
		return false;
	char temp[32]; stringbuilder sb(temp);
	key2str(sb, key);
	auto push_width = width;
	if(format_width == -1)
		format_width = textw(temp) + metrics::padding * 2;
	width = format_width;
	auto result = true; // button(temp, key, pbutton, false);
	width = push_width;
	return result;
}

static void paint_message(const char* format) {
	if(!format || !format[0])
		return;
	pushrect push;
	width = window_width;
	textfs(format);
	caret.y = metrics::padding * 2;
	caret.x = (getwidth() - width - panel_width) / 2;
	strokeout(fillwindow, metrics::padding);
	strokeout(strokeborder, metrics::padding);
	textf(format);
}

static unsigned answer_key(int index) {
	switch(index) {
	case 0: case 1: case 2:
	case 3: case 4: case 5:
	case 6: case 7: case 8:
		return '1' + index;
	case 9: return '0';
	default: return 'A' + (index - 10);
	}
}

static void paint_status() {
	auto push_caret = caret;
	auto push_height = height;
	auto push_width = width;
	caret.x = getwidth() - panel_width;
	caret.y = 0;
	width = panel_width - 1;
	height = getheight() - 1;
	fillform();
	strokeborder();
	setoffset(metrics::border, metrics::border);
	caret.x = getwidth() - panel_width + 1;
	caret.y = getheight() - 128 - 1;
	//	paint_minimap();
	caret = push_caret;
	height = push_height;
	width = push_width - panel_width;
}

void creature::fixact(directionn d) {
	look(d);
	auto s2 = position;
	auto s1 = s2 + (next_tile[d] / 3);
	fixmove(s1, tick_time / 3);
	fixmove(s2, tick_time / 3);
}

static point answer_end;

static void answer_before_paint() {
	caret.x = (getwidth() - window_width - panel_width) / 2;
	caret.y = (getheight() - window_height) / 2;
	answer_end = caret;
	answer_end.y += window_height - texth() - 2;
	width = window_width;
	height = window_height;
	strokeout(fillform, metrics::padding);
	auto push_fore = fore;
	fore = colors::form;
	strokeout(strokeup, metrics::padding);
	strokeout(strokeup, metrics::padding - 1);
	if(answers::header) {
		auto push_font = font;
		font = metrics::h2;
		fore = colors::special;
		texta(answers::header, AlignCenter);
		caret.y += texth();
		font = push_font;
	}
	fore = push_fore;
}

static void answer_paint_cell(int index, long value, const char* format, fnevent proc) {
	auto push_caret = caret;
	auto push_width = width;
	unsigned key = value ? answer_key(index) : KeyEscape;
	auto need_execute = button(key, 24);
	//if(bsdata<creature>::have(value)) {
	//	auto pc = bsdata<creature>::elements + bsdata<creature>::source.indexof(value);
	//	auto pi = pc->getwear(value);
	//	auto st = pc->getwearslot(pi);
	//	if(pi && st >= MeleeWeapon && st <= Legs) {
	//		text(bsdata<weari>::elements[st].getname());
	//		caret.x += wears_offset; width -= wears_offset;
	//	}
	//}
	textf(format);
	width = push_width;
	caret.y = push_caret.y;
	//if(answers::current_columns) {
	//	auto total_width = current_columns->totalwidth() + 4;
	//	char temp[260]; stringbuilder sb(temp);
	//	if(width >= total_width) {
	//		caret.x = push_caret.x + width - total_width;
	//		for(auto p = current_columns; *p; p++) {
	//			sb.clear();
	//			auto push_caret = caret;
	//			auto pn = p->proc(value, sb);
	//			if(p->rightalign) {
	//				pushvalue push_width(width);
	//				pushvalue push_height(height);
	//				textfs(pn);
	//				caret.x += p->width - width;
	//				textf(pn);
	//			} else
	//				textf(pn);
	//			caret = push_caret;
	//			caret.x += p->width;
	//		}
	//	}
	//}
	if(need_execute)
		execute(proc, (long)value);
	caret = push_caret;
	caret.y += texth() + 1;
	width = push_width;
}

static void answer_paint_cell_small(int index, long value, const char* format, fnevent proc) {
	auto push_caret = caret;
	auto push_width = width;
	unsigned key = answer_key(index);
	auto need_execute = button(key, 24);
	width -= caret.x - push_caret.x;
	textf(format);
	if(need_execute)
		execute(proc, (long)value);
	width = push_width;
	caret = push_caret;
	caret.y += texth() + 1;
}

static void get_total_height(const answers& source) {
	auto push_clipping = clipping;
	auto total_height = 0;
	width = window_width - 60;
	auto p = console.begin();
	textfs(p);
	total_height += height;
	auto minimal_width = width;
	auto minimal_height = texth() + 1;
	if(source)
		total_height += metrics::padding;
	for(auto& e : source) {
		width = window_width - 24 - metrics::padding;
		textfs(e.text);
		width += 24 + metrics::padding;
		if(minimal_width < width)
			minimal_width = width;
		if(height < minimal_height)
			height = minimal_height;
		total_height += height;
	}
	width = minimal_width;
	height = total_height;
}

static void paint_message(const answers& source, int window_width) {
	auto p = console.begin();
	if(!p || !p[0])
		return;
	pushrect push;
	width = window_width;
	caret.y = metrics::padding * 2;
	caret.x = (getwidth() - window_width - panel_width) / 2;
	textf(p);
	caret.y += metrics::padding;
	auto index = 0;
	for(auto& e : an)
		answer_paint_cell_small(index++, e.value, e.text, buttonparam);
}

static void paint_dialog_message(int window_width) {
	caret.y = metrics::padding * 2;
	caret.x = (getwidth() - window_width - panel_width) / 2;
	strokeout(fillwindow, metrics::padding);
	strokeout(strokeborder, metrics::padding);
	paint_message(an, window_width);
}

long choose_answers() {
	pushrect push;
	screenshoot screen;
	// pushvalue push_choose(choosing, true);
	while(ismodal()) {
		screen.restore();
		get_total_height(an);
		paint_dialog_message(width);
		domodal();
	}
	screen.restore();
	console.clear();
	return getresult();
}

/*
static void player_info() {
	char temp[1024]; stringbuilder sb(temp); sb.clear();
	player->getinfo(sb);
	textf(temp);
}

static void execute_action() {
	auto push_action = last_action;
	last_action = (siteskilli*)hot.drawable;
	script_execute("ApplyAction", 0);
	last_action = push_action;
}

static void paint_action(const void* p, int index, unsigned key, fnevent proc) {
	auto push_caret = caret;
	auto result = button(key, 32);
	text(getnm(((nameable*)p)->id));
	caret = push_caret;
	if(result)
		execute(proc, index, 0, p);
}

static int get_maximum_width(const collectiona& source) {
	auto result = 0;
	for(auto p : source) {
		auto pn = getnm(((nameable*)p)->id);
		auto w = textw(pn);
		if(result < w)
			result = w;
	}
	return result;
}

static void set_ld_position() {
	caret.y = getheight() - height - metrics::padding * 2;
	caret.x = metrics::padding * 2;
	if(!player)
		return;
	auto po = findobject(player);
	if(!po)
		return;
	auto x = po->position.x - camera.x;
	auto y = po->position.y - camera.y;
	if(y >= caret.y && x >= caret.x && x <= caret.x)
		caret.x = getwidth() - width - metrics::padding * 4 - panel_width;
}

static unsigned get_action_key(const void* pv, int index) {
	auto p = (siteskilli*)pv;
	if(p->key)
		return p->key;
	return answer_key(index);
}

static void paint_collection(const collectiona& source, fnactionkey pkey, fnevent pcommand) {
	if(!source)
		return;
	const int dy = texth() + 1;
	pushrect push;
	width = metrics::padding + get_maximum_width(source) + 32;
	height = last_actions.getcount() * dy;
	set_ld_position();
	strokeout(fillwindow, metrics::padding, metrics::padding);
	strokeout(strokeborder, metrics::padding, metrics::padding);
	auto index = 0;
	for(auto p : source) {
		paint_action(p, index, pkey(p, index), pcommand);
		caret.y += dy;
		index++;
	}
}

static void paint_actions() {
	if(last_actions)
		paint_collection(last_actions, get_action_key, execute_action);
}

static void paint_message() {
	paint_message(console.begin());
}

static void reset_message() {
	static unsigned last_console_size;
	auto current_tick = getcputime();
	auto size = console.size();
	if(!size)
		return;
	if(!last_tick_message || last_console_size < size) {
		last_tick_message = current_tick;
		last_console_size = size;
	}
	auto delay = (current_tick - last_tick_message);
	if(delay >= 4000 || (hot.key == KeyEscape)) {
		console.clear();
		last_tick_message = current_tick;
		last_console_size = 0;
	}
}

static void execute_hotkey() {
	auto pn = (hotkey*)hot.drawable;
	if(pn->data.iskind<widget>())
		choose_dialog(bsdata<widget>::elements[pn->data.value].proc);
	else if(pn->data.iskind<script>())
		bsdata<script>::elements[pn->data.value].proc(pn->data.counter);
}

static void presskey() {
	for(auto& e : bsdata<hotkey>()) {
		if(e.key && hot.key == e.key) {
			execute(execute_hotkey, 0, 0, &e);
			return;
		}
	}
}

void adventure_mode() {
	animate_figures();
	auto human = player;
	if(!human)
		return;
	auto start = human->getwait();
	while((start == human->getwait()) && ismodal()) {
		paintstart();
		paintobjects();
		presskey();
		paintfinish();
		domodal();
	}
}

static void answer_after_paint() {
	auto push_caret = caret;
	caret.x += 2; width -= 2;
	caret.y += 2; height -= 2;
	if(answers::footer)
		textf(answers::footer);
	caret = answer_end;
	auto push_width = width; width = -1;
	if(answers::cancel_text) {
		if(button(answers::cancel_text, KeyEscape, pbutton, false))
			execute(buttoncancel);
	}
	width = push_width;
	caret = push_caret;
}

static point m2a(point m, int z) {
	point r;
	r.x = m.x * z;
	r.y = m.y * z;
	return r;
}

static void paint_minimap_items(point origin, int z) {
	pushrect push;
	height = width = z;
	for(auto& e : area->items) {
		if(!e)
			continue;
		auto i = e.position;
		if(!area->isvalid(i))
			continue;
		if(!area->is(i, Explored))
			continue;
		caret.x = origin.x + i.x * width;
		caret.y = origin.y + i.y * height;
		fillfade(color(255, 255, 0), 192);
	}
}

static void paint_minimap_creatures(point origin, int z, bool use_hearing) {
	pushrect push;
	height = width = z;
	for(auto& e : bsdata<creature>()) {
		if(e.worldpos != game)
			continue;
		auto i = e.getposition();
		if(use_hearing) {
			if(player && !player->canhear(e.getposition()))
				continue;
		} else if(!area->is(i, Explored))
			continue;
		caret.x = origin.x + i.x * width;
		caret.y = origin.y + i.y * height;
		if(e.get(Reputation) <= -20)
			fillfade(color(255, 0, 0), 192);
		else if(e.get(Reputation) >= 20)
			fillfade(color(0, 255, 0), 192);
		else
			fillfade(color(255, 255, 255), 192);
	}
}

static void paint_area(point origin, int z) {
	pushrect push;
	pushvalue push_fore(fore);
	height = width = z;
	point i;
	for(i.y = 0; i.y < area->mps; i.y++) {
		for(i.x = 0; i.x < area->mps; i.x++) {
			auto t = area->tiles[i];
			if(!t || !area->is(i, Explored))
				continue;
			auto p = bsdata<tilei>::elements + t;
			caret.x = origin.x + i.x * width;
			caret.y = origin.y + i.y * height;
			fore = p->minimap;
			rectf();
			auto pf = bsdata<featurei>::elements + (int)area->features[i];
			if(pf->isvisible()) {
				color v = pf->minimap; v.a = 0;
				fillfade(v, pf->minimap.a);
			}
		}
	}
}

static void paint_area_screen(point origin, int z) {
	if(!player)
		return;
	auto pc = player->getposition();
	auto s1 = s2m(camera);
	pushrect push;
	pushvalue push_fore(fore);
	caret.x = origin.x + s1.x * z;
	caret.y = origin.y + s1.y * z;
	width = ((getwidth() - panel_width) / tsx + 2) * z;
	height = (getheight() / tsy + 1) * z;
	fore = colors::white;
	rectb();
}

static void text_header(const char* format) {
	auto push_caret = caret;
	auto push_font = font;
	auto push_fore = fore;
	font = metrics::h2;
	fore = colors::h2;
	caret.x += (width - textw(format)) / 2;
	text(format);
	caret = push_caret;
	caret.y += texth();
	font = push_font;
	fore = push_fore;
}

static void small_header(const char* format) {
	auto push_caret = caret;
	auto push_font = font;
	auto push_fore = fore;
	font = metrics::h3;
	fore = colors::h3;
	caret.x += (width - textw(format)) / 2;
	text(format);
	caret = push_caret;
	caret.y += texth();
	font = push_font;
	fore = push_fore;
}

static void paint_legends(point origin, int z) {
	auto push_caret = caret;
	auto push_fore = fore;
	auto push_font = font;
	font = metrics::font;
	auto index = 1;
	for(auto& e : area->rooms) {
		if(!e.ismarkable())
			continue;
		caret.x = origin.x + center(e.rc).x * z + z / 2;
		caret.y = origin.y + center(e.rc).y * z + z / 2;
		fore = colors::white;
		circlef(7);
		fore = colors::black;
		circle(7);
		char temp[260]; stringbuilder sb(temp);
		sb.add("%1i", index);
		caret.y -= texth() / 2;
		caret.x -= textw(temp) / 2;
		text(temp);
		index++;
	}
	font = push_font;
	fore = push_fore;
	caret = push_caret;
}

static void paint_legends_text(point origin) {
	auto push_caret = caret;
	auto push_fore = fore;
	auto push_font = font;
	font = metrics::font;
	caret = origin;
	auto index = 1;
	char temp[260]; stringbuilder sb(temp);
	for(auto& e : area->rooms) {
		if(!e.ismarkable())
			continue;
		caret.x = origin.x;
		sb.clear(); sb.add("%1i.", index);
		text(temp);
		caret.x += 18;
		text(e.getname());
		caret.y += texth();
		index++;
	}
	font = push_font;
	fore = push_fore;
	caret = push_caret;
}

static void show_area() {
	if(game.level)
		text_header(str("%1 (%Level %2i)", last_location->getname(), game.level));
	else
		text_header(last_location->getname());
	small_header(str(getnm("GlobalMapPosition"), game.position.x, game.position.y));
	const int z = 4;
	point origin;
	origin.x = 16;
	origin.y = (height - area->mps * z) / 2;
	paint_area(origin, z);
	paint_minimap_creatures(origin, z, true);
	paint_minimap_items(origin, z);
	paint_legends(origin, z);
	paint_legends_text({(short)(16 + area->mps * z + 16), origin.y});
	paint_area_screen(origin, z);
}

static void paint_minimap() {
	const auto z = 2;
	paint_area(caret, z);
	paint_minimap_creatures(caret, z, true);
	paint_minimap_items(caret, z);
	paint_area_screen(caret, z);
}

static void beforepaint() {
	paint_status();
}

static void window_back() {
	pushrect push;
	pushvalue push_fore(fore, colors::form);
	setoffset(-metrics::padding + 1, -metrics::padding + 1);
	rectf();
}

static void right_border() {
	pushrect push;
	pushvalue push_fore(fore, colors::border);
	caret.x += width - 1;
	caret.y -= metrics::padding;
	line(caret.x, caret.y + height + metrics::padding * 2);
}

static void show_block(const char* format, ...) {
	pushvalue push_tab(tab_pixels, width - 26);
	char temp[2048]; stringbuilder sb(temp);
	sb.addv(format, xva_start(format));
	textf(temp);
}

static void show_charsheet() {
	setoffset(metrics::padding, metrics::padding);
	pushrect push;
	width = 150;
	right_border();
	show_block("%ListOfFeats%ListOfEffects\n%Reputation");
	caret.y = push.caret.y;
	caret.x += width + metrics::padding * 2;
	right_border();
	show_block("%ListOfSkills");
	pause_keys();
}

*/

static void pause_keys() {
	if(hkey == KeySpace || hkey == KeyEscape)
		execute(buttoncancel);
}

static void camera_direction() {
	const int dx = 4;
	switch(hkey) {
	case KeyLeft: camera.x -= dx; break;
	case KeyRight: camera.x += dx; break;
	case KeyUp: camera.y -= dx; break;
	case KeyDown: camera.y += dx; break;
	default: break;
	}
}

static void add_creatures() {
	update_creatures();
	for(auto p : creatures.records<creature>()) {
		if(p->onscreen())
			add_object(p);
	}
}

static void paint_area() {
	pushrect push;
	auto push_clip = clipping; setclipall();
	set_srceen_area();
	clear_drawobjects();
	clear_objects();
	add_creatures();
	paint_floor();
	paint_items();
	paint_features();
	sort_objects();
	paint_objects();
	paint_los();
	paint_fow();
	for_each_object(paint_health_bar);
	clipping = push_clip;
}

static void paint_test() {
	pushrect push;
	pushfore push_fore(colors::white);
	setpos(10, 10, 400, 200);
	rectb();
}

static void check_orders() {
	if(have_orders())
		breakmodal(1);
}

static void player_move_cmd() {
	if(player_move((directionn)hparam))
		breakmodal(1);
	check_orders();
}

static void direction_keys() {
	switch(hkey) {
	case KeyUp: execute(player_move_cmd, North); break;
	case KeyPageUp: execute(player_move_cmd, NorthEast); break;
	case KeyHome: execute(player_move_cmd, NorthWest); break;
	case KeyDown: execute(player_move_cmd, South); break;
	case KeyLeft: execute(player_move_cmd, West); break;
	case KeyRight: execute(player_move_cmd, East); break;
	case KeyPageDown: execute(player_move_cmd, SouthEast); break;
	case KeyEnd: execute(player_move_cmd, SouthWest); break;
	case 'T': player->is(Mirrorred) ? player->remove(Mirrorred) : player->set(Mirrorred); break;
	default: break;
	}
}

static void update_creature_orders() {
	update_creatures();
	for(auto p : creatures.records<creature>()) {
		auto m = i2s(p->index);
		if(p->position != m)
			p->fixmove(m, tick_time);
	}
}

static void play_game_scene() {
	paint_status();
	paint_area();
	direction_keys();
}

static void play_game_animate() {
	paint_status();
	link_camera();
	paint_area();
}

static void wait_all() {
	wait_all_objects(play_game_animate);
}

static void next_game_move(int mode) {
	switch(mode) {
	case 1: next_scene(choose_game_move); break;
	default: break;
	}
}

void choose_game_move() {
	link_camera();
	update_los();
	auto mode = scene(play_game_scene);
	update_creature_orders();
	wait_all();
	next_game_move(mode);
}

static void camera_initialize() {
	camera_maximum.x = mps * tsx;
	camera_maximum.y = mps * tsy;
}

void initialize_gui() {
	camera_initialize();
	sys_create_window(window_width, window_height);
	sys_caption(getname(ApplicationTitle));
	camera_set_screen(478, 376);
}

BSDATA(drawrender) = {
	{paint_shadow},
	{paint_feature},
	{paint_wall},
	{paint_creature},
};
BSDATAF(drawrender)
static_assert(lenghtof(bsdata<drawrender>::elements) == (RenderCreature + 1), "Invalid render types count");