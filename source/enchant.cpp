#include "bsdata.h"
#include "enchant.h"

static enchanti* find(short unsigned parent, variant value) {
	for(auto& e : bsdata<enchanti>()) {
		if(e.parent == parent && e.value == value)
			return &e;
	}
	return 0;
}

void add_enchant(short unsigned parent, variant value, unsigned stop) {
	auto p = find(parent, value);
	if(!p)
		p = bsdata<enchanti>::addz();
	p->parent = parent;
	p->value = value;
	if(p->stop < stop)
		p->stop = stop;
}

static void shrink() {
	auto pb = bsdata<enchanti>::begin();
	auto pe = bsdata<enchanti>::end();
	while(pe > pb) {
		pe--;
		if(*pe)
			break;
		bsdata<enchanti>::source.count--;
	}
}

void remove_enchant(short unsigned parent) {
	for(auto& e : bsdata<enchanti>()) {
		if(e && e.parent == parent)
			e.clear();
	}
}

void remove_enchant(short unsigned parent, variant value) {
	for(auto& e : bsdata<enchanti>()) {
		if(e && e.parent == parent && e.value==value)
			e.clear();
	}
}

void update_enchantments(unsigned stop) {
	for(auto& e : bsdata<enchanti>()) {
		if(e && e.stop <= stop)
			e.clear();
	}
	shrink();
}

void enchanti::clear() {
	memset((void*)this, 0, sizeof(*this));
}