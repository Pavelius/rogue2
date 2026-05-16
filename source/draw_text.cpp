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