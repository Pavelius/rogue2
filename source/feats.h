#pragma once

enum featn : unsigned char {
	DarkVision, Mirrorred,
	BloodingHit, StunningHit, ParalizeHit, PoisonHit, PushHit, ArmorPierceHit,
	WaterWalking,
};
struct featable {
	unsigned feats;
	constexpr bool is(featn v) const { return (feats & (1 << v)) != 0; }
	constexpr void remove(featn v) { feats &= ~(1 << v); }
	constexpr void set(featn v) { feats |= (1 << v); }
};
