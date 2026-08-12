#include "adat.h"
#include "area.h"
#include "bsdata.h"
#include "draw.h"
#include "item.h"
#include "game.h"
#include "rand.h"

const int minimal_size = 10;

static unsigned char current_box_index;
static adat<abox> locations;
static abox crc = {0, 0, mps, mps};

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

static void set_treasure(short unsigned i) {
	if(d100() < 90) {
		item it = random_coins();
		it.count = xrand(3, 18);
		add_item(current_area, i, it);
	} else {
		item it = random_gems();
		add_item(current_area, i, it);
	}
}

static void set_herbs(short unsigned i) {
	area_set(i, Herbs);
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
		area_set(rc, set_treasure, xrand(3, 8));
		break;
	case ThornedArea:
		area_set(rc, ThornBush, 20);
		break;
	case DeepTreeArea:
		area_set(rc, Tree, 20);
		break;
	case LightTreeArea:
		area_set(rc, Tree, 10);
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