#include "spell.h"

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