#include "draw.h"
#include "stringbuilder.h"

namespace {
struct hotname {
	const char*	id;
	unsigned	key;
};
hotname names[] = {
	{"Backspace", KeyBackspace},
	{"Down", KeyDown},
	{"End", KeyEnd},
	{"Esc", KeyEscape},
	{"Home", KeyHome},
	{"Left", KeyLeft},
	{"Right", KeyRight},
	{"PageUp", KeyPageUp},
	{"PageDown", KeyPageDown},
	{"Up", KeyUp},
	{"Ctrl", Ctrl},
	{"Alt", Alt},
};
}

static const hotname* find_hot(unsigned key) {
	for(auto& e : names) {
		if(e.key == key)
			return &e;
	}
	return 0;
}

//unsigned find_key_by_name(const char* p) {
//	for(auto& e : names) {
//		if(equal(e.id, p))
//			return e.key;
//	}
//	return 0;
//}

const char* find_name_by_key(unsigned key) {
	for(auto& e : names) {
		if(e.key == key)
			return e.id;
	}
	return 0;
}

static void add_prefix_key(stringbuilder& sb, unsigned& key, int prefix) {
	if((key & prefix)==0)
		return;
	auto pn = find_name_by_key(prefix);
	if(!pn)
		return;
	sb.add(pn);
	key = key & (~prefix);
	if(key)
		sb.add("+");
}

void key2str(stringbuilder& sb, unsigned key) {
	add_prefix_key(sb, key, Ctrl);
	add_prefix_key(sb, key, Alt);
	add_prefix_key(sb, key, Shift);
	auto pn = find_hot(key);
	if(pn)
		sb.add(pn->id);
	else
		sb.add(key);
}