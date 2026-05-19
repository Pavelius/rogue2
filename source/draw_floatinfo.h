#pragma once

#include "point.h"

typedef void(*fnevent)();

enum floatinfon : unsigned char {
	InfoRed, InfoGreen, InfoBlue,
};

const unsigned long floatinfo_duration = 300;

void add_floatinfo(point position, const char* format, int param, floatinfon fore);
void paint_floatinfo();
void update_floatinfo();
bool have_floatinfo();