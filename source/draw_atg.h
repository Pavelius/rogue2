#pragma once

#include "stringbuilder.h"

enum messagen : unsigned char;

typedef bool(*fnvisible)(const void* object);
typedef void(*fnevent)();
typedef int(*fnvalue)(const void* object);

extern stringbuilder sb;
extern void* current_avatar;
extern long current_avatar_post;
extern fnevent atg_menu;

void* make_player_move(const char* id, int count_left, int cancel_mode);
void* choose_record(const char* id, const char* source_id, fnvisible allow = 0);

bool allow_paint();
void fixclear();
void fixmsg(messagen id);
void game_run(); // Main game entry point
void main_util(); // External function
void next_scene(fnevent v);
void paint_avatars(void** source, int count, fnstatus getname, void* current_player, fnvalue gethits);
void paint_bar(const char* name, fnevent proc);
void paint_button(const char* format, const void* object, bool choose = false);
void paint_hilite();
void paint_separator();
void paint_status_bar();
void paint_status_text();
void paint_window_center(const char* format);
void paint_window_info(const char* format);
bool running_scene();
void stringbuilder_custom(stringbuilder& sb, const char* id);
