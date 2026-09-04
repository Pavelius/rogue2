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
#include "collectiona.h"
#include "itemlay.h"
#include "game.h"
#include "math.h"
#include "message.h"
#include "pushvalue.h"
#include "rand.h"
#include "slice.h"
#include "stringbuilder.h"
#include "variant.h"

const int cp = 1;
const int sp = 10;
const int gp = 100;

const int lb = 50;

const int mp = 8;

static_assert(sizeof(item) == 2, "Structure `item` must 2 bytes");

collectionv<itemlay> items;
bool need_update_items;
item* last_item;

static variant no_powers[mp] = {Variant};
static variant swords_powers[mp] = {Variant, WeaponSkill, DamageMelee, Dexterity};
static variant pierce_melee_weapon_powers[mp] = {Variant, WeaponSkill, DamageMelee, Dexterity};
static variant blue_potion_powers[mp] = {Hits, Mana, Strenght, Dexterity, Wits, Poison, Illness, Drunk};
static variant red_potion_powers[mp] = {WeaponSkill, BalisticSkill, Dodge, Armor, FastMove, FastAttack, Fly, FireImmunity};
static variant green_potion_powers[mp] = {Regenerating, Boosting, Experience, AcidImmunity, ColdImmunity, DeathImmunity, DiseaseImmunity, PoisonImmunity};
static variant red_tome[mp] = {Alchemy, Gemcutting, BalisticSkill, WeaponSkill, Mining, Stealth, History, Religion};
static variant green_tome[mp] = {Mana, Hits, Wits, Dexterity, Strenght, Literacy, Metallurgy};
static variant blue_tome[mp] = {CureLightWounds, MageArmor, LightSpell, Mending, Sleep, Web, BurningHands, HorrorScare};
static variant blue_rod[mp] = {CureSeriousWounds, CureDisease, CurePoison, IceSpear, HorrorScare, LightSpell, SummonUndead};
static variant green_rod[mp] = {Sleep, Entaglement, MageArmor, Web};
static variant red_rod[mp] = {BurningHands, FireBolt};

static itemn random_coins[] = {CP, CP, CP, CP, SP, SP, SP, GP};
static itemn random_gems[] = {BlueGem, BlueGem, BlueGem, WhiteGem, WhiteGem, YellowGem, YellowGem, GreenGem, RedGem};
static itemn random_treasure[] = {RandomCoins, RandomCoins, RandomCoins, RandomCoins, RandomCoins, RandomCoins, RandomCoins, RandomCoins, RandomCoins, RandomGems};
static itemn random_small_blades[] = {Dagger, Dagger, ShortSword, Scimitar};
static itemn random_blades[] = {ShortSword, LongSword, GreatSword};
static itemn random_martial_weapons[] = {Staff, Spear, Spear, Axe, Axe, Mace, Mace, WarHammer, WarHammer, GreatMace, GreatAxe};
static itemn random_missile_weapons[] = {ShortBow, ShortBow, LongBow, Crossbow, HeavyCrossbow};
static itemn random_armors[] = {Robe, LeatherArmor, LeatherArmor, LeatherArmor, StuddedArmor, StuddedArmor, HideArmor, HideArmor, ScaleMail, ChainMail, PlateMail};
static itemn random_weapon[] = {RandomBladesSmall, RandomBlades, RandomMartialWeapon, RandomMartialWeapon, RandomMissileWeapon};
static itemn random_food[] = {Meat, Ration, Bread};

itemi item_data[LastItem + 1] = {
	{Backpack, 0, 0, "item-1", {}, {0, 2}},
	{MeleeWeapon, 4 * lb, 2 * gp, "item6", {StunningHit, Large}, {0, 5, 7}, swords_powers}, // Staff
	{MeleeWeapon, 4 * lb, 2 * gp, "item60", {PierceHit, RetaliateHit, Large}, {0, 6, 7}, pierce_melee_weapon_powers}, // Spear
	{MeleeWeapon, 2 * lb, 5 * gp, "item3", {MightyHit}, {0, 6, 7}, swords_powers}, // Axe
	{MeleeWeapon, 5 * lb, 1 * gp, "item7", {StunningHit}, {0, 6, 7}, swords_powers}, // Mace
	{MeleeWeapon, 5 * lb, 4 * gp, "item42", {StunningHit}, {0, 7, 7}, swords_powers}, // WarHammer
	{MeleeWeapon, 10 * lb, 20 * gp, "item24", {StunningHit, Large}, {-1, 9, 10}, swords_powers}, // GreatMace
	{MeleeWeapon, 7 * lb, 30 * gp, "item189", {MightyHit, Large}, {-1, 9, 10}, swords_powers}, // GreatAxe
	{MeleeWeapon, 1 * lb, 5 * gp, "item0", {PierceHit}, {0, 4, 2}, swords_powers}, // Dagger
	{MeleeWeapon, 2 * lb, 5 * gp, "item2", {}, {0, 6, 6}, swords_powers}, // ShortSword
	{MeleeWeapon, 3 * lb, 5 * gp, "item4", {}, {0, 8, 7}, swords_powers}, // LongSword
	{MeleeWeapon, 2 * lb, 6 * gp, "item36", {}, {0, 7, 7}, swords_powers}, // Scimitar
	{MeleeWeapon, 10 * lb, 40 * gp, "item190", {BleedingHit, Large}, {-1, 10, 10}, swords_powers}, // GreatSword
	{RangedWeapon, 1 * lb, 10 * gp, "item50", {Large}, {0, 6, 0, 0, 0, Arrow}, pierce_melee_weapon_powers}, // ShortBow
	{RangedWeapon, 2 * lb, 20 * gp, "item76", {Large}, {0, 7, 0, 0, 0, Arrow}, pierce_melee_weapon_powers}, // LongBow
	{RangedWeapon, 3 * lb, 20 * gp, "item67", {Large}, {0, 7, 0, 0, 0, Bolt}, pierce_melee_weapon_powers}, // Crossbow
	{RangedWeapon, 5 * lb, 30 * gp, "item77", {Large}, {0, 10, 0, 0, 0, Bolt}, pierce_melee_weapon_powers}, // HeavyCrossbow
	{Torso, 10 * lb, 5 * gp, "item8", {}, {0, 0, 0, 0, 0}}, // Robe
	{Torso, 15 * lb, 10 * gp, "item29", {}, {0, 0, 0, 1, 0}}, // LeatherArmor
	{Torso, 25 * lb, 15 * gp, "item43", {}, {0, 0, -1, 1, 1}}, // StuddedArmor
	{Torso, 30 * lb, 30 * gp, "item10", {}, {0, 0, -2, 2, 0}}, // HideArmor
	{Torso, 45 * lb, 70 * gp, "item11", {}, {0, 0, -4, 3, 1}}, // ScaleMail
	{Torso, 40 * lb, 100 * gp, "item12", {}, {0, 0, -3, 3, 0}}, // ChainMail
	{Torso, 50 * lb, 300 * gp, "item13", {}, {0, 0, -6, 4, 0}}, // PlateMail
	{MeleeWeaponOffhand, 5 * lb, 10 * gp, "item16", {}, {0, 0, 0, 0, 1}}, // Shield small
	{MeleeWeaponOffhand, 10 * lb, 10 * gp, "item17", {}, {0, 0, 0, 0, 2}},
	{MeleeWeaponOffhand, 15 * lb, 10 * gp, "item18", {}, {0, 0, 0, 1, 1}},
	{MeleeWeaponOffhand, 20 * lb, 10 * gp, "item19", {}, {0, 0, -1, 1, 2}},
	{Legs, 3 * lb, 10 * gp, "item40", {}, {0, 0, 0, 0, 1}}, // LeatherBoots
	{Legs, 7 * lb, 10 * gp, "item58", {}, {0, 0, 0, 1, 0}},
	{Backpack, 1 * lb, 10 * gp, "item488", {}, {}, blue_potion_powers}, // Blue Potions
	{Backpack, 1 * lb, 10 * gp, "item482", {}, {}, green_potion_powers},
	{Backpack, 1 * lb, 10 * gp, "item567", {}, {}, red_potion_powers},
	{Backpack, 2 * lb, 10 * gp, "item682", {}, {}, blue_tome}, // Blue tome
	{Backpack, 3 * lb, 10 * gp, "item450", {}, {}, green_tome},
	{Backpack, 3 * lb, 10 * gp, "item88", {}, {}, red_tome},
	{Backpack, 1 * lb, 10 * gp, "item230", {}, {}, blue_rod}, // Blue rod
	{Backpack, 1 * lb, 10 * gp, "item364", {}, {}, green_rod},
	{Backpack, 1 * lb, 10 * gp, "item231", {}, {}, red_rod},
	// Countable
	{Backpack, 1 * lb, 10 * gp, "item21"}, // Rations
	{Backpack, 1 * lb, 10 * gp, "item55"},
	{Backpack, 1 * lb, 10 * gp, "item240"},
	{Backpack, 1 * lb, 10 * gp, "item22"},
	{Backpack, 1 * lb, 10 * gp, "item258"},
	{Backpack, 1 * lb, 10 * gp, "items383"},
	{Backpack, 1 * lb, 10 * gp, "item103"},
	{Ammunition, 1 * lb, 10 * gp, "item51"}, // Arrow
	{Ammunition, 1 * lb, 10 * gp, "item34"},
	{Backpack, 1 * lb, 10 * gp, "item404"}, // Blue gem
	{Backpack, 1 * lb, 10 * gp, "item402"},
	{Backpack, 1 * lb, 10 * gp, "item406"},
	{Backpack, 1 * lb, 10 * gp, "item405"},
	{Backpack, 1 * lb, 10 * gp, "item403"},
	{Backpack, 1 * lb, 10 * gp, "items37"}, // CP
	{Backpack, 1 * lb, 10 * gp, "items37"},
	{Backpack, 1 * lb, 10 * gp, "items37"},
};

itemn random(itemn v) {
	switch(v) {
	case RandomArmor: return random(maprnd(random_armors));
	case RandomBlades: return random(maprnd(random_blades));
	case RandomBladesSmall: return random(maprnd(random_small_blades));
	case RandomCoins: return random(maprnd(random_coins));
	case RandomGems: return random(maprnd(random_gems));
	case RandomMartialWeapon: return random(maprnd(random_martial_weapons));
	case RandomMissileWeapon: return random(maprnd(random_missile_weapons));
	case RandomTreasure: return random(maprnd(random_treasure));
	case RandomWeapon: return random(maprnd(random_weapon));
	default: return v;
	}
}

static int get_power_count(const variant* source) {
	if(!source)
		return 0;
	for(auto i = 0; i < 8; i++) {
		if(!source[i])
			return i;
	}
	return 8;
}

variant item::getpower() const {
	if(countable())
		return Variant;
	auto pv = powers();
	if(!pv)
		return variant();
	return pv[power];
}

static int find_power(const variant* source, variant v) {
	for(auto i = 0; i < 8; i++) {
		if(source[i] == v)
			return i;
	}
	return -1;
}

bool item::setpower(variant v) {
	if(countable())
		return false;
	auto n = find_power(powers(), v);
	if(n == -1)
		return false;
	power = n;
	return true;
}

bool item::setpower() {
	if(countable())
		return false;
	auto source = powers();
	if(!source)
		return false;
	auto is_magical = source[0].operator bool();
	if(!is_magical) {
		// If first item is empthy power, then this is item, that can be powerless.
		// Exclude this case.
		auto count = get_power_count(source + 1);
		if(!count)
			return false;
		power = 1 + rand() % count;
	} else
		power = rand() % get_power_count(source);
	return true;
}

bool item::ispower(variant v) const {
	if(!v)
		return false;
	return getpower() == v;
}

static itemn get_ammo(itemn v) {
	switch(v) {
	case LongBow: case ShortBow: return Arrow;
	case Crossbow: return Bolt;
	default: return (itemn)0;
	}
}

int decoy_chance(itemn v) {
	switch(v) {
	case Apple: return 5;
	case BloodyRemains: return 30;
	case Meat: return 20;
	default: return 0;
	}
}

/*
static int get_weapon_speed(itemn v) {
	switch(v) {
	case LongSword: return 7;
	case ShortSword: return 8;
	case Dagger: return 9;
	case GreatAxe: return 1;
	case GreatSword: return 1;
	case Mace: return 6;
	case Staff: return 5;
	case Axe: return 6;
	default: return 10;
	}
}*/

bool item::ismagical() const {
	// Item is magical if is not mudane, if power filled and if power zero and no empty power.
	if(magic != Mundane)
		return true;
	if(power)
		return true;
	auto source = powers();
	if(!source)
		return false;
	return source[0].u != 0;
}

bool item::is(wearn v) const {
	switch(v) {
	case FingerLeft: case FingerRight:
		return wear() == FingerRight;
	default:
		return wear() == v;
	}
}

void item::broke(messagen msg) {
	if(countable()) {
		if(msg && owner() == player)
			act(msg, GlowBlack);
		if(count)
			count--;
		else {
			clear();
			need_update_items = true;
		}
	} else {
		switch(magic) {
		case Blessed:
			// RULE: Blessed don't always use charge
			if(game_chance(30))
				return;
			break;
		case Artifact:
			// RULE: Artifact don't always use charge
			if(game_chance(70))
				return;
			break;
		default:
			// RULE: Mundane item don't always use charge
			if(game_chance(10))
				return;
			break;
		}
		if(hits == 3) {
			if(msg && owner() == player)
				act(msg, GlowBlack);
			clear();
			need_update_items = true;
		} else
			hits++;
	}
}

bool item::is(featn v) const {
	switch(v) {
	case BleedingHit: return (type >= ShortSword && type <= GreatSword);
	case StunningHit: return type == Mace || type == GreatMace || type == WarHammer || type == Staff;
	case PierceHit: return type == Dagger || type == Spear;
	case RetaliateHit: return type == Spear;
	case Large: return type == GreatAxe || type == GreatSword || type == Staff || type == Spear;
	default: return false;
	}
}

void item::setslot(item& v) {
	auto s = equiped();
	if(s == Ammunition || s == Backpack)
		join(v);
	else {
		iswap(*this, v);
		last_item = this;
		need_update_items = true;
	}
}

void item::join(item& v) {
	if(!type) {
		*this = v;
		v.clear();
		need_update_items = true;
	} else {
		if(type != v.type || !v.countable())
			return;
		if(count >= 255)
			return;
		int new_count = count + v.count;
		if(new_count > 255) {
			v.count = new_count - 255;
			count = 255;
		} else {
			count = (unsigned char)new_count;
			v.clear();
		}
		need_update_items = true;
	}
	last_item = this;
}

static const char* get_power_name(variant v) {
	switch(v.type) {
	case Ability: return power_names[v.value];
	case Spell: return spell_power_names[v.value];
	default: return power_names[0];
	}
}

static void add_weapon_info(stringbuilder& sb, const item& it) {
	sb.adds("(%1i)", it.damage());
	if(it.is(Large))
		sb.adds(getname(MsgTwoHands));
}

static void add_info(stringbuilder& sb, int v1, int v2) {
	sb.adds("(%1i,%2i)", v1, v2);
}

const char* item::description() const {
	static char temp[160];
	stringbuilder sb(temp); sb.clear();
	auto w = wear();
	switch(w) {
	case MeleeWeapon: case MeleeWeaponOffhand: case RangedWeapon:
		if(type >= ShieldSmall && type <= ShieldTower)
			add_info(sb, armor(), block());
		else
			add_weapon_info(sb, *this);
		break;
	case Torso: case Head: case Neck: case Girdle:
	case Backward: case Gloves: case Elbows:
	case Legs:
		add_info(sb, armor(), block());
		break;
	default:
		break;
	}
	sb.lower();
	return temp;
}

const char* item::name() const {
	static char temp[160];
	stringbuilder sb(temp);
	auto pn = getname(type);
	auto gi = gender_by_name(pn);
	if(countable()) {
		sb.adds(pn);
		if(countable() && count > 1)
			sb.adds("x%1i", count);
	} else {
		if(known_magic) {
			if(magic)
				sb.adds(getname((magicn)(gi * 4 + magic)));
		}
		sb.adds(pn);
		if(known_magic) {
			auto pw = getpower();
			if(pw)
				sb.adds(get_power_name(pw));
		}
	}
	sb.lower();
	return temp;
}

bool wearable::equip(item& it) {
	auto w = it.wear();
	if(w == Backpack || wears[w])
		return false;
	wears[w].setslot(it);
	return true;
}

void wearable::additem(item& it, bool try_equip) {
	if(it.iscoins()) {
		money += it.cost();
		it.clear();
		need_update_items = true;
		return;
	}
	if(try_equip && equip(it))
		return;
	add_item(owner(), it);
}

int wearable::totalweight() const {
	auto r = 0;
	for(auto& v : wears) {
		if(v)
			r += v.weight();
	}
	return r;
}

void add_item(short unsigned area_index, short unsigned index, item& v) {
	if(!v)
		return;
	for(auto& e : bsdata<itemlay>()) {
		if(e.area_index == area_index && e.index == index)
			e.join(v);
	}
	if(!v) {
		need_update_items = true;
		return;
	}
	auto p = bsdata<itemlay>::addz();
	p->area_index = area_index;
	p->index = index;
	p->join(v);
	need_update_items = true;
}

void update_items() {
	if(!need_update_items)
		return;
	need_update_items = false;
	items.clear();
	for(auto& e : bsdata<itemlay>()) {
		if(e && e.ispresent())
			items.add(&e);
	}
}

bool item::unequip() {
	auto p = owner();
	if(!p)
		return false;
	if(is(Cursed)) {
		known_magic = 1;
		p->say(SayItsMine);
		return false;
	}
	add_item(p, *this);
	return operator bool();
}

void item::act(messagen v, glown glow) const {
	pushvalue push(last_item, const_cast<item*>(this));
	player->act(' ', getname(v), getname(glow));
}

void item::create(int chance_blessed, int chance_cursed, int chance_power) {
	if(countable())
		return;
	auto source = powers();
	if((source && source[0]) || game_chance(chance_power))
		setpower();
	if(game_chance(chance_cursed))
		magic = Cursed;
	else if(game_chance(chance_blessed)) {
		magic = Blessed;
		if(game_chance(chance_blessed / 3))
			magic = Artifact;
	}
}

static void add_support(itemn type) {
	auto ammo = get_ammo(type);
	if(ammo)
		player->equip(item(ammo, xrand(3, 18)));
}

void add_equipment(itemn type) {
	item it(type); it.create(10, 0, 20);
	player->equip(it);
	add_support(type);
}

item citem(itemn v) {
	v = random(v);
	item it(v); it.create();
	if(it.iscoins())
		it.count = xrand(3, 18);
	return it;
}