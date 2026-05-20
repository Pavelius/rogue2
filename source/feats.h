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

#pragma once

#include "flagable.h"

enum featn : unsigned char {
	DarkVision,
	BleedingHit, MightyHit, ParalizeHit, PierceHit, StunningHit, PoisonHit, PushHit, VorpalHit, RetaliateHit,
	WeakPoison, StrongPoison, DeathPoison, ColdDamage, FireDamage,
	AcidResistance, ColdResistance, DeathResistance, DiseaseResist, FireResistance, PoisonResistance, StunResistance,
	AcidImmunity, ColdImmunity, DeathImmunity, DiseaseImmunity, FireImmunity, PoisonImmunity, StunImmunity,
	Fly, FastMove, SlowMove, FastAttack, SlowAttack,
	Blooding, Regenerating, Stun,
	Mirrorred, Enemy, Local,
};
struct featable {
	unsigned feats[2];
	constexpr bool is(featn v) const { return (feats[v / 32] & (1 << (v % 32))) != 0; }
	constexpr void remove(featn v) { feats[v / 32] &= ~(1 << (v % 32)); }
	constexpr void set(featn v) { feats[v / 32] |= (1 << (v % 32)); }
};
