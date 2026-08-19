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

#include "variant.h"

struct creature;

extern bool need_update_items;

enum featn : unsigned char;
enum messagen : unsigned char;
enum glown : unsigned char;

enum wearn : unsigned char {
	MeleeWeapon, MeleeWeaponOffhand, RangedWeapon, Ammunition,
	Torso, Head, Neck, Backward, Girdle, Gloves, FingerRight, FingerLeft, Elbows, Legs,
	Backpack
};
enum magicn : unsigned char {
	Mundane, Cursed, Blessed, Artifact,
};
enum itemn : unsigned char {
	NoItem,
	Staff, Spear, Axe, Mace, WarHammer, GreatMace, GreatAxe,
	Dagger, ShortSword, LongSword, Scimitar, GreatSword,
	ShortBow, LongBow, Crossbow, HeavyCrossbow,
	Robe, LeatherArmor, StuddedArmor, HideArmor, ScaleMail, ChainMail, PlateMail,
	ShieldSmall, ShieldMedium, ShieldLarge, ShieldTower,
	LeatherBoots, IronBoots,
	BluePotion, GreenPotion, RedPotion,
	BlueTome, GreenTome, RedTome,
	BlueRod, GreenRod, RedRod,
	// Countable items start here
	Ration, Apple, Bread, Meat, Shell, Bones, BloodyRemains,
	Arrow, Bolt,
	BlueGem, WhiteGem, YellowGem, GreenGem, RedGem,
	CP, SP, GP,
	LastItem = GP,
	// Random items set
	RandomTreasure, RandomCoins, RandomGems,
	RandomBlades, RandomBladesSmall, RandomMartialWeapon, RandomMissileWeapon,
	RandomArmor, RandomWeapon
};

extern const char* power_names[];
extern const char* item_state_names[4 * 2]; // Status: used and damaged

struct item {
	itemn type;
	union {
		unsigned char count;
		struct {
			unsigned char power : 3; // Item magical power index (1-15) or 0 - if no magical power
			unsigned char known_magic : 1; // Identified item
			unsigned char hits : 2; // 0-1 undamaged, 2 used, 3 damaged
			magicn magic : 2; // Mundane, Cursed, Blessed, Artifact
		};
	};
	constexpr item() : type((itemn)0), count(0) {}
	constexpr item(itemn v) : type(v), count(countable() ? 1 : 0) {}
	constexpr item(itemn v, unsigned char count) : type(v), count(count) {}
	explicit operator bool() const { return type != (itemn)0; }
	constexpr bool countable() const { return type >= Ration; }
	itemn ammo() const;
	creature* owner() const;
	wearn equiped() const;
	wearn wear() const;
	const char* description() const;
	const char* name() const;
	variant getpower() const;
	int	armor() const;
	int	block() const;
	int	cost() const;
	int damage() const;
	int dodge() const;
	int speed() const;
	int	weight() const;
	int getcount() const { return countable() ? count : 1; }
	void act(messagen v, glown glow) const;
	bool apply(spelln v, bool run = true);
	void broke(messagen msg = (messagen)0);
	void clear() { count = 0; type = (itemn)0; }
	void create(int chance_blessed = 10, int chance_cursed = 5, int chance_power = 20);
	bool is(magicn v) const { return magic == v; }
	bool is(wearn v) const;
	bool is(featn v) const;
	bool iscoins() const { return type >= CP && type <= GP; }
	bool ismagical() const;
	bool ispower(variant v) const;
	void join(item& v);
	void setslot(item& v);
	bool setpower();
	bool setpower(variant v);
	bool unequip();
	bool use(bool run);
};
extern item* last_item;

struct wearable {
	item wears[Legs + 1];
	int	money;
	creature* owner() const;
	void additem(item& v, bool try_equip = false);
	void additem(const item& v) { item it = v; additem(it); }
	bool equip(item& v);
	bool equip(const item& v) { item it = v; return equip(it); }
	bool is(itemn v) const { for(auto& e : wears) if(e && e.type == v) return true; return false; }
	bool iswear(const void* p) const { return p >= wears && p <= wears + Legs; }
	item* getwear(wearn id) { return wears + id; }
	const item* getwear(const void* data) const;
	int totalweight() const;
};

item citem(itemn v);

itemn random(itemn v);

item* choose_backpack();
item* choose_backpack(wearn wear);
item* choose_ground();
item* choose_inventory();

int decoy_chance(itemn v);

void add_item(short unsigned area_index, short unsigned index, item& v);
void add_item(creature* p, item& v);
void drop_item(short unsigned index, item& v);
void drop_item(short unsigned index, itemn v);
void set_item_color(const item& it);
void update_items();