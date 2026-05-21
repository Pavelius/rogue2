#pragma once

struct drawcolumn {
	typedef const char*(*fninfo)(long value);
	fninfo		proc;
	int			width;
	unsigned	flags; // Align
	constexpr explicit operator bool() const { return proc != 0; }
};
extern drawcolumn* last_columns;
