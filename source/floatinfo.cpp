#include "adat.h"
#include "color.h"
#include "draw.h"
#include "floatinfo.h"
#include "stringbuilder.h"
#include "timer.h"

struct floatinfo {
	const char*		format;
	int				param;
	floatinfon		fore;
	unsigned long	start, finish;
	point			start_position, finish_position, position;
	constexpr explicit operator bool() { return format != 0; }
	void clear() { memset((void*)this, 0, sizeof(*this)); }
};
static adat<floatinfo, 64> objects;
static unsigned long timestamp, timestamp_last;
static point floatinfo_offset(0, -32);

static void update_timestamp() {
	auto c = getcputime();
	if(!timestamp_last || (c - timestamp_last) > 300)
		timestamp_last = c;
	timestamp += c - timestamp_last;
	timestamp_last = c;
}

static floatinfo* find_info(point position) {
	for(auto& e : objects) {
		if(e && e.position == position)
			return &e;
	}
}

static unsigned long find_max_start(point position) {
	unsigned long result = 0;
	for(auto& e : objects) {
		if(e && e.position == position) {
			if(e.start > result)
				result = e.start;
		}
	}
	return result;
}

void add_text(point position, const char* format, int param, floatinfon fore) {
	auto max_start = find_max_start(position);
	if(!max_start)
		max_start = timestamp;
	else
		max_start += floatinfo_duration / 2;
	auto p = objects.addz();
	p->position = position;
	p->start_position = position;
	p->finish_position = position + floatinfo_offset;
	p->start = max_start;
	p->finish = p->start + floatinfo_duration;
	p->format = format;
	p->param = param;
	p->fore = fore;
}

static void shrink() {
	auto pb = objects.begin();
	auto pe = objects.end();
	while(pe > pb) {
		pe--;
		if(*pe)
			break;
		objects.count--;
	}
}

static int calculate(int v1, int v2, int n, int m) {
	return v1 + (v2 - v1) * n / m;
}

void floatinfo_update() {
	update_timestamp();
	for(auto& e : objects) {
		if(e.finish <= timestamp)
			e.clear();
		else if(e.start < timestamp)
			continue;
		else {
			auto m = e.finish - e.start;
			if(m > 0) {
				auto n = timestamp - e.start;
				if(n >= m)
					n = m;
				e.position.x = (short)calculate(e.start_position.x, e.finish_position.x, n, m);
				e.position.y = (short)calculate(e.start_position.y, e.finish_position.y, n, m);
			}
		}
	}
	shrink();
}

static color get_color(floatinfon v) {
	switch(v) {
	case InfoRed: return colors::red;
	case InfoGreen: return colors::green;
	case InfoBlue: return colors::blue;
	default: return colors::text;
	}
}

void floatinfo_paint() {
	pushrect push;
	pushfore push_fore;
	rect area = clipping; area.offset(-128, -128);
	for(auto& e : objects) {
		if(!e || e.start > timestamp)
			continue;
		if(!e.position.in(area))
			continue;
		auto p = str(e.format, e.param);
		auto w = textw(p);
		fore = get_color(e.fore);
		caret = e.position;
		caret.x -= w / 2;
		text(p);
	}
}

void floatinfo_wait(fnevent proc) {
	update_timestamp();
	while(objects && running_scene() && ismodal()) {
		update_timestamp();
		proc();
		sys_redraw();
		waitcputime(1);
	}
}

bool have_floatinfo() {
	return objects.operator bool();
}