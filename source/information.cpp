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
#include "dice.h"
#include "stringbuilder.h"
#include "stringvar.h"

static void list_of_feats(stringbuilder& sb) {
}

static void player_name(stringbuilder& sb) {
	sb.add(player->name());
}

static void addv(stringbuilder& sb, const char* header, const char* value) {
	sb.addn("%1: %2", header, value);
}

static void addv(stringbuilder& sb, const char* header, int value) {
	sb.addn("%1: %2i", header, value);
}

static void addv(stringbuilder& sb, abilityn id) {
	addv(sb, getname(id), player->abilities[id]);
}

static void player_character_panel(stringbuilder& sb) {
	sb.addn(player->name());
	sb.addn(getname(player->type));
	addv(sb, Strenght);
	addv(sb, Dexterity);
	addv(sb, Wits);
	addv(sb, WeaponSkill);
}

BSDATA(stringvari) = {
	{"ListOfFeats", list_of_feats},
	{"PlayerCharacterPanel", player_character_panel},
	{"PlayerName", player_name},
};
BSDATAF(stringvari)