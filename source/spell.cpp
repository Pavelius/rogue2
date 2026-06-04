#include "area.h"
#include "creature.h"
#include "game.h"
#include "message.h"
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

bool use_spell(spelln v, creature* opponent, bool run) {
	if(opponent->is(v))
		return false;
	switch(v) {
	case CureLightWounds:
		if(opponent->hits >= opponent->hits_maximum)
			return false;
		if(run)
			opponent->heal(xrand(1, 6) + 1);
		break;
	case CureSeriousWounds:
		if(opponent->hits >= opponent->hits_maximum)
			return false;
		if(run)
			opponent->heal(xrand(2, 12) + 3);
		break;
	case CureCriticalWounds:
		if(opponent->hits >= opponent->hits_maximum)
			return false;
		if(run)
			opponent->heal(xrand(3, 18) + 5);
		break;
	case CurePoison:
		if(!opponent->abilities[Poison])
			return false;
		if(run) {
			auto v = xrand(2, 12);
			opponent->fixmsg(getname(MsgCurePoison), v, InfoGreen);
		}
		break;
	case MageArmor:
		if(run) {
			opponent->enchant(MageArmor, xrand(3 * Hour, 5 * Hour));
			// opponent->fixmsg(getname(MageArmor), 0, InfoGreen);
		}
		break;
	case Web:
		if(area_is(opponent->index, Webbed))
			return false;
		if(run) {
			area_set(opponent->index, Webbed);
			area_set_near(opponent->index, Webbed, 1 + player->get(Level) / 4);
		}
		break;
	default:
		return false;
	}
	return true;
}

bool use_spell(spelln v, item* target, bool run) {
	switch(v) {
	case BlessItem:
		if(target->magic == Mundane)
			return false;
		if(run) {
			target->magic = Blessed;
			target->known_magic = 1;
		}
		break;
	case DetectMagicItem:
		if(target->known_magic)
			return false;
		if(run)
			target->known_magic = 1;
		break;
	case EnchantItem:
		if(target->power)
			return false;
		if(run) {
		}
		break;
	case IdentifyItem:
		if(target->known_power)
			return false;
		if(run)
			target->known_power = 1;
		break;
	case UncurseItem:
		if(!target->known_magic || target->magic != Cursed)
			return false;
		if(run)
			target->magic = Mundane;
		break;
	case CreateArtifact:
		if(target->magic != Artifact)
			return false;
		if(run)
			target->magic = Artifact;
		break;
	default:
		return false;
	}
	return true;
}

