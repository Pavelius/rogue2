#pragma once

extern unsigned long current_tick;
extern unsigned long current_game_tick;

extern int current_tick_delta;

void update_tick();
void update_game_tick();
unsigned long getcputime();
void waitcputime(unsigned v);