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
#include "pushvalue.h"
#include "rand.h"
#include "resid.h"
#include "stringbuilder.h"
#include "stringvar.h"

BSDATAC(areai, 512)
BSDATAC(creature, 1024)
BSDATAC(itemlay, 8192)
BSDATAC(sitei, 2048)

void initialize_gui();
void main_util();
void set_dark_theme();

gamei game;

int getv(gamen v) {
	switch(v) {
	case Money: return human->money;
	default: return game.variables[v];
	}
}

void addv(gamen v, int i) {
	switch(v) {
	case Money: human->money += i; break;
	default: game.variables[v] += i; break;
	}
}

static void all(fnevent proc) {
	pushvalue push(player);
	update_creatures();
	for(auto p : creatures) {
		player = p;
		proc();
	}
}

static void all_next(fnevent proc) {
	pushvalue push(player);
	update_creatures();
	for(auto p : creatures) {
		player = p;
		proc();
		if(!running_scene())
			break;
	}
}

static void monsters_spawning() {
}

static void decoy_items() {
}

static void update_need() {
}

static void update_all_boost(int minutes) {
}

static void update_every_serveral_days() {
}

static void creature_every_day_part() {
}

static void remove_flags(areafn f, int chance) {
	for(short unsigned i = 0; i < mps * mps; i++) {
		if(!area_is(i, f))
			continue;
		if(d100() >= chance)
			continue;
		area_remove(i, f);
	}
}

static void apply_temperature() {
	remove_flags(Iced, 20);
}

static void auto_activate_features() {
}

void pass_minute() {
	addv(Rounds, 1);
	auto minutes = getv(Rounds);
	update_all_boost(minutes);
	all(creature_every_minute);
	while(game.restore_half_turn < minutes) {
		game.restore_half_turn += 5;
		apply_temperature();
	}
	while(game.restore_turn < minutes) {
		all(creature_every_10_minutes);
		monsters_spawning();
		game.restore_turn += 10;
	}
	while(game.restore_hour < minutes) {
		game.restore_hour = (game.restore_hour / 60 + 1) * 60 + rand() % 60;
		update_need();
	}
	while(game.restore_day_part < minutes) {
		all(creature_every_day_part);
		decoy_items();
		game.restore_day_part = (game.restore_day_part / (60 * 4) + 1) * (60 * 4) + rand() % (60 * 4);
	}
	while(game.restore_day < minutes) {
		auto_activate_features();
		game.restore_day = (game.restore_day / (60 * 24) + 1) * (60 * 24) + rand() % (60 * 24);
	}
	while(game.restore_several_days < minutes) {
		update_every_serveral_days();
		game.restore_several_days += xrand(60 * 24 * 2, 60 * 24 * 6);
	}
}

static void make_move_long() {
	if(player->wait_seconds >= 100 * 6)
		player->wait_seconds -= 100 * 6;
}

void skip_long_time() {
	if(!human)
		return;
	while(human->isunaware()) {
		all(make_move_long);
		pass_minute();
	}
}

static bool checkalive() {
	if(!human)
		return false;
	if(!human->hits)
		return false;
	return true;
}

static void play_minute() {
	const int moves_per_minute = 5 * 4;
	bool need_continue = true;
	while(need_continue) {
		need_continue = true;
		for(auto i = 0; i < moves_per_minute; i++) {
			update_los();
			all_next(make_move);
			if(!checkalive() || !running_scene()) {
				need_continue = false;
				break;
			}
		}
		pass_minute();
		skip_long_time();
	}
}

void play_game() {
	while(checkalive() && running_scene())
		play_minute();
}

void end_game() {
}

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

static void create_enemy(short unsigned index, monstern type) {
	create_creature(index, type);
	player->set(Enemy);
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
	// player->equip(PlateMail);
	player->equip(LongBow);
	player->update();
	human = player;
	create_enemy(apos(7, 3), Goblin);
	create_enemy(apos(7, 4), Goblin);
	create_enemy(apos(8, 3), Goblin);
	next_scene(play_game);
	start_scene();
	return 0;
}

#ifndef __GNUC__
int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}
#endif // _DEBUG