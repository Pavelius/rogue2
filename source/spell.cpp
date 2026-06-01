#include "area.h"
#include "creature.h"
#include "message.h"
#include "rand.h"
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

bool use_spell(spelln v, creature* opponent, bool run) {
	int effect = 0;
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
			effect = xrand(2, 12);
			opponent->fixmsg(getname(MsgCurePoison), effect, InfoGreen);
		}
		break;
	case Web:
		if(area_is(opponent->index, Webbed))
			return false;
		if(run) {
			auto i = opponent->index;
			area_set(i, Webbed);
		}
		break;
	default:
		return false;
	}
	return true;
}