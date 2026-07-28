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

#include "bsdata.h"
#include "creature.h"
#include "collectiona.h"
#include "itemlay.h"
#include "feats.h"
#include "math.h"
#include "rand.h"
#include "slice.h"
#include "stringbuilder.h"
#include "variant.h"

struct treasurei {
	struct infoi {
		char	chance;
		short	from, to;
	};
	char symbol;
	infoi cooper, silver, gold, platinum, gems, magic;
};

static itemn small_blades[] = {Dagger, Dagger, ShortSword, Scimitar};
static itemn blades[] = {ShortSword, LongSword, GreatSword};
static itemn weapons[] = {Staff, Spear, Spear, Axe, Axe, Mace, Mace, WarHammer, WarHammer, GreatMace, GreatAxe};
static itemn bows[] = {ShortBow, ShortBow, LongBow, Crossbow, HeavyCrossbow};
static itemn armors[] = {Robe, LeatherArmor, LeatherArmor, LeatherArmor, StuddedArmor, StuddedArmor, HideArmor, HideArmor, ScaleMail, ChainMail, PlateMail};

static item random_item(const slice<itemn>& source) {
	return item(source.begin()[rand() % source.count]);
}

item random_item() {
	static slice<itemn> source[] = {
		small_blades, blades, weapons, bows,
		armors, armors,
	};
	return random_item(maprnd(source));
}