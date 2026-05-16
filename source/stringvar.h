#include "stringbuilder.h"

#pragma once

struct stringvari {
	const char*	id;
	fnprint		proc;
};

bool stringvar_identifier(stringbuilder& sb, const char* identifier);