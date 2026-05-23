#include "adat.h"
#include "area.h"
#include "draw.h"
#include "draw_object.h"
#include "draw_effect.h"
#include "resid.h"
#include "timer.h"

struct draweffect {
	point			position;
	resid			res;
	unsigned char	start_alpha, finish_alpha;
	short unsigned	frame;
	unsigned long	start, finish;
	constexpr explicit operator bool() { return res != (resid)0; }
	void clear() { memset((void*)this, 0, sizeof(*this)); }
};
static adat<draweffect> objects;

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

static draweffect* find_effect(point position, resid res, int cicle) {
	for(auto& e : objects) {
		if(e && e.position == position && e.res == res && e.frame == cicle)
			return &e;
	}
	return 0;
}

//static draweffect* find_effect(point position) {
//	for(auto& e : objects) {
//		if(e && e.position == position)
//			return &e;
//	}
//	return 0;
//}

void add_effect(point position, resid res, int cicle, int duration) {
	auto p = find_effect(position, res, cicle);
	if(p)
		return;
	p = objects.addz();
	p->position = position;
	p->frame = cicle;
	p->res = res;
	p->start = animation_tick;
	p->finish = p->start + duration;
}

void add_effect(point position, visualn id) {
	switch(id) {
	case BloodVisual: add_effect(position, ResConditions, 0, 500); break;
	case PoisonVisual: add_effect(position, ResConditions, 2, 500); break;
	case SearchVisual: add_effect(position, ResConditions, 2, 500); break;
	default: break;
	}
}

void update_effects() {
	for(auto& e : objects) {
		if(!e || e.start > animation_tick)
			continue;
		else if(e.finish <= animation_tick)
			e.clear();
	}
	shrink();
}

static void paint_effect(const draweffect* p) {
	auto ps = gres(p->res);
	if(!ps)
		return;
	auto frame = p->frame;
	if(ps->cicles_offset) {
		auto pc = ps->gcicle(p->frame);
		if(!pc->count)
			return;
		auto per_frame = (p->finish - p->start) / pc->count;
		if(!per_frame)
			per_frame = 1;
		frame = (short unsigned)(pc->start + ((animation_tick - p->start) / per_frame));
	}
	image(ps, frame, 0);
}

void paint_effects() {
	pushrect push;
	for(auto& e : objects) {
		caret = e.position - camera;
		paint_effect(&e);
	}
}

bool have_effects() {
	return objects.operator bool();
}