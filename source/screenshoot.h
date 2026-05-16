#pragma once

#include "point.h"
#include "draw.h"

struct screenshoot : public point, public surface {
	screenshoot(bool fade = false);
	screenshoot(rect rc, bool fade = false);
	~screenshoot();
	void fading(const screenshoot& destination, unsigned milliseconds) const;
	void restore() const;
};

long open_dialog(fnevent proc, bool faded);
void scene_appear(fnevent before_paint, unsigned long milliseconds);
void scene_disapear(unsigned long milliseconds, color disapear_color);