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

#include "collectiona.h"
#include "indexa.h"

struct creature;
struct itemlay;

extern collectionv<creature> creatures;
extern collectionv<itemlay> items;

struct gamei {
	unsigned restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day, restore_several_days;
	unsigned minutes;
};
extern gamei game;

short unsigned choose_indecies(const char* header, bool can_cancel = true);

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
void skip_long_time();
void wait_all();