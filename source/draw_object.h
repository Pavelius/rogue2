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

#include "point.h"

typedef void(*fnevent)();
typedef bool(*fncondition)();

enum resid : unsigned char;
enum rendern : unsigned char;

struct sprite;

struct drawable { // Abstract game obbject
	point			position;
	unsigned char	priority; // Priority for sort
	rendern			render; // Render type
	constexpr explicit operator bool() const { return priority != 0; }
	void fixmove(point v, unsigned long duration);
	bool onscreen() const;
	void stop() const;
};
struct drawobject : drawable {
	short unsigned	frame; // Frame or data index
};
struct drawrender {
	fnevent			proc;
};

extern drawable* last_object;
extern point camera_maximum;

int object_def_sort(const void* object);

void add_object(rendern render, point v, short unsigned frame, unsigned char priority);
void add_object(drawable* p);
void camera_correct();
void camera_set(point v);
void camera_set_screen(int w, int h);
bool camera_visible(point goal, int border);
void clear_drawobjects();
void clear_objects();
void for_each_object(fnevent proc);
bool have_orders();
bool object_onscreen(point position);
void paint_objects();
void remove_order(const drawable* object);
void set_srceen_area(int offset=-128);
void sort_objects();
void sync_scene(fnevent proc, fncondition allow);
void update_object_orders();