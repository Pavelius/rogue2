#pragma once

#include "point.h"

typedef void(*fnevent)();

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
void paint_objects();
void set_srceen_area(int offset=-128);
void sort_objects();
void update_object_orders();
void update_object_timestamps();
void wait_all_objects(fnevent proc);