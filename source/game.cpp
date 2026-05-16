#include "area.h"
#include "bsdata.h"
#include "creature.h"
#include "draw.h"
#include "game.h"
#include "itemlay.h"

BSDATAC(creature, 1024)
BSDATAC(itemlay, 8192)

void initialize_gui();
void main_util();
void set_dark_theme();

static void put_item(short unsigned i, itemn v) {
	item it(v);
	add_item(current_area, i, it);
}

int main(int argc, char* argv[]) {
	main_util();
	area_clear();
	set_dark_theme();
	initialize_gui();
	area_clear();
	area_set({0, 0, mps, mps}, Grass);
	area_set({0, 0, mps, mps}, Visible);
	area_set({0, 0, mps, mps}, Explored);
	area_set(apos(3, 3), Grave);
	area_set(apos(3, 4), Grave);
	area_set(apos(6, 4), Tree);
	area_set({5, 5, 4, 4}, WoodenFloor);
	area_hor(apos(5, 5), WallBuilding, 4);
	area_ver(apos(5, 5), WallBuilding, 4);
	area_ver(apos(5 + 4, 5), WallBuilding, 4);
	area_set(apos(5 + 3, 5 + 3), WallBuilding);
	area_set(apos(5 + 2, 5 + 3), WallBuilding);
	area_set(apos(5 + 1, 5 + 3), Door);
	put_item(apos(4, 4), LongSword);
	put_item(apos(4, 4), LongSword);
	put_item(apos(4, 4), LongBow);
	create_creature(apos(4, 3), Human);
	player->equip(LongSword);
	player->equip(LeatherArmor);
	player->equip(LongBow);
	human = player;
	next_scene(choose_game_move);
	start_scene();
	return 0;
}

#ifndef __GNUC__
int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}
#endif // _DEBUG