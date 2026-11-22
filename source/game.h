/*
	Copyright 2026 by Pavel Chistyakov

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.77
*/

#pragma once

typedef void(*fnevent)(); // Callback function of any command executing
typedef bool(*fnvisible)(const void* object);

enum resid : unsigned char {
	FontT, Font1, Font2, Font3,
	ResPCBody, ResPCArms, ResPCAccessories,
	ResFow, ResLos, ResCursors,
	ResFloor, ResBorders, ResDecals, ResFeatures, ResItems, ResMonsters,
	ResStatus, ResConditions, ResSplash, ResMissile,
	ResWalls, ResShadows,
};
enum durationn : unsigned {
	Minute = 1, Hour = 60 * Minute, Day = 24 * Hour,
};
enum glown : unsigned char {
	GlowGray, GlowBlack, GlowWhite,
	GlowRed, GlowGreen, GlowBlue,
	GlowBrown, GlowYellow,
	LastGlow = GlowYellow,
};
enum messagen : unsigned char {
	ApplicationTitle, Cancel, Apply, PickUp, DropDown, Inventory,
	Coins, Rounds,
	LevelUp, MsgMultiplyChoose,
	MsgHeal, MsgCurePoison, MsgTwoHands,
	TomeLearnNothing, TomeLearnSkill, TomeLearnSpell, TomeReaded,
	NotEnoughMana, MsgNoTargets, MsgNoRangedWeapon, MsgNoAmmo,
	TasteBad, TasteNormal, TasteGood,
	PlayerEat, PlayerReadTome, PlayerKilled, PlayerFoundSecretDoor, PlayerFoundTrap,
	PlayerBlock, PlayerDodge,
	PlayerPickUpItem, PlayerDropItem,
	PlayerUseLastRodCharge,
	MsgSkillFooter,
	ItemGrowColor, ItemWearTotal, ItemDecoy,
	ItemDamaged, ItemBurnAway, ItemMeldDown, ItemCrumbleToDust, ItemCrushingOut,
	LastMessage = ItemCrushingOut,
};

struct sprite;
struct gamei {
	unsigned restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day, restore_several_days;
	unsigned minutes;
};

extern gamei game;
extern sprite* resid_data[];
extern bool interactive;

short unsigned choose_indecies(const char* header, bool can_cancel = true);

inline sprite* gres(resid id) { return resid_data[id]; }

void add_answer_items(short unsigned area_index, short unsigned index, fnvisible filter);
void choose_player_move();
long choose_menu(const char* cancel, const char* footer = 0);
void end_game();
void initialize_gui();
void open_backpack();
void open_drop_item();
void open_inventory();
void open_ground();
void pass_minute();
void paint_item_description(int index, long value, const char* format);
void set_disabled_fore();
void show_message(const char* header, const char* subheader, const char* message, const char* cancel);
void show_scene(fnevent proc, const char* header, const char* cancel);
void skip_long_time();
void wait_all();