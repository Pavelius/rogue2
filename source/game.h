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

struct creature;
struct itemlay;

extern collectionv<creature> creatures;
extern collectionv<itemlay> items;

enum gamen : unsigned char {
	Rounds, Blessing,
	Money,
};

struct gamei {
	int variables[Blessing + 1];
	int restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day, restore_several_days;
};

int getv(gamen v);

void addv(gamen v, int i);
void choose_player_move();
void pass_minute();
void skip_long_time();