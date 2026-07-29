#pragma once

#include "slice.h"

struct stringset {
	const char*			id;
	slice<const char*>	strings;
};
