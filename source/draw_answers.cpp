#include "answers.h"
#include "draw.h"

void answers::paintanswers(fnabutton paintcell, int columns, const char* cancel_text) const {
	auto column_width = width;
	if(columns > 1)
		column_width = (column_width - (metrics::border * 2 + metrics::padding) * (columns - 1)) / columns + 1;
	auto index = 0;
	auto y1 = (int)caret.y, x1 = (int)caret.x;
	auto y2 = (int)caret.y;
	auto push_width_normal = width;
	auto push_x2 = caret.x + width;
	width = column_width;
	pushfore push;
	for(auto& e : elements) {
		fore = push.fore;
		if(e.value <= 0)
			fore = fore.mix(colors::header, 128);
		paintcell(index, e.value, e.text);
		caret.y += height + metrics::padding;
		fire(buttonparam, (long)e.value);
		index++;
		if(caret.y > y2)
			y2 = caret.y;
		if(columns > 1) {
			auto current_column = index % columns;
			width = column_width;
			if(current_column == 0) {
				y1 = caret.y;
				caret.x = x1;
			} else {
				caret.y = y1;
				caret.x += width + metrics::border * 2 + metrics::padding;
				if(current_column == columns - 1)
					width = push_x2 - caret.x;
			}
		}
	}
	caret.x = x1; caret.y = y2;
	width = push_width_normal;
	if(cancel_text) {
		fore = fore.mix(colors::header, 128);
		paintcell(-1, 0, cancel_text);
		fire(buttonparam, 0);
		caret.y += height + metrics::padding;
	}
}