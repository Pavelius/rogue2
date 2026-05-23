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

enum wearn : unsigned char {
	MeleeWeapon, MeleeWeaponOffhand, RangedWeapon, Ammunition,
	Torso, Head, Neck, Backward, Girdle, Gloves, FingerRight, FingerLeft, Elbows, Legs,
	Backpack
};
enum magicn : unsigned char {
	Mundane, Cursed, Blessed, Artifact,
};
enum itemn : unsigned char {
	CP, SP, GP,
	Staff, Spear, Axe, Mace, WarHammer, GreatMace, GreatAxe,
	Dagger, ShortSword, LongSword, Scimitar, GreatSword,
	Claws,
	ShortBow, LongBow, Crossbow, HeavyCrossbow,
	Robe, LeatherArmor, StuddedArmor, HideArmor, ScaleMail, ChainMail, PlateMail,
	BluePotion, GreenPotion, RedPotion,
	LastItem = RedPotion,
};

int get_pierce(itemn v);

wearn get_wear(itemn v);

bool is_feat(itemn type, featn v);

struct item {
	itemn type;
	unsigned char count;
	union {
		unsigned short properties;
		struct {
			unsigned char power : 5; // Item magical power index (1-31) or 0 - if no magical power
			unsigned char broken : 3; // Charges or Broken status
			magicn magic : 2;
			unsigned char known_power : 1;
			unsigned char known_magic : 1;
		};
	};
	constexpr item() : type((itemn)0), count(0), properties(0) {}
	constexpr item(itemn v, unsigned char count = 1) : type(v), count(count), properties(0) {}
	explicit operator bool() const { return count != 0; }
	creature* owner() const;
	wearn equiped() const;
	const char* name() const;
	const char* fullname() const;
	variant getpower() const;
	int	armor() const;
	int	cost() const;
	int damage() const;
	int dodge() const;
	int speed() const;
	int	weight() const;
	bool broke();
	void clear() { count = 0; type = (itemn)0; properties = 0; }
	bool is(magicn v) const { return magic == v; }
	bool is(wearn v) const;
	bool is(featn v) const { return is_feat(type, v); }
	bool iscoins() const { return type == CP || type == SP || type == GP; }
	bool ismagical() const;
	bool istwohanded() const;
	void join(item& v);
	void setslot(item& v);
	bool unequip();
	void use();
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
};

item* choose_backpack();
item* choose_backpack(wearn wear);
item* choose_ground();
item* choose_inventory();

void add_item(short unsigned area_index, short unsigned index, item& v);
void add_item(creature* p, item& v);
void set_item_color(const item& it);
void update_items();