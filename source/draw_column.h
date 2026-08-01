#pragma once

struct drawcolumn {
	typedef const char*(*fninfo)(int index, long value, const char* format);
	typedef void(*fnpaint)(int index, long value, const char* format);
	fninfo		proc;
	int			width;
	unsigned	flags; // Align
	fnpaint		paint; // Override paint proc
	constexpr explicit operator bool() const { return proc != 0; }
};
extern drawcolumn* last_columns;
