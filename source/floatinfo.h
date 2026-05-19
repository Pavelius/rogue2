#pragma once

#include "point.h"

typedef void(*fnevent)();

enum floatinfon : unsigned char {
	InfoRed, InfoGreen, InfoBlue,
};

const unsigned long floatinfo_duration = 300;

void add_text(point position, const char* format, int param, floatinfon fore);
void floatinfo_paint();
void floatinfo_update();
void floatinfo_wait(fnevent proc);
bool have_floatinfo();