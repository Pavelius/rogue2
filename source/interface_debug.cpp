#include "draw.h"
#include "game.h"

static bool show_grid = true;

static void toggle(bool& value, unsigned key) {
	if(hkey == key)
		value = !value;
}

static void show_images(sprite* ps, point size) {
	if(!ps)
		return;
	pushrect push;
	pushfore push_fore(colors::gray);
	int origin = 0, current = 0, maximum = ps->count;
	auto per_line = (canvas->width - 8) / size.x;
	auto per_page = (canvas->height - 8) / size.y;
	while(ismodal()) {
		if(current >= maximum)
			current = maximum - 1;
		if(current < 0)
			current = 0;
		if(current > origin + per_page * per_line)
			origin = (current / per_line - (per_page - 1)) * per_line;
		if(current < origin)
			origin = (current / per_line) * per_line;
		fillform();
		for(auto y = 0; y < per_page; y++) {
			for(auto x = 0; x < per_line; x++) {
				auto index = origin + y * per_line + x;
				if(index >= maximum)
					break;
				caret.x = 4 + x * size.x + 1;
				caret.y = 4 + y * size.y + 1;
				width = size.x - 2;
				height = size.y - 2;
				if(show_grid)
					rectb();
				image(caret.x + width / 2, caret.y + height / 2, ps, index, 0);
				if(index == current) {
					pushfore push(colors::white);
					setoffset(1, 1);
					rectb();
				}
			}
		}
		domodal();
		toggle(show_grid, 'G');
		switch(hkey) {
		case KeyEscape: breakmodal(0); break;
		case KeyRight: current++; break;
		case KeyLeft: current--; break;
		case KeyUp: current -= per_line; break;
		case KeyDown: current += per_line; break;
		default: break;
		}
	}
}

static void images_command() {
	auto id = (resid)hparam;
	show_images(gres(id), {64, 64});
}

void debug_keys() {
	switch(hkey) {
	case Ctrl + 'F': execute(images_command, ResFeatures); break;
	default: break;
	}
}