#include "adat.h"
#include "area.h"
#include "bsdata.h"
#include "direction.h"
#include "draw.h"
#include "item.h"
#include "game.h"
#include "rand.h"

const int minimal_size = 10;

static unsigned char current_box_index;
static adat<abox> locations;
static abox crc = {0, 0, mps, mps};
static directionn strait_direction[] = {North, South, West, East};

static abox* add(const abox& source) {
	auto p = locations.add();
	*p = source;
	return p;
}

static abox random(abox e) {
	auto n = e.minimum();
	if(n == e.w && n == e.h)
		return e;
	else if(n == e.w) {
		e.y += (rand() % (e.h - n));
		e.h = n;
	} else {
		e.x += (rand() % (e.w - n));
		e.w = n;
	}
	return e;
}

static void add_location(abox p1) {
	if(p1.maximum() < minimal_size * 2) {
		locations.add(random(p1).resize(1, 1));
		return;
	}
	auto size = p1.maximum();
	auto m = size * xrand(30, 75) / 100;
	if(m < minimal_size)
		m = minimal_size;
	if(size == p1.w) {
		auto p2 = p1;
		p2.x = p1.x + m;
		p2.w = size - m;
		p1.w = m;
		add_location(p1);
		add_location(p2);
	} else {
		auto p2 = p1;
		p2.y = p1.y + m;
		p2.h = size - m;
		p1.h = m;
		add_location(p1);
		add_location(p2);
	}
}

static void add_locations() {
	add_location(crc);
}

static void place(featuren v, int chance) {
	area_set(crc, v, chance);
}

static void place(tilen v) {
	area_set(crc, v);
}

static void place(tilen v, int chance) {
	area_set(crc, v);
}

static void create_road(abox rc) {
	if(rc.w > rc.h) {
		if(rc.x <= 6)
			rc.x = 0;
		if(rc.x >= mps - 6)
			rc.x = mps - 1;
	} else {
		if(rc.y >= 1 && rc.y <= 6)
			rc.y = 0;
		if(rc.y >= mps - 6)
			rc.y = mps - 1;
	}
	area_set(rc, Rock);
}

static void create_city_level(const abox& rc, int level) {
	const int min_building_size = 5;
	const int max_building_size = 8;
	auto w = rc.w;
	auto h = rc.h;
	if(w <= max_building_size + 1 || h <= max_building_size + 1) {
		auto x1 = rc.x;
		auto y1 = rc.y;
		if(w > h)
			w = h;
		if(h > w)
			h = w;
		if(h > max_building_size)
			h = max_building_size;
		if(h != rc.h)
			y1 += rand() % (rc.h - h);
		if(w > max_building_size)
			w = max_building_size;
		if(w != rc.w)
			x1 += rand() % (rc.w - w);
		locations.add({x1 + 1, y1 + 1, w - 1, h - 1});
		return;
	}
	auto m = xrand(40, 60);
	auto r = (d100() < 50) ? 0 : 1;
	if(w > h)
		r = 0;
	else if(h > w)
		r = 1;
	auto rd = 2;
	if(level == 2)
		rd = 1;
	else if(level > 2)
		rd = 0;
	if(r == 0) {
		auto w1 = (w * m) / 100; // horizontal
		if(w1 < min_building_size)
			w1 = min_building_size;
		create_city_level({rc.x, rc.y, w1 - rd - 1, rc.h}, level + 1);
		create_city_level({rc.x + w1, rc.y, rc.w - (w1 - rd - 1), rc.h}, level + 1);
		if(rd)
			create_road({rc.x + w1 - rd, rc.y, w1 - 1, rc.h});
	} else {
		auto h1 = (h * m) / 100; // vertial
		if(h1 < min_building_size)
			h1 = min_building_size;
		create_city_level({rc.x, rc.y, rc.w, h1 - rd - 1}, level + 1);
		create_city_level({rc.x, rc.y + h1, rc.w, rc.h - (h1 - rd - 1)}, level + 1);
		if(rd)
			create_road({rc.x, rc.y + h1 - rd, rc.w, h1 - 1});
	}
}

static void create(landscapen type) {
	switch(type) {
	case Plains:
		place(Grass);
		place(FootHill, 3);
		place(FootMud, 2);
		place(Tree, 2);
		break;
	case Forest:
		place(Grass);
		place(FootHill, 3);
		place(Tree, 10);
		break;
	case DeepForest:
		place(Grass);
		place(FootHill, 3);
		place(Tree, 20);
		break;
	case Village:
		place(Grass);
		place(FootHill, 3);
		place(FootMud, 2);
		place(Tree, 2);
		create_city_level({0, 0, mps - 1, mps - 1}, 1);
		break;
	default:
		break;
	}
}

static int compare_location(const void* v1, const void* v2) {
	auto p1 = (abox*)v1;
	auto p2 = (abox*)v2;
	return p2->area() - p1->area();
}

void area_generate(landscapen type) {
	bsdata<areai>::elements[current_area].type = type;
	create(type);
	locations.clear();
	add_locations();
	locations.sort(compare_location);
	current_box_index = 0;
}

static void set_herbs(short unsigned i) {
	area_set(i, Herbs);
}

static apos side(const abox& rc, directionn dir) {
	switch(dir) {
	case North: return (rc.w < 2) ? apos() : apos(rc.x + 1 + rand() % (rc.w - 2), rc.y);
	case South: return (rc.w < 2) ? apos() : apos(rc.x + 1 + rand() % (rc.w - 2), rc.y + rc.h - 1);
	case West: return (rc.h < 2) ? apos() : apos(rc.x, rc.y + 1 + rand() % (rc.h - 2));
	case East: return (rc.h < 2) ? apos() : apos(rc.x + rc.w - 1, rc.y + 1 + rand() % (rc.h - 2));
	default: return apos();
	}
}

static void place_door(const abox& rc, directionn dir) {
	auto pt = side(rc, dir);
	if(!pt)
		return;
	area_set(pt, WoodenFloor);
	area_set(pt, Door);
}

static void place_house(const abox& rc) {
	area_set(rc, WoodenFloor);
	area_hor(rc.lu(), WallBuilding, rc.w);
	area_hor(rc.ld(), WallBuilding, rc.w);
	area_ver(rc.lu().to(0, 1), WallBuilding, rc.h - 2);
	area_ver(rc.ru().to(0, 1), WallBuilding, rc.h - 2);
	place_door(rc, maprnd(strait_direction));
}

static void create_content(const abox& rc, siten v) {
	switch(v) {
	case MonstersLair:
		area_set(rc.center(), Statue);
		break;
	case HerbsPlace:
		area_set(rc, set_herbs, xrand(2, 4));
		break;
	case TreasureRoom:
		area_set(rc, RandomTreasure, xrand(3, 8));
		break;
	case ThornedArea:
		area_set(rc, ThornBush, 15);
		break;
	case DeepTreeArea:
		area_set(rc, Tree, 20);
		break;
	case LightTreeArea:
		area_set(rc, Tree, 10);
		break;
	case WeaponSmith:
		place_house(rc);
		area_set(rc.resize(1, 1), RandomWeapon, xrand(3, 12));
		break;
	case Shop:
		place_house(rc);
		area_set(rc.resize(1, 1), RandomFood, xrand(3, 12));
		break;
	default:
		break;
	}
}

void create_site(siten type) {
	if(current_box_index >= locations.count)
		return; // Overflow locations.
	last_site = bsdata<sitei>::add();
	last_site->set(locations.data[current_box_index++]);
	last_site->area_index = current_area;
	last_site->type = type;
	create_content(*last_site, last_site->type);
}

void show_locations() {
	pushrect push;
	while(ismodal()) {
		fore = colors::black;
		rectf();
		fore = colors::white;
		const auto z = 3;
		point pt = {20, 20};
		for(auto& e : locations) {
			caret.x = pt.x + e.x * z;
			caret.y = pt.y + e.y * z;
			width = e.w * z - 1;
			height = e.h * z - 1;
			rectf();
		}
		domodal();
		switch(hkey) {
		case KeyEscape: case KeyEnter: case KeySpace: breakmodal(0); break;
		default: break;
		}
	}
}