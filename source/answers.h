#include "adat.h"
#include "stringbuilder.h"

#pragma once

typedef void(*fnabutton)(int index, long value, const char* text);

struct answers {
	struct element {
		long 		value;
		const char* text;
		int			weight;
	};
	char buffer[2048];
	stringbuilder sc;
	adat<element, 32> elements;
	static const answers* last;
	static bool			interactive;
	static int			column_count;
	static const char*	header;
	static const char*	resid;
	static const char*	string;
	static bool			show_tips;
	answers() : sc(buffer) {}
	constexpr operator bool() const { return elements.count != 0; }
	void				add(long value, const char* name, ...);
	void				addv(long value, const char* name, const char* format);
	const element*		begin() const { return elements.data; }
	element*			begin() { return elements.data; }
	long				choose(const char* title = 0, const char* cancel_text = 0, int cancel_mode = 0) const;
	void				clear();
	static int			compare(const void* v1, const void* v2);
	const element*		end() const { return elements.end(); }
	const element*		find(long value) const;
	int					getcount() const { return elements.getcount(); }
	const char*			getname(long v);
	int					indexof(const void* v) const { return elements.indexof(v); }
	bool				makeweight();
	bool				modal(const char* title, const char* cancel) const;
	void				paintanswers(fnabutton proc, int columns, const char* cancel_text) const;
	long				random() const;
	long				randomweight() const;
	void				remove(int index) { elements.remove(index, 1); }
	void				sort();
	int					totalweight() const;
};
extern answers an;

unsigned anhotkey(int index);

const char* find_separator(const char* p);

long choose_answers(const char* title, const char* cancel_text, int columns);
