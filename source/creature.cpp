#include "area.h"
#include "bsdata.h"
#include "collectiona.h"
#include "creature.h"
#include "message.h"
#include "rand.h"

creature* human;
creature* player;
creature* opponent;
static int roll_result;

collectiona creatures;
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

static bool is_blocked(featuren v) {
	switch(v) {
	case TreePalm: case Tree: case DeadTree:
	case Grave:
	case Statue:
	case Door: case LockedDoor: case StuckDoor:
	case StairsUp: case StairsDown:
		return true;
	default:
		return false;
	}
}

bool is_blocked(short unsigned i) {
	switch(area_tiles[i]) {
	case Water:
	case DeepWater:
	case DarkWater:
		return true;
	case WallBuilding:
	case WallCave:
	case WallDungeon:
	case WallFire:
	case WallIce:
		return true;
	default:
		break;
	}
	if(is_blocked(area_features[i]))
		return true;
	return false;
}