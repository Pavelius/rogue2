#include "adat.h"
#include "area.h"
#include "direction.h"
#include "draw.h"
#include "draw_move.h"
#include "draw_object.h"
#include "resid.h"
#include "timer.h"

struct drawmove {
	point			position, position_start, position_target;
	resid			res;
	short unsigned	frame, flags;
	unsigned long	start, finish;
	constexpr explicit operator bool() { return res != (resid)0; }
	void clear() { memset((void*)this, 0, sizeof(*this)); }
};
static adat<drawmove> objects;

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

static drawmove* find_effect(point position, resid res, int cicle) {
	for(auto& e : objects) {
		if(e && e.position_start == position && e.res == res && e.frame == cicle)
			return &e;
	}
	return 0;
}

static int calculate(int v1, int v2, int n, int m) {
	return v1 + (v2 - v1) * n / m;
}

void update_move_effects() {
	for(auto& e : objects) {
		if(!e || e.start > animation_tick)
			continue;
		else if(e.finish <= animation_tick || e.start == e.finish)
			e.clear();
		else {
			int m = e.finish - e.start;
			int n = animation_tick - e.start;
			if(n >= m)
				n = m;
			e.position.x = (short)calculate(e.position_start.x, e.position_target.x, n, m);
			e.position.y = (short)calculate(e.position_start.y, e.position_target.y, n, m);
		}
	}
	shrink();
}

static void paint_effect(const drawmove* p) {
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
	image(ps, frame, p->flags);
}

void paint_move_effects() {
	pushrect push;
	for(auto& e : objects) {
		caret = e.position - camera;
		paint_effect(&e);
	}
}

bool have_move_effects() {
	return objects.operator bool();
}

static void add_effect(point position, point target, int duration, resid res, int cicle, short unsigned flags) {
	auto p = find_effect(position, res, cicle);
	if(p)
		return;
	p = objects.addz();
	p->position = position;
	p->position_start = position;
	p->position_target = target;
	p->res = res;
	p->frame = cicle;
	p->flags = flags;
	p->start = animation_tick;
	p->finish = p->start + duration;
}

directionn lookat(point from, point to);

static void add_effect(point position, point target, int duration, resid res, int frame) {
	auto d = lookat(position, target);
	switch(d) {
	case North: add_effect(position, target, duration, res, frame + 0, 0); break;
	case South: add_effect(position, target, duration, res, frame + 0, ImageMirrorV); break;
	case East: add_effect(position, target, duration, res, frame + 1, ImageMirrorH); break;
	case West: add_effect(position, target, duration, res, frame + 1, 0); break;
	case NorthEast: add_effect(position, target, duration, res, frame + 2, 0); break;
	case SouthEast: add_effect(position, target, duration, res, frame + 2, ImageMirrorV); break;
	case NorthWest: add_effect(position, target, duration, res, frame + 2, ImageMirrorH); break;
	case SouthWest: add_effect(position, target, duration, res, frame + 2, ImageMirrorH | ImageMirrorV); break;
	default: break;
	}
}

void add_effect(point position, point target, moveablen type, int duration) {
	switch(type) {
	case ShootArrow: add_effect(position, target, duration, ResMissile, 0); break;
	case ShootBolt: add_effect(position, target, duration, ResMissile, 3);  break;
	default: break;
	}
}