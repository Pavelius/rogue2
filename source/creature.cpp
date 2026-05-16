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
#include "collectiona.h"
#include "creature.h"
#include "math.h"
#include "message.h"
#include "rand.h"

creature* human;
creature* player;
creature* opponent;
static int roll_result;

collectionv<creature> creatures;
bool need_update_creatures;

void fix(messagen v) {
}

bool roll(int chance) {
	roll_result = d100();
	return roll_result < chance;
}

int damage_mode(abilityn mode) {
	switch(mode) {
	case BalisticSkill: return DamageRanged;
	default: return DamageMelee;
	}
}

static int get_bonus(magicn v) {
	return 0;
}

static void attack_roll(abilityn mode, item& weapon) {
	auto skill = player->get(mode);
	auto damage = player->get(DamageMelee) + weapon.damage();
	auto armor = opponent->get(Armor);
	if(!roll(skill))
		damage /= 2;
	if(damage<armor) {
		fix(MsgMiss);
		return;
	}
}

void add_value(abilityn v, int value) {
	value += player->abilities[v];
	if(value > 120)
		value = 120;
	else if(value < -120)
		value = -120;
	player->abilities[v] = (char)value;
}

static void update_derived() {
	player->hits_maximum += player->abilities[Hits];
	player->hits_maximum += player->abilities[Strenght] / 4;
}

static void update_abilities() {
	add_value(Armor, player->abilities[Strenght] / 15);
	add_value(Dodge, player->abilities[Dexterity] / 2);
}

static void create_finish() {
	player->hits = player->hits_maximum;
}

static void copy(statable& v1, const statable& v2) {
	v1 = v2;
}

void update_player() {
	copy(*player, player->basic);
	update_abilities();
	update_derived();
}

static void apply_monster(monstern type) {
	player->basic.abilities[Strenght] += getv(type, Strenght);
	player->basic.abilities[Dexterity] += getv(type, Dexterity);
	player->basic.abilities[Wits] += getv(type, Wits);
}

void creature::clear() {
	memset((void*)this, 0, sizeof(*this));
	index = 0xFFFF;
	area_index = 0xFFFF;
}

void creature::setindex(short unsigned i) {
	player->index = i;
	player->area_index = current_area;
	player->position = i2s(i);
}

void creature::look(directionn d) {
	switch(d) {
	case NorthWest:
	case West:
	case SouthWest:
		player->set(Mirrorred);
		break;
	case NorthEast:
	case East:
	case SouthEast:
		player->remove(Mirrorred);
		break;
	default:
		break;
	}
}

int creature::getlos() const {
	auto darkness = 0;
	if(is(DarkVision))
		darkness--;
	if(darkness > 4)
		darkness = 4;
	else if(darkness < 0)
		darkness = 0;
	return 5 - darkness;
}

bool creature::canhear(short unsigned i) const {
	auto n = 2 + get(Wits) / 10 + get(Dexterity) / 20;
	return area_range(index, i) < n;
}

void create_creature(short unsigned index_position, monstern type) {
	player = bsdata<creature>::add();
	player->clear();
	player->type = type;
	player->render = RenderCreature;
	player->priority = 10;
	player->setindex(index_position);
	apply_monster(type);
	update_player();
	create_finish();
	need_update_creatures = true;
}

creature* wearable::owner() {
	return bsdata<creature>::get(this);
}

creature* item::owner() {
	return bsdata<creature>::get(this);
}

void add_item(creature* p, item& v) {
	if(!p)
		return;
	add_item(0xFFFF, p - bsdata<creature>::elements, v);
}

void update_creatures() {
	if(!need_update_creatures)
		return;
	need_update_creatures = false;
	creatures.clear();
	for(auto& e : bsdata<creature>()) {
		if(e.ispresent())
			creatures.add(&e);
	}
}

static bool is_free(short unsigned i) {
	if(!is_free(area_tiles[i], player->is(WaterWalking)))
		return false;
	if(!is_free(area_features[i], false))
		return false;
	return true;
}

bool use_area(short unsigned i) {
	auto f = area_features[i];
	switch(f) {
	case Door:
		area_features[i] = OpenedDoor;
		break;
	case LockedDoor:
		break;
	default:
		return false;
	}
	return true;
}

bool player_move(directionn d) {
	player->look(d);
	auto i1 = to(player->index, d);
	if(i1 == Blocked || !is_free(i1)) {
		player->fixact(d);
		if(i1 != Blocked)
			use_area(i1);
		return true;
	}
	player->index = i1;
	return true;
}

static bool is_free_light_set(short unsigned i) {
	area_set(i, Visible);
	area_set(i, Explored);
	if(area_is(i, Darkened))
		return false;
	if(!is_free(area_tiles[i], true))
		return false;
	if(!is_free(area_features[i], false))
		return false;
	return true;
}

void update_los() {
	for(short unsigned i = 0; i < mps * mps; i++) {
		area_remove(i, Visible);
		if(area_is(i, Darkened))
			area_remove(i, Explored);
	}
	if(human)
		area_los(human->index, player->getlos(), is_free_light_set);
}