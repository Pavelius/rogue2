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

enum featn : unsigned char {
	DarkVision,
	BloodingHit, StunningHit, ParalizeHit, PoisonHit, PushHit, ArmorPierceHit,
	FireResistance, ColdResistance,
	FireImmunity, ColdImmunity,
	WaterWalking,
	Blooding, Regenerating,
	Mirrorred, Enemy, Local,
};
struct featable {
	unsigned feats;
	constexpr bool is(featn v) const { return (feats & (1 << v)) != 0; }
	constexpr void remove(featn v) { feats &= ~(1 << v); }
	constexpr void set(featn v) { feats |= (1 << v); }
};
