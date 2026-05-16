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
#include "collectiona.h"
#include "itemlay.h"
#include "math.h"
#include "rand.h"
#include "slice.h"

const int cp = 1;
const int sp = 10;
const int gp = 100;

const int lb = 50;

static_assert(sizeof(item) == 4, "Structure `item` must 4 bytes");

collectiona items;
bool need_update_items;

static int get_weight(itemn v) {
	switch(v) {
	case LongSword: return 3 * lb;
	case ShortSword: return 2 * lb;
	case GreatAxe: return 7 * lb;
	case GreatSword: return 6 * lb;
	default: return 1 * lb;
	}
}

static int get_cost(itemn v) {
	switch(v) {
	case CP: return cp;
	case SP: return sp;
	case GP: return gp;
	case Axe: return 5 * gp;
	case GreatAxe: return 30 * gp;
	case Dagger: return 2 * gp;
	case ShortSword: return 10 * gp;
	case LongSword: return 15 * gp;
	case Scimitar: return 25 * gp;
	case GreatSword: return 50 * gp;
	default: return 0;
	}
}

static int get_damage(itemn v) {
	switch(v) {
	case Dagger:
		return 3;
	case ShortSword: case Axe: case Spear: case Mace:
		return 4;
	case Scimitar: case WarHammer:
		return 5;
	case LongSword:
		return 6;
	case GreatAxe:
		return 8;
	case GreatSword:
		return 9;
	default:
		return 0;
	}
}

static wearn get_wear(itemn v) {
	if(v >= CP && v <= GP)
		return Backpack;
	else if(v >= Staff && v <= GreatSword)
		return MeleeWeapon;
	else if(v >= ShortBow && v <= HeavyCrossbow)
		return RangedWeapon;
	else if(v >= Robe && v <= PlateMail)
		return Torso;
	return Backpack;
}

int item::weight() const {
	return get_weight(type) * count;
}

int item::cost() const {
	return get_cost(type) * count;
}

int item::damage() const {
	return get_damage(type);
}

bool item::istwohanded() const {
	switch(type) {
	case GreatAxe:
	case GreatSword:
	case Staff:
		return true;
	default:
		return false;
	}
}

void item::join(item& v) {
	if(!count) {
		*this = v;
		v.clear();
		need_update_items = true;
	} else {
		if(type != v.type || properties != v.properties)
			return;
		if(count >= 255)
			return;
		int new_count = count + v.count;
		if(new_count > 255) {
			v.count = new_count - 255;
			count = 255;
		} else {
			count = (unsigned char)new_count;
			v.count = 0;
		}
		need_update_items = true;
	}
}

bool wearable::equip(item& it) {
	auto w = get_wear(it.type);
	if(w == Backpack || wears[w])
		return false;
	wears[w] = it;
	if(w == Ammunition)
		it.clear();
	else
		it.count--;
	return true;
}

void wearable::additem(item& it, bool try_equip) {
	if(it.iscoins()) {
		money += it.cost();
		it.clear();
		return;
	}
	if(try_equip && equip(it))
		return;
	add_item(owner(), it);
}

void add_item(short unsigned area_index, short unsigned index, item& v) {
	for(auto& e : bsdata<itemlay>()) {
		if(e.area_index == area_index && e.index == index)
			e.join(v);
	}
	if(!v)
		return;
	auto p = bsdata<itemlay>::addz();
	p->area_index = area_index;
	p->index = index;
	p->join(v);
}

void update_items() {
	if(!need_update_items)
		return;
	need_update_items = false;
	items.clear();
	for(auto& e : bsdata<itemlay>()) {
		if(e.ispresent())
			items.add(&e);
	}
}
