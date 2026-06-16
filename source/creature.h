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
#include "draw_floatinfo.h"
#include "feats.h"
#include "item.h"
#include "posable.h"
#include "spell.h"

enum messagen : unsigned char;
enum visualn : unsigned char;

struct creature;
struct sitei;

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
	Goblin,
	Bear, Boar, DireBoar, DireWolf, Dog,
	GiantFrog, GiantBat, GiantLizard, GiantSpider,
	Rabbit, RabbitFemale, Racoon, Rat,
	StagBeetle, Wolf,
	FirstMonster = Goblin, LastMonster = Wolf,
};
enum namen : unsigned short {
	HumanNames,
	ElfNames = HumanNames +50,
	DwarfNames = ElfNames + 50,
	OrcNames = DwarfNames + 50,
	NoName = 0xFFFF
};
enum speechn : unsigned char {
	SayHello,
	SayItsMine,
};
struct statable {
	char abilities[Drunk + 1];
	void add(abilityn v, int i) { abilities[v] += i; }
};

extern creature* human;
extern creature* player;
extern creature* opponent;

extern bool need_update_creatures;
extern bool need_end_turn;

struct creature : drawable, posable, statable, featable, spellable, wearable {
	namen			custom_name; // Random name seed or 0xFF if no name
	monstern		type; // Character or Monster type
	statable		basic; // Raw ability before any modification
	spellable		known; // Known spells
	short unsigned	fear_id, boss_id;
	short unsigned	site_id; // Current site of 0xFFFF
	short unsigned	move_index; // Move to this point
	short			hits, hits_maximum, mana;
	int				experience;
	int				wait_seconds;
	const char* name() const;
	creature* getboss() const;
	creature* getfear() const;
	sitei* getsite() const;
	short unsigned bsi() const;
	int	get(abilityn v) const { return (v < sizeof(abilities) / sizeof(abilities[0])) ? abilities[v] : 0; }
	int getlos() const;
	void act(messagen v) const;
	void actn(messagen v) const;
	void act(char separator, const char* format, ...) const;
	void add(abilityn v, int i);
	bool canhear(short unsigned i) const;
	void clear();
	void damage(int v);
	void enchant(spelln spell, unsigned duration);
	bool is(abilityn v) const { return abilities[v] > 0; }
	bool is(featn v) const { return featable::is(v); }
	bool is(featn v, const item& m) const { return featable::is(v) || m.is(v); }
	bool is(spelln v) const { return spellable::is(v); }
	bool ischaracter() const { return type <= Elf; }
	bool isenemy(const creature* p) const { return p->is(Enemy) != is(Enemy); }
	bool isfemale() const { return false; }
	bool ishuman() const { return this == human; }
	bool ismirror() const { return is(Mirrorred); }
	bool isunaware() const { return wait_seconds >= 25 * 4 * 6; }
	bool isvisible() const;
	void fixact(directionn d);
	void fixact(visualn v);
	void fixmsg(const char* format, int param, floatinfon color);
	void heal(int value);
	void kill();
	void look(directionn d);
	bool move(directionn d);
	bool moveto(short unsigned ni);
	bool moveaway(short unsigned ni);
	void remove(featn v) { featable::remove(v); }
	void remove(spelln v) { spellable::remove(v); }
	bool resist(featn resistane, featn immunity) const;
	bool roll(abilityn v, int bonus = 0) const;
	void say(speechn v) const;
	void set(abilityn v, int i) { if(v<=Drunk) abilities[v] = (char)i; }
	void set(featn v) { featable::set(v); }
	void setindex(short unsigned i);
	void update();
	bool use(item& it, bool run);
	void wait(int v) { wait_seconds += v; need_end_turn = true; }
	void wait() { wait(100); }
};

creature* find_creature(short unsigned i);

const char* get_avatar(monstern v);

point i2s(short unsigned i);

int getv(monstern v, abilityn a);

void apply_monster(monstern type);
void create_creature(short unsigned index_position, monstern type);
void creature_every_minute();
void creature_every_10_minutes();
bool is_free(short unsigned i);
void make_move();
void fix(messagen v);
bool player_move(directionn d);
void update_creatures();
void update_player();
bool use_area(short unsigned i);