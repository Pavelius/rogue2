#pragma once

struct creature;

enum spelln : unsigned char {
	CureLightWounds, CureSeriousWounds, CureCriticalWounds, CureDisease, CurePoison,
	MageArmor, MageShield, LightSpell, Mending, Sleep, Web,
	SummonUndead,
	BurningHands, FireBolt, IceSpear, MagicMissile, Entaglement, HorrorScare,
	Gate, Teleport,
	FirstSpell = CureLightWounds, LastSpell = Teleport,
};
struct spellable {
	unsigned spells;
	bool is(spelln v) const { return (spells & (1 << v)) != 0; }
	void set(spelln v) { spells |= (1 << v); }
	void remove(spelln v) { spells &= ~(1 << v); }
};
int get_mana(spelln v);

bool use_spell(spelln v, creature* opponent, bool run);