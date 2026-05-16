#include "creature.h"
#include "io_stream.h"
#include "item.h"
#include "pushvalue.h"
#include "sprite_util.h"
#include "stringlocale.h"

using namespace util;

char const* util::image_source = "D:/games/adom/gfx/adom";
char const* source_folder = 0;

static sprite* create_sprite(int count, int cicles = 0, int additional = 0, int size = 1024 * 1024 * 4) {
	auto p = new unsigned char[size];
	sprite_create((sprite*)p, count, cicles, additional);
	return (sprite*)p;
}

static void close_sprite(sprite* p, const char* id, const char* folder = "art") {
	char temp[260];
	sprite_write(szurl(temp, 0, folder, id, "pma"), p);
	delete[](unsigned char*)p;
}

static void add(sprite* p, const char* id, point position) {
	util::add(p, source_folder, id, -1, position, util::add_image);
}

static void convert_items() {
	static const char* avatars[] = {
		"items37", "items37", "items37",
		"item6", "item60", "item3", "item7", "item42", "item24", "item189",
		"item0", "item2", "item4", "item36", "item190",
		"item50", "item76", "item67", "item77",
		"item8", "item29", "item43", "item10", "item11", "item12", "item13",
	};
	assert_enum(avatars, LastItem);
	pushvalue push(source_folder, "items");
	auto p = create_sprite(LastItem + 1);
	for(auto s : avatars)
		add(p, s, {-1000, -1000});
	close_sprite(p, "items");
}

static point get_offset(monstern v) {
	switch(v) {
	case GiantFrog: return point(36, -16);
	case DireBoar: return point(36, -24);
	default: return point(-1000, -24);
	}
}

static void convert_monsters() {
	pushvalue push(source_folder, "NPC");
	auto p = create_sprite(LastMonster - FirstMonster + 1);
	for(auto i = FirstMonster; i <= LastMonster; i = (monstern)(i + 1))
		add(p, get_avatar(i), get_offset(i));
	close_sprite(p, "monsters");
}

void main_util() {
	// convert_monsters();
	// convert_items();
}