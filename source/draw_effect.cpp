#include "adat.h"
#include "area.h"
#include "draw.h"
#include "draw_object.h"
#include "draw_effect.h"
#include "resid.h"
#include "timer.h"

struct draweffect : drawobject {
	resid			res;
	unsigned long	start, finish;
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

void add_effect(point position, resid res, int cicle, int priority, int duration) {
	auto p = find_effect(position, res, cicle);
	if(p)
		return;
	p = objects.addz();
	p->render = RenderEffect;
	p->priority = priority;
	p->position = position;
	p->frame = cicle;
	p->res = res;
	p->start = animation_tick;
	p->finish = p->start + duration;
}

void add_effect(point position, visualn id) {
	switch(id) {
	case BloodVisual: add_effect(position, ResConditions, 0, 15, 500); break;
	default: break;
	}
}

void add_effects() {
	for(auto& e : objects) {
		if(!e || e.start > animation_tick)
			continue;
		else if(e.finish <= animation_tick) {
			e.clear();
			continue;
		}
		if(e.onscreen())
			add_object(&e);
	}
	shrink();
}

void paint_effect() {
	auto p = (draweffect*)last_object;
	auto ps = gres(p->res);
	if(!ps)
		return;
	auto pc = ps->gcicle(p->frame);
	if(!pc->count)
		return;
	auto per_frame = (p->finish - p->start) / pc->count;
	if(!per_frame)
		return;
	auto frame = pc->start + ((animation_tick - p->start) / per_frame);
	image(ps, frame, 0);
}

bool have_effects() {
	return objects.operator bool();
}