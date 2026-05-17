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
#include "draw.h"
#include "game.h"
#include "itemlay.h"
#include "resid.h"
#include "stringbuilder.h"
#include "stringvar.h"

BSDATAC(areai, 512)
BSDATAC(creature, 1024)
BSDATAC(itemlay, 8192)
BSDATAC(roomi, 2048)

void initialize_gui();
void main_util();
void set_dark_theme();

static void put_item(short unsigned i, itemn v) {
	item it(v);
	add_item(current_area, i, it);
}

static void game_string(stringbuilder& sb, const char* id) {
	if(stringvar_identifier(sb, id))
		return;
	sb.add(id);
}

static void initialize_strings() {
	metrics::font = gres(FontT);
	metrics::h1 = gres(Font1);
	metrics::h2 = gres(Font2);
	metrics::h3 = gres(Font3);
	stringbuilder::custom = game_string;
}

int main(int argc, char* argv[]) {
	initialize_strings();
	main_util();
	area_clear();
	set_dark_theme();
	initialize_gui();
	area_clear();
	area_set({0, 0, mps, mps}, Grass);
	area_set(apos(3, 3), Grave);
	area_set(apos(3, 4), Grave);
	area_set(apos(6, 4), Tree);
	area_set({5, 5, 4, 4}, WoodenFloor);
	area_set({10, 10, 3, 4}, DeepWater);
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