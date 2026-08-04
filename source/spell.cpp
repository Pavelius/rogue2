#include "area.h"
#include "collectiona.h"
#include "creature.h"
#include "game.h"
#include "message.h"
#include "pushvalue.h"
#include "rand.h"
#include "slice.h"
#include "spell.h"
#include "stringbuilder.h"
#include "variant.h"

int get_mana(spelln v) {
	switch(v) {
	case CureLightWounds: return 5;
	case CureSeriousWounds: return 8;
	case CureCriticalWounds: return 11;
	case CureDisease: return 20;
	case CurePoison: return 10;
	case Teleport: return 40;
	case Gate: return 50;
	default: return 3;
	}
}

bool spell_is_hostile(unsigned char v) {
	switch(v) {
	case BurningHands:
	case FireBolt:
	case IceSpear:
	case MagicMissile:
	case Entaglement:
	case HorrorScare:
		return true;
	default:
		return false;
	}
}

bool spell_is_combat(unsigned char v) {
	switch(v) {
	case SummonAnimals: case SummonMinions: case SummonUndead:
		return true;
	default:
		return spell_is_hostile(v);
	}
}

static variantn get_target(spelln v) {
	switch(v) {
	case CureLightWounds: case CureSeriousWounds: case CureCriticalWounds:
	case MageArmor: case LightSpell: case Sleep: case Web:
	case BurningHands: case FireBolt: case IceSpear:
		return Monster; // Target creature
	case MageShield:
		return Spell; // Self only
	case EnchantItem: case CreateArtifact: case UncurseItem:
		return Item;
	default:
		return Variant;
	}
}

static monstern get_minions(monstern v) {
	switch(v) {
	case GiantAntQueen: return GiantAnt;
	default: (monstern)0;
	}
}

static void area_set_near(short unsigned i, areafn v, int maximum) {
	directionn source[] = {North, South, East, West, NorthEast, SouthEast, NorthWest, SouthWest};
	zshuffle(source, lenghtof(source));
	if(maximum > lenghtof(source))
		maximum = lenghtof(source);
	for(auto i = 0; i < 2; i++)
		area_set(to(i, source[i], i), Webbed);
}

bool area_apply(short unsigned index, spelln spell, bool run) {
	return false;
}

bool creature::apply(spelln v, bool run) {
	if(is(v))
		return false;
	switch(v) {
	case CureLightWounds:
		if(hits >= hits_maximum)
			return false;
		if(run)
			heal(xrand(1, 6) + 1);
		break;
	case CureSeriousWounds:
		if(hits >= hits_maximum)
			return false;
		if(run)
			heal(xrand(2, 12) + 3);
		break;
	case CureCriticalWounds:
		if(hits >= hits_maximum)
			return false;
		if(run)
			heal(xrand(3, 18) + 5);
		break;
	case CurePoison:
		if(!abilities[Poison])
			return false;
		if(run) {
			auto v = xrand(2, 12);
			fixmsg(getname(MsgCurePoison), v, GlowGreen);
		}
		break;
	case MageArmor:
		if(run)
			enchant(MageArmor, xrand(3 * Hour, 5 * Hour));
		break;
	case Web:
		if(area_is(index, Webbed))
			return false;
		if(run) {
			area_set(index, Webbed);
			area_set_near(index, Webbed, 1 + player->get(Level) / 4);
		}
		break;
	case SummonAnimals:
	case SummonMinions:
	case SummonUndead:
		break;
	default:
		return false;
	}
	return true;
}

bool item::apply(spelln v, bool run) {
	switch(v) {
	case BlessItem:
		if(magic != Mundane || !known_magic)
			return false;
		if(run) {
			known_magic = 1;
			magic = Blessed;
		}
		break;
	case DetectMagicItem:
		if(known_magic)
			return false;
		if(run)
			known_magic = 1;
		break;
	case EnchantItem:
		if(power || !known_power)
			return false;
		if(run) {
		}
		break;
	case IdentifyItem:
		if(known_power)
			return false;
		if(run)
			known_power = 1;
		break;
	case UncurseItem:
		if(!known_magic || magic != Cursed)
			return false;
		if(run) {
			act(ItemGrowColor, GlowBlue);
			magic = Mundane;
		}
		break;
	case CreateArtifact:
		if(!known_magic || !known_power || !power || magic != Blessed)
			return false;
		if(run) {
			act(ItemGrowColor, GlowYellow);
			magic = Artifact;
		}
		break;
	default:
		return false;
	}
	return true;
}

void choose_spell_targets(spelln spell) {
	targets.clear();
	switch(spell) {
	case Tile:
		break;
	case Item:
		for(auto& e : player->wears) {
			if(!e)
				continue;
			if(e.apply(spell, false))
				targets.add(&e);
		}
		break;
	case Monster:
		if(spell_is_hostile(spell)) {
			for(auto p : enemies) {
				if(p->apply(spell, false))
					targets.add(p);
			}
		} else {
			for(auto p : parcipants) {
				if(p->apply(spell, false))
					targets.add(p);
			}
		}
		break;
	case Spell:
		if(player->apply(spell, false))
			targets.add(player);
		break;
	}
}

bool creature::cast(spelln spell, int mana_cost, bool run) {
	pushvalue push(player, this);
	if(mana_cost > 0) {
		if(mana < mana_cost)
			return false;
	}
	choose_spell_targets(spell);
	if(!targets)
		return false;
	auto maximum_targets = 1;
	auto target = get_target(spell);
	if(run) {
		switch(target) {
		case Monster:
			for(auto p : targets.records<creature>())
				p->apply(spell, true);
			break;
		case Item:
			for(auto p : targets.records<item>())
				p->apply(spell, true);
			break;
		case Feature:
			for(auto p : targets.records<unsigned char>()) {
				auto index = p2i(p);
			}
			break;
		default:
			break;
		}
		if(mana_cost)
			mana -= mana_cost;
	}
	return true;
}