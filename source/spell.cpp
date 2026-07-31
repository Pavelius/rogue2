#include "area.h"
#include "creature.h"
#include "game.h"
#include "message.h"
#include "pushvalue.h"
#include "rand.h"
#include "slice.h"
#include "spell.h"
#include "stringbuilder.h"

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

static void area_set_near(short unsigned i, areafn v, int maximum) {
	directionn source[] = {North, South, East, West, NorthEast, SouthEast, NorthWest, SouthWest};
	zshuffle(source, lenghtof(source));
	if(maximum > lenghtof(source))
		maximum = lenghtof(source);
	for(auto i = 0; i < 2; i++)
		area_set(to(i, source[i], i), Webbed);
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
	default:
		return false;
	}
	return true;
}

bool item::apply(spelln v, bool run) {
	switch(v) {
	case BlessItem:
		if(magic >= Blessed)
			return false;
		if(run) {
			magic = Blessed;
			known_magic = 1;
		}
		break;
	case DetectMagicItem:
		if(known_magic)
			return false;
		if(run)
			known_magic = 1;
		break;
	case EnchantItem:
		if(power)
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
		if(!known_magic || !known_power || !power || magic == Artifact)
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