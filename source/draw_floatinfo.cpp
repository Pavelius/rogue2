#include "adat.h"
#include "color.h"
#include "draw.h"
#include "draw_floatinfo.h"
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
static point floatinfo_offset(0, -32);

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

void add_floatinfo(point position, const char* format, int param, floatinfon fore) {
	auto max_start = find_max_start(position);
	if(!max_start)
		max_start = animation_tick;
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

void update_floatinfo() {
	for(auto& e : objects) {
		if(!e || e.start > animation_tick)
			continue;
		else if(e.finish <= animation_tick)
			e.clear();
		else {
			auto m = e.finish - e.start;
			if(m > 0) {
				auto n = animation_tick - e.start;
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

void paint_floatinfo() {
	pushrect push;
	pushfore push_fore;
	rect area = clipping; area.offset(-128, -128);
	for(auto& e : objects) {
		if(!e || e.start > animation_tick)
			continue;
		caret = e.position - camera;
		if(!caret.in(area))
			continue;
		auto p = str(e.format, e.param);
		auto w = textw(p);
		fore = get_color(e.fore);
		caret.x -= w / 2;
		text(p);
	}
}

bool have_floatinfo() {
	return objects.operator bool();
}

