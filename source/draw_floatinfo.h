#pragma once

#include "point.h"

typedef void(*fnevent)();

enum glown : unsigned char;

const unsigned long floatinfo_duration = 300;

void add_floatinfo(point position, const char* format, int param, glown fore);
void paint_floatinfo();
void update_floatinfo();
bool have_floatinfo();