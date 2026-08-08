#pragma once

struct creature;
struct item;

enum spelln : unsigned char {
	CureLightWounds, CureSeriousWounds, CureCriticalWounds, CureDisease, CurePoison,
	MageArmor, MageShield, LightSpell, Mending, Sleep, Web,
	SummonAnimals, SummonMinions, SummonUndead,
	BlessItem, DetectMagicItem, IdentifyItem, UncurseItem, EnchantItem, CreateArtifact,
	BurningHands, FireBolt, IceSpear, MagicMissile, Entaglement, HorrorScare, TurnUndead,
	Gate, Teleport,
	FirstSpell = CureLightWounds, LastSpell = Teleport,
};
extern const char* spell_power_names[LastSpell + 1];
struct spellable {
	unsigned spells;
	constexpr bool is(spelln v) const { return (spells & (1 << v)) != 0; }
	constexpr void set(spelln v) { spells |= (1 << v); }
	constexpr void remove(spelln v) { spells &= ~(1 << v); }
};
int get_mana(spelln v);

bool area_apply(short unsigned index, spelln spell, bool run);
void choose_spell_targets(spelln spell);
bool spell_is_combat(unsigned char v);
bool spell_is_hostile(unsigned char v);