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

#include "bsdata.h"
#include "creature.h"
#include "feats.h"
#include "slice.h"
#include "math.h"

struct statblock : featable, statable {
	itemn items[4];
	constexpr statblock() : featable(), statable(), items{} {}
	template<typename T, typename... Ts> constexpr statblock(T v, Ts... args) : statblock(args...) {
		set(v);
	}
	constexpr void set(featn v) {
		feats = 1 << v;
	}
	constexpr void set(itemn v) {
		for(auto& e : items) {
			if(!e) {
				e = v;
				break;
			}
		}
	}
	constexpr void set(abilityn v) {
		switch(v) {
		case Armor: abilities[v]++; break;
		default: abilities[v] += 5; break;
		}
	}
};
struct monsteri {
	char		level;
	monstern	race;
	short		strenght, dexterity, wits;
	const char*	avatar;
	statblock	stats;
};
static monsteri monsters[] = {
	{0, Human, 20, 20, 20}, // Human
	{0, Dwarf, 25, 15, 20}, // Dwarf
	{0, Elf, 20, 20, 20}, // Elf
	{1, Goblin, 10, 30, 10, "0", {Dagger, Dodge}}, // Goblin
	{8, Bear, 50, 20, 4, "347"}, // Bear
	{4, Boar, 30, 25, 3, "357"}, // Boar
	{5, Boar, 45, 25, 3, "478", {StunningHit, PushHit}}, // DireBoar
	{5, Wolf, 35, 30, 4, "91", {BleedingHit}}, // DireWolf
	{2, Dog, 20, 30, 4, "6"}, // Dog
	{2, GiantFrog, 20, 25, 3, "9"}, // GiantFrog
	{2, GiantBat, 10, 30, 3, "2"}, // GiantBat
	{6, GiantLizard, 40, 20, 2, "78", {Armor, Armor}}, // GiantLizard
	{4, GiantSpider, 35, 20, 1, "29", {PoisonHit}}, // GiantSpider
	{1, Rabbit, 6, 20, 3, "103"}, // Rabbit
	{1, Rabbit, 5, 22, 3, "104"}, // RabbitFemale
	{2, Racoon, 10, 30, 3, "89"}, // Racoon
	{1, Rat, 5, 30, 3, "3"}, // Rat
	{10, StagBeetle, 30, 15, 1, "26", {StunningHit}}, // StagBeelte
	{3, Wolf, 20, 30, 4, "113", {BleedingHit, Claws}}, // Wolf
};
static_assert(lenghtof(monsters) == (Wolf + 1), "Invalid tile frames data count");

int getv(monstern v, abilityn a) {
	switch(a) {
	case Level: return monsters[v].level;
	case Strenght: return monsters[v].strenght;
	case Dexterity: return monsters[v].dexterity;
	case Wits: return monsters[v].wits;
	case WeaponSkill: return imax(15, monsters[v].level * 5);
	case BalisticSkill: return imax(15, monsters[v].level * 5);
	default: return 0;
	}
}

const char* get_avatar(monstern v) {
	return monsters[v].avatar;
}

void apply_monster(monstern type) {
	player->basic.abilities[Strenght] += getv(type, Strenght);
	player->basic.abilities[Dexterity] += getv(type, Dexterity);
	player->basic.abilities[Wits] += getv(type, Wits);
	if(type >= FirstMonster && type <= LastMonster) {
		auto& e = monsters[type];
		player->basic.abilities[Level] += e.level;
		player->basic.abilities[WeaponSkill] += getv(type, WeaponSkill);
		player->basic.abilities[BalisticSkill] += getv(type, BalisticSkill);
		player->feats |= e.stats.feats;
		for(auto i = 0; i < lenghtof(e.stats.abilities); i++)
			player->basic.abilities[i] += e.stats.abilities[i];
		for(auto v : e.stats.items) {
			if(v)
				player->equip(v);
		}
	} else {
		player->basic.abilities[WeaponSkill] += 15;
		player->basic.abilities[BalisticSkill] += 15;
		player->basic.abilities[Hits] += 10;
		player->basic.abilities[Mana] += 10;
	}
}