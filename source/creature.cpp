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
#include "collection.h"
#include "collectiona.h"
#include "creature.h"
#include "indexa.h"
#include "game.h"
#include "math.h"
#include "message.h"
#include "pushvalue.h"
#include "rand.h"
#include "speech.h"
#include "stringbuilder.h"

creature* human;
creature* player;
creature* opponent;
static int roll_result;

collectionv<creature> creatures, enemies;
bool need_update_creatures;
bool need_end_turn;

static collection allowed_spells;
static short unsigned compare_index;

void fix(messagen v) {
}

static int compare_distace(const void* v1, const void* v2) {
	auto p1 = (*((creature**)v1))->index;
	auto p2 = (*((creature**)v2))->index;
	auto d1 = area_range(p1, compare_index);
	auto d2 = area_range(p2, compare_index);
	return d1 - d2;
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
	player->hits_maximum = player->abilities[Hits];
	player->hits_maximum += player->abilities[Strenght] / 4;
}

static void update_abilities() {
	add_value(Armor, player->abilities[Strenght] / 15);
	add_value(Armor, player->wears[Torso].armor());
	add_value(Armor, player->wears[Backward].armor());
	add_value(Armor, player->wears[MeleeWeaponOffhand].armor());
	add_value(Armor, player->wears[Head].armor());
	add_value(Armor, player->wears[Elbows].armor());
	add_value(Armor, player->wears[Legs].armor());
	// Dodge bonuses
	add_value(Dodge, player->abilities[Dexterity] / 3);
	add_value(Dodge, player->wears[Torso].dodge());
	add_value(Dodge, player->wears[Elbows].dodge());
	add_value(Dodge, player->wears[Backward].dodge());
}

static void create_finish() {
	player->hits = player->hits_maximum;
	player->mana = player->abilities[Mana];
}

static void copy(statable& v1, const statable& v2) {
	v1 = v2;
}

static void update_player() {
	copy(*player, player->basic);
	update_abilities();
	update_derived();
}

static void apply_monster(monstern type) {
	player->basic.abilities[Strenght] += getv(type, Strenght);
	player->basic.abilities[Dexterity] += getv(type, Dexterity);
	player->basic.abilities[Wits] += getv(type, Wits);
	if(type >= FirstMonster) {
		player->basic.abilities[WeaponSkill] += getv(type, WeaponSkill);
		player->basic.abilities[BalisticSkill] += getv(type, BalisticSkill);
	} else {
		player->basic.abilities[WeaponSkill] += 15;
		player->basic.abilities[BalisticSkill] += 15;
		player->basic.abilities[Hits] += 10;
		player->basic.abilities[Mana] += 10;
	}
}

static speechn get_name_speech(monstern v) {
	switch(v) {
	case Elf: return ElfNames;
	case Dwarf: return DwarfNames;
	default: return HumanNames;
	}
}

static void random_name() {
	if(player->ischaracter()) {
		player->name_id = 2 * (rand() % 25);
		if(player->isfemale())
			player->name_id++;
	} else
		player->name_id = 0xFF;
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
	random_name();
	need_update_creatures = true;
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

static sitei* find_site(apos m) {
	for(auto& e : bsdata<sitei>()) {
		if(e.have(m))
			return &e;
	}
	return 0;
}

static void update_player_site() {
	auto pc = player->getsite();
	auto pn = find_site(player->index);
	if(pc == pn)
		return;
	if(pn) {
		player->site_id = pn - bsdata<sitei>::elements;
		// TODO: When enter site event
	} else
		player->site_id = 0xFFFF;
	player->update();
}

bool player_move(directionn d) {
	player->look(d);
	auto i1 = to(player->index, d);
	if(i1 == Blocked || !is_free(i1)) {
		player->fixact(d);
		if(i1 != Blocked) {
			if(use_area(i1))
				player->wait();
		}
		return false;
	}
	player->index = i1;
	player->waitmove();
	update_player_site();
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

void creature::update() {
	auto push = player; player = this;
	update_player();
	player = push;
}

static void nullify_elements(abilityn v1, abilityn v2) {
	if(player->is(v1) && player->is(v2)) {
		player->set(v1, 0);
		player->set(v2, 0);
	}
}

static void nullify_elements(featn v1, featn v2) {
	if(player->is(v1) && player->is(v2)) {
		player->remove(v1);
		player->remove(v2);
	}
}

static void nullify_elements() {
	nullify_elements(Burning, Freezing);
	//nullify_elements(FastAttack, SlowAttack);
	//nullify_elements(FastMove, SlowMove);
}

static void check_blooding() {
	if(player->is(Blooding)) {
		player->damage(1);
		area_set(player->index, Blooded);
		if(player->roll(Strenght))
			player->remove(Blooding);
	}
}

static void detect_hidden_objects() {
	if(!last_site)
		return;
	indexa source;
	source.select(player->index, imin(player->getlos(), 2));
	source.match(Hidden, true);
	source.shuffle();
	// Secrect doors
	for(auto i : source) {
		if(!player->roll(Alertness))
			continue;
		auto f = area_features[i];
		if(f==HiddenDoor)
			player->act(PlayerFoundSecretDoor);
		else if(is_trap(f))
			player->act(PlayerFoundTrap);
		area_remove(i, Hidden);
		break;
	}
}

static void check_burning() {
	if(player->is(Burning)) {
		if(!player->resist(FireResistance, FireImmunity))
			player->damage(xrand(1, 3));
		player->add(Burning, -1);
	}
}

static void damage_player_items(itemn v, int chance) {
}

static void check_freezing() {
	if(player->is(Freezing)) {
		if(!player->resist(ColdResistance, ColdImmunity)) {
			player->wait(100);
			damage_player_items(BluePotion, 30);
			damage_player_items(GreenPotion, 20);
			damage_player_items(RedPotion, 15);
		}
		player->add(Freezing, -1);
	}
}

static void ready_skills() {
}

static void select(collection& result, const spellable& known) {
	allowed_spells.clear();
	for(auto i = FirstSpell; i <= LastSpell; i = spelln(i + 1)) {
		if(known.is(i))
			allowed_spells.add(i);
	}
}

static bool is_enough_mana(unsigned char v) {
	auto mana = get_mana(spelln(v));
	return player->mana >= mana;
}

static bool is_enemy(const void* object) {
	return player->isenemy((creature*)object);
}

static void ready_spells() {
	select(allowed_spells, player->known);
	allowed_spells.match(is_enough_mana, true);
}

static void ready_actions() {
	compare_index = player->index;
	enemies = creatures;
	enemies.match(is_enemy, true);
	enemies.sort(compare_distace);
	if(enemies)
		opponent = *enemies.begin();
	else
		opponent = 0;
}

void make_move() {
	// Recoil form action
	if(player->wait_seconds > 0) {
		player->wait_seconds -= 25;
		return;
	}
	pushvalue push_player(opponent);
	pushvalue push_site(last_site, player->getsite());
	player->set(EnemyAttacks, 0);
	player->update();
	nullify_elements();
	check_blooding();
	if(!player->is(Local))
		detect_hidden_objects();
	check_burning();
	check_freezing();
	// check_corrosion();
	if(!player->operator bool())
		return; // Dead from blooding, burning, cold or other bad
	// check_levelup();
	ready_spells();
	ready_actions();
	// check_horror();
	if(player->ishuman()) {
		ready_skills();
//		last_actions.sort(compare_actions);
		auto last = player->wait_seconds;
		while(last==player->wait_seconds)
			choose_player_move();
	} else if(player->getfear()) {
		//if(!player->moveaway(player->getfear()->getposition()))
		//	pay_action();
	} else if(opponent) {
		//allowed_spells.select(player);
		//allowed_spells.match(spell_iscombat, true);
		//allowed_spells.match(spell_allowmana, true);
		//allowed_spells.match(spell_allowuse, true);
		//if(allowed_spells && d100() < 70)
		//	cast_spell(*((spelli*)allowed_spells.random()));
		//else if(can_shoot())
		//	attack_range(0);
		//else if(can_thrown() && d100() < 60)
		//	attack_thrown(0);
		//else if(d100() < 20 && use_items()) {
		//	// Nothing to do
		//} else
			player->moveto(opponent->index);
	} else {
		//allowed_spells.match(spell_isnotcombat, true);
		//allowed_spells.match(spell_allowmana, true);
		//allowed_spells.match(spell_allowuse, true);
		//if(player->isfollowmaster())
		//	player->moveto(player->getowner()->getposition());
		//else if(d100() < 20)
		//	use_skills();
		//else if(allowed_spells && d100() < 20)
		//	cast_spell(*((spelli*)allowed_spells.random()));
		//else if(area->isvalid(player->moveorder)) {
		//	if(player->moveorder == player->getposition())
		//		player->moveorder = {-1000, -1000};
		//	else if(!player->moveto(player->moveorder))
		//		player->moveorder = {-1000, -1000};
		//} else if(area->isvalid(player->guardorder)) {
		//	if(player->guardorder != player->getposition())
		//		player->moveorder = player->guardorder;
		//} else if(d100() < 20 && use_items()) {
		//	// Nothing to do
		//} else
		//	random_walk();
	}
}

void creature::clear() {
	memset((void*)this, 0, sizeof(*this));
	index = 0xFFFF;
	area_index = 0xFFFF;
	name_id = 0xFF;
	site_id = 0xFFFF;
	boss_id = 0xFFFF;
	fear_id = 0xFFFF;
	move_index = 0xFFFF;
}

void creature::setindex(short unsigned i) {
	player->index = i;
	player->area_index = current_area;
	player->position = i2s(i);
}

void creature::add(abilityn v, int i) {
	if(v < sizeof(abilities) / sizeof(abilities[0])) {
		i += abilities[v];
		if(i > 120)
			i = 120;
		else if(i < 0)
			i = 0;
		abilities[v] = (char)i;
	} else if(v == Experience)
		experience += i;
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
	return 4 - darkness;
}

bool creature::canhear(short unsigned i) const {
	auto n = 2 + get(Wits) / 10 + get(Dexterity) / 20;
	return area_range(index, i) < n;
}

const char* creature::name() const {
	if(name_id == 0xFF)
		return getname(type);
	return getspeech(get_name_speech(type), name_id);
}

creature* wearable::owner() {
	return bsdata<creature>::get(this);
}

creature* item::owner() {
	return bsdata<creature>::get(this);
}

creature* creature::getfear() const {
	if(fear_id == 0xFFFF)
		return 0;
	return bsdata<creature>::elements + fear_id;
}

creature* creature::getboss() const {
	if(boss_id == 0xFFFF)
		return 0;
	return bsdata<creature>::elements + boss_id;
}

sitei* creature::getsite() const {
	if(site_id == 0xFFFF)
		return 0;
	return bsdata<sitei>::elements + site_id;
}

void creature::act(messagen v) const {
	if(!ispresent() || !area_is(index, Visible))
		return;
}

void creature::waitmove() {
	auto modifier = get_move_cost(area_features[player->index]);
	auto base = 100 - player->get(Dexterity) / 10;
	if(base < 50)
		base = 50;
	auto result = base * 100 / base;
	wait(result);
}

bool creature::moveto(short unsigned ni) {
	clear_path();
	block_walls();
	block_features(false);
	block_creatures(this);
	make_wave(ni);
	auto d = move_lower(index, ni);
	if(d==Center)
		return false;
	pushvalue push(player, this);
	player_move(d);
	return true;
}

bool creature::moveaway(short unsigned ni) {
	clear_path();
	block_walls();
	block_features(false);
	block_creatures(this);
	make_wave(ni);
	auto d = move_greater(index, ni);
	if(d == Center)
		return false;
	pushvalue push(player, this);
	player_move(d);
	return true;
}