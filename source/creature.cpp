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
#include "draw_effect.h"
#include "draw_floatinfo.h"
#include "enchant.h"
#include "itemlay.h"
#include "indexa.h"
#include "game.h"
#include "math.h"
#include "message.h"
#include "print.h"
#include "pushvalue.h"
#include "rand.h"
#include "stringbuilder.h"

creature* human;
creature* player;
creature* opponent;
static int roll_result, last_value;

collectionv<creature> creatures, enemies;
bool need_update_creatures;
bool need_end_turn;

static collection allowed_spells;
static short unsigned compare_index;

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
	player->hits_maximum += player->abilities[Level] * 5;
}

static void update_abilities() {
	if(player->is(Stun)) {
		add_value(Dexterity, -10);
		add_value(WeaponSkill, -10);
		add_value(BalisticSkill, -10);
	}
	// Armor
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

static void apply_boost(variant v) {
	switch(v.type) {
	case Ability: player->add((abilityn)v.value, 5); break;
	case Feat: player->set((featn)v.value); break;
	case Spell: player->spellable::set((spelln)v.value); break;
	default: break;
	}
}

static void update_boost_effect() {
	short unsigned parent = player->bsi();
	for(auto& e : bsdata<enchanti>()) {
		if(e && e.parent == parent)
			apply_boost(e.value);
	}
}

void update_player() {
	copy(*player, player->basic);
	update_abilities();
	update_derived();
}

static void random_name() {
	if(player->ischaracter()) {
		player->custom_name = namen(2 * (rand() % 25));
		if(player->isfemale())
			player->custom_name = namen(player->custom_name + 1);
	} else
		player->custom_name = NoName;
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

creature* find_creature(short unsigned i) {
	update_creatures();
	for(auto p : creatures) {
		if(p->index == i)
			return p;
	}
	return 0;
}

bool is_free(short unsigned i) {
	if(!is_free(area_tiles[i], player->is(Fly)))
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

static void add_hits(creature* player, int value) {
	if(value < 0)
		player->damage(-value);
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

static void pay_action() {
	player->wait(100);
}

static void pay_movement() {
	auto cost = 150 - player->get(Dexterity);
	if(!player->is(Fly))
		cost = cost * get_move_cost(area_features[player->index]) / 100;
	if(player->is(FastMove))
		cost /= 2;
	else if(player->is(SlowMove))
		cost *= 2;
	if(cost < 30)
		cost = 30;
	player->wait(cost);
}

static void pay_attack(const item& weapon) {
	auto cost = 150 - weapon.speed() * 2 - player->get(Dexterity) / 10;
	if(player->is(FastAttack))
		cost /= 2;
	else if(player->is(SlowAttack))
		cost *= 2;
	if(cost < 30)
		cost = 30;
	player->wait(cost);
}

static void poison_attack(creature* player, int value) {
	if(value <= 0)
		return;
	if(player->resist(PoisonResistance, PoisonImmunity))
		return;
	if(player->roll(Strenght, -value))
		return;
	auto v = player->get(Poison) + value;
	player->fixact(PoisonVisual);
	if(v >= player->basic.abilities[Hits])
		player->kill();
	else
		player->set(Poison, v);
}

static void poison_attack(const item& weapon) {
	auto strenght = 0;
	if(player->is(WeakPoison, weapon))
		strenght += xrand(1, 2);
	if(player->is(StrongPoison, weapon))
		strenght += xrand(2, 4);
	if(player->is(DeathPoison, weapon))
		strenght += xrand(3, 6);
	poison_attack(opponent, strenght);
}

static void illness_attack(creature* player, int value) {
	if(value <= 0)
		return;
	if(player->resist(DiseaseResist, DiseaseImmunity))
		return;
	auto v = player->get(Illness) + value;
	player->fixact(PoisonVisual);
	if(v >= player->get(Strenght))
		player->kill();
	else
		player->set(Illness, v);
}

static void damage_player_items(itemn v, int chance) {
}

static void damage_equipment(int value, bool run) {
}

static void check_corrosion() {
	if(player->is(Corrosion)) {
		if(!player->resist(AcidResistance, AcidImmunity)) {
			player->damage(player->get(Corrosion));
			damage_equipment(1, true);
		}
		player->add(Corrosion, -1);
	}
}

static abilityn weapon_skill(const item& weapon) {
	return WeaponSkill;
}

static abilityn damage_skill(abilityn v) {
	switch(v) {
	case BalisticSkill: return DamageRanged;
	default: return DamageMelee;
	}
}

static void attack_effect_stun(creature* opponent) {
	if(!opponent->resist(StunResistance, StunImmunity))
		opponent->set(Stun);
}

static void special_attack(item& weapon, creature* opponent, int& pierce, int& damage) {
	opponent->fixact(BloodVisual);
	if(player->is(VorpalHit, weapon)) {
		if(!opponent->resist(DeathResistance, DeathImmunity)) {
			damage = 100;
			pierce = 100;
		}
	}
	if(player->is(BleedingHit, weapon))
		opponent->set(Blooding);
	if(player->is(StunningHit, weapon))
		attack_effect_stun(opponent);
	if(player->is(PierceHit, weapon))
		pierce += 3;
	if(player->is(MightyHit, weapon))
		damage += 2;
	if(player->is(ColdDamage, weapon)) {
		opponent->add(Freezing, 2);
		area_set(opponent->index, Iced);
	}
}

static void apply_pierce(int& armor, int pierce) {
	if(armor > 0) {
		if(pierce > armor)
			armor = 0;
		else
			armor -= pierce;
	} else
		armor = 0;
}

static int add_bonus_damage(creature* player, creature* enemy, const item& weapon, featn feat, int value, featn resistance, featn immunity) {
	if(!player->is(feat, weapon))
		return 0;
	auto bonus_damage = value;
	if(immunity && enemy->is(immunity))
		bonus_damage = 0;
	else if(resistance && enemy->is(resistance))
		bonus_damage /= 2;
	return bonus_damage;
}

static void make_attack(item& weapon, int attack_skill, int damage_percent) {
	if(!opponent)
		return;
	auto weapon_ability = weapon_skill(weapon);
	auto weapon_damage = weapon.damage();
	auto damage = weapon_damage;
	damage += player->get(damage_skill(weapon_ability));
	damage += add_bonus_damage(player, opponent, weapon, FireDamage, 2, FireResistance, FireImmunity);
	damage += add_bonus_damage(player, opponent, weapon, ColdDamage, 2, ColdResistance, ColdImmunity);
	attack_skill += player->get(weapon_ability);
	// Roll how match damage make
	auto roll_result = d100();
	if(roll_result <= attack_skill)
		damage += xrand(0, 2); // Hit, gain random damage increment
	else
		damage -= xrand(1, 4); // Miss gain great damage reduction
	if(roll_result <= attack_skill - 30)
		damage += xrand(weapon_damage / 2, weapon_damage);
	if(roll_result <= attack_skill - 60)
		damage += xrand(weapon_damage / 2, weapon_damage);
	if(damage_percent)
		damage = damage * damage_percent / 100;
	auto armor = opponent->get(Armor);
	auto pierce = get_pierce(weapon.type);
	if(player->roll(Dexterity))
		special_attack(weapon, opponent, pierce, damage); // If hit critical
	apply_pierce(armor, pierce);
	auto block_damage = opponent->get(Block);
	if(block_damage > 0 && armor < damage) {
		armor += xrand(0, block_damage);
		if(armor >= damage) {
			//player->logs("AttackBlocked", damage - armor, opponent->getname(), roll_result, damage, -armor);
			opponent->fixmsg(getname(PlayerBlock), InfoGreen);
			return;
		}
	}
	auto damage_result = damage - armor;
	if(damage_result > 0 && weapon.is(Cursed) && (d100() < 50)) // Cursed weapon miss half time
		damage_result = 0;
	if(damage_result <= 0) {
		// player->logs("AttackMiss", damage_result, opponent->getname(), roll_result, damage, -armor);
		return;
	}
	if(opponent->roll(Dodge)) {
		// player->logs("AttackHitButEnemyDodge", opponent->getname());
		opponent->fixmsg(getname(PlayerDodge), InfoGreen);
	} else {
		// player->logs("AttackHit", damage_result, opponent->getname(), roll_result, damage, -armor);
		opponent->damage(damage_result);
		poison_attack(weapon);
		if(opponent->is(RetaliateHit, opponent->wears[MeleeWeapon])) {
			if(!player->roll(Dodge))
				player->damage(1);
		}
	}
	if(roll_result >= 95 && d100() < 30)
		weapon.broke();
}

static bool player_interact(short unsigned i, directionn d) {
	auto p = find_creature(i);
	if(!p)
		return false;
	pushvalue push(opponent, p);
	if(opponent->isenemy(player)) {
		player->fixact(d);
		make_attack(player->wears[MeleeWeapon], 0, 100);
		pay_attack(player->wears[MeleeWeapon]);
	}
	return true;
}

bool player_move(directionn d) {
	player->look(d);
	auto i1 = to(player->index, d);
	if(player_interact(i1, d))
		return true;
	if(i1 == Blocked || !is_free(i1)) {
		player->fixact(d);
		if(use_area(i1))
			player->wait();
		return false;
	}
	player->index = i1;
	pay_movement();
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
	nullify_elements(FastAttack, SlowAttack);
	nullify_elements(FastMove, SlowMove);
}

static void check_blooding() {
	if(player->is(Blooding)) {
		player->damage(1);
		area_set(player->index, Blooded);
		if(player->roll(Strenght))
			player->remove(Blooding);
	}
}

static void check_stun() {
	if(player->is(Stun)) {
		if(player->roll(Strenght))
			player->remove(Stun);
		if(player->is(StunResistance) && player->roll(Strenght))
			player->remove(Stun);
	}
}

static void check_illness_effect() {
	if(!player->is(Illness))
		return;
	auto minimal_hp = player->hits_maximum / 3;
	if(player->abilities[Hits] > minimal_hp)
		add_hits(player, -1);
}

static void check_illness_cure() {
	if(player->abilities[Illness] > 0) {
		if(player->resist(DiseaseResist, DiseaseImmunity) || player->roll(Strenght))
			player->abilities[Illness]--;
		else if(d100() < 20)
			illness_attack(player, 1); // Disease have 20% chance to progress
	}
}

static void check_recovery(short& result, short maximum, abilityn v) {
	if(result < maximum) {
		if(player->roll(v, 10))
			result++;
	}
}

static void check_recovery(short& result, short maximum) {
	if(result < maximum)
		result++;
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
		if(f == HiddenDoor)
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
		while(last == player->wait_seconds)
			choose_player_move();
	} else if(player->getfear()) {
		auto master = player->getfear();
		if(!player->moveaway(master->index))
			pay_action();
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

void creature_every_minute() {
	check_stun();
	check_recovery(player->mana, player->abilities[Mana], Wits);
	if(player->is(Boosting))
		check_recovery(player->mana, player->abilities[Mana]);
	if(player->is(Regenerating))
		check_recovery(player->hits, player->hits_maximum);
}

void creature_every_10_minutes() {
	check_illness_effect();
	check_illness_cure();
	check_recovery(player->hits, player->hits_maximum, Strenght);
}

static int get_experience_reward(const creature* player) {
	static int rewards[] = {
		5, 10, 15, 20, 25, 30, 35, 50, 75, 125,
		175, 225, 275, 350, 450, 650, 900, 1100, 1350, 2000,
		2500};
	auto level = player->get(Level);
	return maptbl(rewards, level);
}

static bool drink_effect(abilityn v, magicn magic, bool run) {
	int multiply = 1;
	switch(magic) {
	case Artifact:
		if(run)
			player->basic.add(v, 1);
		break;
	case Cursed:
		if(run)
			player->basic.add(v, -1);
		break;
	default:
		if(magic == Blessed)
			multiply = 4;
		switch(v) {
		case Hits:
			if(player->hits >= player->hits_maximum)
				return false;
			if(run) {
				player->hits += xrand(3, 12) * multiply;
				if(player->hits > player->hits_maximum)
					player->hits = player->hits_maximum;
			}
			break;
		case Mana:
			if(player->mana >= player->get(Mana))
				return false;
			if(run) {
				player->mana += xrand(3, 18) * multiply;
				if(player->mana > player->get(Mana))
					player->mana = player->get(Mana);
			}
			break;
		default:
			add_enchant(player->bsi(), v, game.minutes + xrand(60, 120) * multiply);
			break;
		}
		break;
	}
	return true;
}

static bool drink_effect(featn v, magicn magic, bool run) {
	if(player->is(v))
		return false;
	unsigned duration = xrand(60, 120);
	switch(magic) {
	case Artifact: duration = duration * 10; break;
	case Cursed: break;
	case Blessed: duration = duration * 3; break;
	default: break;
	}
	add_enchant(player->bsi(), v, game.minutes + duration);
	return true;
}

static bool drink_potion(item& v, bool run) {
	auto power = v.getpower();
	switch(power.type) {
	case Ability: return drink_effect((abilityn)power.value, v.magic, run);
	case Feat: return drink_effect((featn)power.value, v.magic, run);
	default: return false;
	}
}

void creature::clear() {
	memset((void*)this, 0, sizeof(*this));
	index = 0xFFFF;
	area_index = 0xFFFF;
	custom_name = NoName;
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

void creature::update() {
	auto push = player; player = this;
	update_player();
	player = push;
}

void creature::kill() {
	need_update_creatures = true;
	if(d100() < 40)
		area_set(index, Blooded);
	// logs("ApplyKill");
	auto human_killed = ishuman();
	remove_order(this);
	fixact(BloodVisual);
	//	drop_treasure(this);
	//	drop_throphy(this, 30);
	if(opponent == this && player)
		player->experience += get_experience_reward(opponent);
	clear();
	if(human_killed)
		end_game();
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

bool creature::resist(featn resist, featn immunity) const {
	if(!resist)
		return false;
	if(is(immunity))
		return true;
	if(is(resist)) {
		if(d100() < 50)
			return true;
	}
	return false;
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
	if(custom_name == NoName)
		return getname(type);
	return getname(custom_name);
}

creature* wearable::owner() const {
	return bsdata<creature>::get(this);
}

creature* item::owner() const {
	auto p = bsdata<creature>::get(this);
	if(p)
		return p;
	auto pn = bsdata<itemlay>::get(this);
	if(pn->area_index == 0xFFFF)
		return bsdata<creature>::elements + pn->index;
	return 0;
}

wearn item::equiped() const {
	auto p = owner();
	if(!p)
		return Backpack;
	if(p->iswear(this))
		return wearn(this - p->wears);
	return Backpack;
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

bool creature::isvisible() const {
	return area_is(index, Visible);
}

void creature::act(messagen v) const {
	if(!ispresent() || !area_is(index, Visible))
		return;
	print(getname(v));
}

void creature::actn(messagen v) const {
	if(!ispresent() || !area_is(index, Visible))
		return;
	println(getname(v));
}

bool creature::moveto(short unsigned ni) {
	clear_path();
	block_tiles(is(Fly));
	block_features(false);
	block_creatures(this);
	make_wave(ni, index);
	auto d = move_lower(index, ni);
	if(d == Center)
		return false;
	pushvalue push(player, this);
	player_move(d);
	return true;
}

bool creature::moveaway(short unsigned ni) {
	clear_path();
	block_tiles(is(Fly));
	block_features(false);
	block_creatures(this);
	make_wave(ni, index);
	auto d = move_greater(index, ni);
	if(d == Center)
		return false;
	pushvalue push(player, this);
	player_move(d);
	return true;
}

bool creature::roll(abilityn v, int bonus) const {
	last_value = get(v) + bonus;
	roll_result = d100();
	return roll_result < last_value;
}

void creature::damage(int v) {
	if(v <= 0)
		return;
	fixmsg("-%1i", v);
	hits -= v;
	if(hits <= 0)
		kill();
}

void creature::say(speechn v) const {
	if(!isvisible())
		return;
	auto n = speechn(v * 3 + rand() % 3);
	println("[%1:] \"%2\"", name(), getname(n));
}

bool creature::use(item& it, bool run) {
	pushvalue push(player, this);
	switch(it.type) {
	case RedPotion: case BluePotion: case GreenPotion:
		return drink_potion(it, run);
	default:
		return false;
	}
}

void creature::enchant(spelln spell, unsigned duration) {
	add_enchant(bsi(), spell, game.minutes + duration);
	update();
}

short unsigned creature::bsi() const {
	return this - bsdata<creature>::elements;
}