///////////////////////////////////////////////////////////////////////////
//
//  Copyright 2026 by Pavel Chistyakov
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http ://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "adat.h"
#include "bsdata.h"
#include "draw.h"
#include "draw_object.h"
#include "pushvalue.h"
#include "slice.h"
#include "timer.h"

point camera_maximum;
drawable* last_object;

struct draworder {
	drawable*		object;
	point			start, finish;
	unsigned long	start_time, finish_time;
	constexpr explicit operator bool() const { return object != 0; }
	void clear() { object = 0; }
	void update();
};
static adat<draworder, 128> orders;
static adat<drawable*, 256> objects;
static rect last_screen, last_area;

BSDATAC(drawobject, 128)

static int calculate(int v1, int v2, int n, int m) {
	return v1 + (v2 - v1) * n / m;
}

static draworder* last_order(const drawable* p) {
	draworder* result = 0;
	for(auto& e : orders) {
		if(e.object == p) {
			if(result && result->start_time > e.start_time)
				continue;
			result = &e;
		}
	}
	return result;
}

bool drawable::onscreen() const {
	return position.in(last_area);
}

void drawable::fixmove(point v, unsigned long duration) {
	auto po = last_order(this);
	if(position == v && !po)
		return;
	auto pn = orders.addz();
	pn->object = this;
	if(po) {
		pn->start = po->finish;
		pn->start_time = po->finish_time + 1;
		duration -= 1;
	} else {
		pn->start = position;
		pn->start_time = animation_tick;
	}
	pn->finish = v;
	pn->finish_time = pn->start_time + duration;
}

void drawable::stop() const {
	for(auto& e : orders) {
		if(e.object == this)
			e.object = 0;
	}
}

void draworder::update() {
	if(!object || start_time > animation_tick)
		return;
	auto m = finish_time - start_time;
	if(m > 0) {
		auto n = animation_tick - start_time;
		if(n >= m)
			n = m;
		object->position.x = (short)calculate(start.x, finish.x, n, m);
		object->position.y = (short)calculate(start.y, finish.y, n, m);
	}
	if(animation_tick >= finish_time)
		clear();
}

static int compare(const void* v1, const void* v2) {
	auto p1 = *((drawable**)v1);
	auto p2 = *((drawable**)v2);
	auto r1 = p1->priority / 5;
	auto r2 = p2->priority / 5;
	if(r1 != r2)
		return r1 - r2;
	if(p1->position.y != p2->position.y)
		return p1->position.y - p2->position.y;
	if(p1->priority != p2->priority)
		return p1->priority - p2->priority;
	if(p1->position.x != p2->position.x)
		return p1->position.x - p2->position.x;
	return p1 - p2;
}

void sort_objects() {
	qsort(objects.data, objects.count, sizeof(objects.data[0]), compare);
}

void clear_drawobjects() {
	bsdata<drawobject>::source.clear();
}

void clear_objects() {
	objects.clear();
}

void set_srceen_area(int offset) {
	last_screen = {caret.x, caret.y, caret.x + width, caret.y + height};
	last_area = last_screen; last_area.move(camera.x, camera.y);
	last_area.offset(offset, offset);
	camera_correct();
}

void paint_objects() {
	pushrect push;
	pushvalue push_last(last_object);
	for(auto p : objects) {
		last_object = p;
		caret = p->position - camera;
		bsdata<drawrender>::elements[p->render].proc();
	}
}

void for_each_object(fnevent proc) {
	pushrect push;
	pushvalue push_last(last_object);
	for(auto p : objects) {
		last_object = p;
		caret = p->position - camera;
		proc();
	}
}

static int screen_width() {
	auto r = last_screen.width();
	if(!r)
		r = getwidth();
	return r;
}

static int screen_height() {
	auto r = last_screen.height();
	if(!r)
		r = getheight();
	return r;
}

void camera_correct() {
	if(camera.x > camera_maximum.x - screen_width())
		camera.x = camera_maximum.x - screen_width();
	if(camera.y > camera_maximum.y - screen_height())
		camera.y = camera_maximum.y - screen_height();
	if(camera.x < 0)
		camera.x = 0;
	if(camera.y < 0)
		camera.y = 0;
}

void camera_set_screen(int x, int y) {
	last_screen.set(0, 0, x, y);
	last_area = last_screen;
}

void camera_set(point v) {
	v.x -= screen_width() / 2;
	v.y -= screen_height() / 2;
	camera = v;
	camera_correct();
}

bool camera_visible(point goal, int border) {
	rect rc = {camera.x, camera.y, camera.x + last_screen.width(), camera.y + last_screen.height()};
	rc.offset(-border);
	return goal.in(rc);
}

void add_object(rendern render, point v, short unsigned frame, unsigned char priority) {
	auto p = bsdata<drawobject>::add();
	p->render = render;
	p->position = v;
	p->frame = frame;
	p->priority = priority;
	objects.add(p);
}

void add_object(drawable* p) {
	objects.add(p);
}

static void shrink_orders() {
	auto pb = orders.begin();
	auto pe = orders.end();
	while(pe > pb) {
		pe--;
		if(*pe)
			break;
		orders.count--;
	}
}

void remove_order(const drawable* object) {
	for(auto& e : orders) {
		if(e.object==object)
			e.clear();
	}
}

void update_object_orders() {
	for(auto& e : orders)
		e.update();
	shrink_orders();
}

void sync_scene(fnevent proc, fncondition allow) {
	while(allow() && running_scene() && ismodal()) {
		proc();
		sys_redraw();
		waitcputime(1);
	}
}

bool have_orders() {
	return orders.operator bool();
}