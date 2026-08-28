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
#include "feats.h"
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

static_assert(sizeof(item) == 2, "Structure `item` must 2 bytes");

collectionv<itemlay> items;
bool need_update_items;
item* last_item;

static variant no_powers[8] = {Variant};
static variant swords_powers[8] = {Variant, WeaponSkill, DamageMelee, Dexterity};
static variant pierce_melee_weapon_powers[8] = {Variant, WeaponSkill, DamageMelee, Dexterity};
static variant blue_potion_powers[8] = {Hits, Mana, Strenght, Dexterity, Wits, Poison, Illness, Drunk};
static variant red_potion_powers[8] = {WeaponSkill, BalisticSkill, Dodge, Armor, FastMove, FastAttack, Fly, FireImmunity};
static variant green_potion_powers[8] = {Regenerating, Boosting, Experience, AcidImmunity, ColdImmunity, DeathImmunity, DiseaseImmunity, PoisonImmunity};
static variant red_tome[8] = {Alchemy, Gemcutting, BalisticSkill, WeaponSkill, Mining, Stealth, History, Religion};
static variant green_tome[8] = {Mana, Hits, Wits, Dexterity, Strenght, Literacy, Metallurgy};
static variant blue_tome[8] = {CureLightWounds, MageArmor, LightSpell, Mending, Sleep, Web, BurningHands, HorrorScare};
static variant blue_rod[8] = {CureSeriousWounds, CureDisease, CurePoison, IceSpear, HorrorScare, LightSpell, SummonUndead};
static variant green_rod[8] = {Sleep, Entaglement, MageArmor, Web};
static variant red_rod[8] = {BurningHands, FireBolt};

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

template<unsigned N>
static itemn random(itemn(&source)[N]) {
	return random(source[rand() % N]);
}

itemn random(itemn v) {
	switch(v) {
	case RandomArmor: return random(random_armors);
	case RandomBlades: return random(random_blades);
	case RandomBladesSmall: return random(random_small_blades);
	case RandomCoins: return random(random_coins);
	case RandomGems: return random(random_gems);
	case RandomMartialWeapon: return random(random_martial_weapons);
	case RandomMissileWeapon: return random(random_missile_weapons);
	case RandomTreasure: return random(random_treasure);
	case RandomWeapon: return random(random_weapon);
	default: return v;
	}
}

static const variant* get_powers(itemn v) {
	switch(v) {
	case Dagger: case LongSword: case ShortSword: case GreatSword: return swords_powers;
	case Spear: return pierce_melee_weapon_powers;
	case BluePotion: return blue_potion_powers;
	case GreenPotion: return green_potion_powers;
	case RedPotion: return red_potion_powers;
	case RedTome: return red_tome;
	case GreenTome: return green_tome;
	case BlueTome: return blue_tome;
	case BlueRod: return blue_rod;
	case GreenRod: return green_rod;
	case RedRod: return red_rod;
	default: return no_powers;
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
	auto powers = get_powers(type);
	if(!powers)
		return variant();
	return powers[power];
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
	auto n = find_power(get_powers(type), v);
	if(n == -1)
		return false;
	power = n;
	return true;
}

bool item::setpower() {
	if(countable())
		return false;
	auto source = get_powers(type);
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

static int get_weight(itemn v) {
	switch(v) {
	case LongSword: return 3 * lb;
	case ShortSword: return 2 * lb;
	case GreatAxe: return 7 * lb;
	case GreatSword: return 6 * lb;
	case ChainMail: return 40 * lb;
	case LeatherArmor: return 15 * lb;
	case StuddedArmor: return 25 * lb;
	case ShieldSmall: return 5 * lb;
	case ShieldMedium: return 10 * lb;
	case ShieldLarge: return 15 * lb;
	case ShieldTower: return 20 * lb;
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
	case RedPotion: return 15 * gp;
	case GreenPotion: return 15 * gp;
	case BluePotion: return 10 * gp;
	case RedTome: return 30 * gp;
	case BlueTome: return 30 * gp;
	case GreenTome: return 30 * gp;
	case ShieldSmall: return 5 * gp;
	case ShieldMedium: return 10 * gp;
	case ShieldLarge: return 15 * gp;
	case ShieldTower: return 30 * gp;
	default: return 0;
	}
}

static int get_damage(itemn v) {
	switch(v) {
	case NoItem: // Unarmed attack
		return 2;
	case Dagger:
		return 4;
	case ShortSword: case Axe: case Spear: case Mace:
		return 6;
	case Scimitar: case WarHammer:
		return 7;
	case LongSword:
		return 8;
	case GreatAxe:
		return 9;
	case GreatSword:
		return 10;
	case ShortBow:
		return 6;
	case LongBow:
		return 8;
	default:
		return 0;
	}
}

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
}

itemn item::ammo() const {
	return get_ammo(type);
}

wearn item::wear() const {
	if(type >= CP && type <= GP)
		return Backpack;
	else if(type >= Staff && type <= GreatSword)
		return MeleeWeapon;
	else if(type >= ShieldSmall && type <= ShieldTower)
		return MeleeWeaponOffhand;
	else if(type >= ShortBow && type <= HeavyCrossbow)
		return RangedWeapon;
	else if(type >= Robe && type <= PlateMail)
		return Torso;
	else if(type >= LeatherBoots && type <= IronBoots)
		return Legs;
	else if(type >= Arrow && type <= Bolt)
		return Ammunition;
	return Backpack;
}

bool item::ismagical() const {
	// Item is magical if is not mudane, if power filled and if power zero and no empty power.
	if(magic != Mundane)
		return true;
	if(power)
		return true;
	auto source = get_powers(type);
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

int item::weight() const {
	return get_weight(type) * getcount();
}

int item::cost() const {
	return get_cost(type) * getcount();
}

int item::damage() const {
	return get_damage(type);
}

int item::speed() const {
	return get_weapon_speed(type);
}

int item::dodge() const {
	switch(type) {
	case StuddedArmor: return -5;
	case HideArmor: return -10;
	case ChainMail: return -15;
	case ScaleMail: return -20;
	case PlateMail: return -30;
	case ShieldLarge: return -10;
	default: return 0;
	}
}

int item::armor() const {
	switch(type) {
	case LeatherArmor: return 1;
	case StuddedArmor: return 1;
	case HideArmor: return 2;
	case ChainMail: return 3;
	case ScaleMail: return 4;
	case PlateMail: return 5;
	case IronBoots: return 1;
	case ShieldMedium: return 1;
	case ShieldLarge: case ShieldTower: return 2;
	default: return 0;
	}
}

int item::block() const {
	switch(type) {
	case StuddedArmor: return 1;
	case LeatherBoots: return 1;
	case ShieldSmall: return 2;
	case ShieldMedium: return 1;
	case ShieldTower: return 1;
	default: return 0;
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
	auto source = get_powers(type);
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