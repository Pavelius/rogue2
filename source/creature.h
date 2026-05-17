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

#include "direction.h"
#include "draw_object.h"
#include "feats.h"
#include "item.h"
#include "posable.h"
#include "spell.h"

enum messagen : unsigned char;

struct creature;

enum abilityn : unsigned char {
	Level,
	Strenght, Dexterity, Wits,
	WeaponSkill, BalisticSkill,
	DamageMelee, DamageRanged, DamageThrown,
	Armor, Block, BlockRanged,
	ChanceFailSpell, EnemyAttacks,
	Alchemy, Alertness, Gemcutting, Riding, Dodge, Thievery, Literacy, Metallurgy, Mining,
	Stealth, Survival, Haggling, History, Religion, Woodcutting,
	CarryCapacity,
	Hits, Mana, Faith, Mood, Reputation,
	Poison, Illness, Corrosion, Burning, Freezing, Drunk,
	Experience,
	FirstSkill = Alertness, LastSkill = Woodcutting,
};
enum monstern : unsigned char {
	Human, Dwarf, Elf,
	Bear, Boar, DireBoar, DireWolf, Dog,
	GiantFrog, GiantBat, GiantLizard, GiantSpider,
	Rabbit, RabbitFemale, Racoon, Rat,
	StagBeetle, Wolf,
	FirstMonster = Bear, LastMonster = Wolf,
};
struct statable {
	char abilities[Drunk + 1];
};

extern creature* human;
extern creature* player;
extern creature* opponent;

extern bool need_update_creatures;
extern bool need_end_turn;

struct creature : drawable, posable, statable, featable, spellable, wearable {
	unsigned char	name_id; // Random name seed or 0xFF if no name
	monstern		type; // Character or Monster type
	statable		basic; // Raw ability before any modification
	spellable		known; // Known spells
	short unsigned	fear_id, boss_id;
	short			hits, hits_maximum, mana;
	int				experience;
	int				wait_seconds;
	const char* name() const;
	creature* getboss() const;
	creature* getfear() const;
	int	get(abilityn v) const { return (v < sizeof(abilities) / sizeof(abilities[0])) ? abilities[v] : 0; }
	int getlos() const;
	void act(messagen v) const;
	void add(abilityn v, int i);
	bool canhear(short unsigned i) const;
	void damage(int v) {}
	bool is(featn v) const { return featable::is(v); }
	bool is(abilityn v) const { return abilities[v] > 0; }
	bool ischaracter() const { return type <= Elf; }
	bool isenemy(const creature* p) const { return false; }
	bool isfemale() const { return false; }
	bool ishuman() const { return this == human; }
	bool ismirror() const { return is(Mirrorred); }
	bool isunaware() const { return wait_seconds >= 25 * 4 * 6; }
	void clear();
	void fixact(directionn d);
	void look(directionn d);
	void remove(featn v) { featable::remove(v); }
	void remove(spelln v) { spellable::remove(v); }
	bool resist(featn resistane, featn immunity) { return false; }
	bool roll(abilityn v, int bonus = 0) { return true; }
	void set(abilityn v, int i) { abilities[v] = (char)i; }
	void set(featn v) { featable::set(v); }
	void setindex(short unsigned i);
	void update();
	void wait(int v) { wait_seconds += v; need_end_turn = true; }
	void wait() { wait(100); }
	void waitmove();
};

const char* get_avatar(monstern v);

point i2s(short unsigned i);

int getv(monstern v, abilityn a);

void create_creature(short unsigned index_position, monstern type);
bool is_free(short unsigned i);
void make_move();
void fix(messagen v);
bool player_move(directionn d);
void update_creatures();
bool use_area(short unsigned i);