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

#include "timer.h"

unsigned long current_tick;
unsigned long animation_tick;
static unsigned long last_tick;
int current_tick_delta;

void clear_last_tick() {
	last_tick = 0;
}

void update_current_time() {
	current_tick = getcputime();
	if(!last_tick || (current_tick - last_tick) > 300)
		last_tick = current_tick;
	current_tick_delta = current_tick - last_tick;
	last_tick = current_tick;
}

void update_time() {
	update_current_time();
	animation_tick += current_tick_delta;
}