#include "draw.h"

const int glyph_start = 32;
const int glyph_count = 256 - glyph_start;

void glyph(int n, unsigned flags) {
	if(n <= glyph_start)
		return;
	pushfore push;
	image(font, n - glyph_start, 0);
	if(flags & TextBold) {
		fore = fore_stroke;
		image(font, n - glyph_start + glyph_count * 1, 0);
	}
	if(flags & TextItalic) {
		fore = fore_stroke;
		image(font, n - glyph_start + glyph_count * 2, 0);
	}
}

int textw(int n) {
	if(n <= glyph_start)
		n = 'l';
	auto widths = (short*)font->ptr(font->size - glyph_count * 2);
	return widths[n - glyph_start];
}

int texth() {
	if(!font)
		return 0;
	return font->height;
}