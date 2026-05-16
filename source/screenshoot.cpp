#include "draw.h"
#include "screenshoot.h"
#include "timer.h"

screenshoot::screenshoot(rect rc, bool fade) : surface(rc.width(), rc.height(), getbpp()) {
	x = rc.x1;
	y = rc.y1;
	if(canvas) {
		blit(*this, 0, 0, width, height, 0, *canvas, x, y);
		if(fade) {
			pushrect push;
			auto push_canvas = canvas;
			auto push_clip = clipping;
			auto push_alpha = alpha;
			auto push_fore = fore;
			caret = point(0, 0); ::width = this->width; ::height = this->height;
			canvas = this;
			setclip();
			alpha = 128;
			fore = colors::form;
			rectf();
			fore = push_fore;
			alpha = push_alpha;
			clipping = push_clip;
			canvas = push_canvas;
		}
	}
}

screenshoot::screenshoot(bool fade) : screenshoot({0, 0, getwidth(), getheight()}, fade) {
}

screenshoot::~screenshoot() {
}

void screenshoot::fading(const screenshoot& destination, unsigned milliseconds) const {
	if(!milliseconds)
		return;
	auto start = getcputime();
	auto finish = start + milliseconds;
	auto current = start;
	while(ismodal() && current < finish) {
		auto alpha = ((current - start) << 8) / milliseconds;
		restore();
		canvas->blend(destination, alpha);
		sys_redraw();
		waitcputime(1);
		current = getcputime();
	}
	destination.restore();
}

void screenshoot::restore() const {
	setclip();
	if(canvas)
		blit(*canvas, x, y, width, height, 0, *this, 0, 0);
}

long open_dialog(fnevent proc, bool faded) {
	screenshoot push(faded);
	while(ismodal()) {
		push.restore();
		proc();
		domodal();
	}
	return getresult();
}

void open_dialog() {
	open_dialog((fnevent)hobject, hparam);
}

void scene_appear(fnevent before_paint, unsigned long milliseconds) {
	if(!milliseconds)
		milliseconds = 500;
	pushrect push;
	auto push_clip = clipping;
	setpos(0, 0, canvas->width, canvas->height);
	clipping.set(0, 0, width, height);
	screenshoot before;
	before_paint();
	screenshoot after;
	before.fading(after, milliseconds);
	clipping = push_clip;
}

void scene_disapear(unsigned long milliseconds, color disapear_color) {
	if(!milliseconds)
		milliseconds = 500;
	pushrect push;
	auto push_clip = clipping;
	setpos(0, 0, canvas->width, canvas->height);
	clipping.set(0, 0, width, height);
	screenshoot before;
	pushfore pushf(disapear_color);
	rectf();
	screenshoot after;
	before.fading(after, milliseconds);
	clipping = push_clip;
}