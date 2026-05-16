#include "answers.h"
#include "pushvalue.h"
#include "rand.h"

const char* answers::header;
const char* answers::string;
const char* answers::resid;

bool answers::show_tips = true;
bool answers::interactive = true;
int answers::column_count = 1;

const answers* answers::last;

answers an;

static bool allow(const answers& an, size_t max_width) {
	for(auto& e : an) {
		if(zlen(e.text) > max_width)
			return false;
	}
	return true;
}

static int getcolumns(const answers& an) {
	auto count = an.getcount();
	if(!count)
		return 1;
	auto result = 1;
	if(count > 3 && (count % 3) == 0 && allow(an, 13))
		result = 3;
	else if((count % 2) == 0)
		result = 2;
	return result;
}

unsigned anhotkey(int index) {
	static char hotkeys[] = {
		'1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
		'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
	};
	if((size_t)index > sizeof(hotkeys) / sizeof(hotkeys[0]) - 1)
		index = sizeof(hotkeys) / sizeof(hotkeys[0]) - 1;
	return hotkeys[index];
}

int answers::compare(const void* v1, const void* v2) {
	return szcmp(((answers::element*)v1)->text, ((answers::element*)v2)->text);
}

void answers::addv(long value, const char* text, const char* format) {
	auto p = elements.add();
	p->value = value;
	p->text = sc.get();
	sc.addv(text, format);
	sc.addsz();
}

void answers::add(long value, const char* name, ...) {
	XVA_FORMAT(name);
	addv(value, name, format_param);
}

int	answers::totalweight() const {
	auto n = 0;
	for(auto& e : elements)
		n += e.weight;
	return n;
}

void answers::sort() {
	qsort(elements.data, elements.count, sizeof(elements.data[0]), compare);
}

bool answers::modal(const char* title, const char* cancel) const {
	auto proc = (fnevent)choose(title, cancel);
	if(!proc)
		return false;
	proc();
	return true;
}

long answers::random() const {
	if(!elements.count)
		return 0;
	return elements.data[rand() % elements.count].value;
}

long answers::randomweight() const {
	if(!elements.count)
		return 0;
	auto n = totalweight();
	if(!n)
		return 0;
	auto m = rand() % n;
	for(auto& e : elements) {
		m -= e.weight;
		if(m <= 0)
			return e.value;
	}
	return 0;
}

const char* answers::getname(long v) {
	for(auto& e : elements) {
		if(e.value == v)
			return e.text;
	}
	return 0;
}

void answers::clear() {
	elements.clear();
	sc.clear();
}

long answers::choose(const char* title, const char* cancel_text, int cancel_mode) const {
	if(!interactive)
		return random();
	if(cancel_mode == 2 && elements.getcount() == 1)
		return elements.data[0].value;
	if(!elements) {
		if(!cancel_mode || (cancel_mode && cancel_text == 0))
			return 0;
	}
	auto columns = column_count;
	if(columns == -1)
		columns = getcolumns(*this);
	pushvalue push(last, this);
	return choose_answers(title, cancel_text, columns);
}

bool answers::makeweight() {
	auto ps = elements.begin();
	for(auto& e : *this) {
		if(!e.weight)
			continue;
		*ps++ = e;
	}
	if(ps == elements.data)
		return false;
	elements.count = ps - elements.data;
	return true;
}

const answers::element* answers::find(long value) const {
	for(auto& e : elements) {
		if(e.value==value)
			return &e;
	}
	return 0;
}

const char* find_separator(const char* pb) {
	auto p = pb;
	while(*p) {
		if(*p == '-' && p[1] == '+' && p[2] == '-' && (p[3] == 10 || p[3] == 13) && p > pb && (p[-1] == 10 || p[-1] == 13))
			return p;
		p++;
	}
	return 0;
}