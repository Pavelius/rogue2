#include "draw.h"
#include "math.h"

#ifndef __GNUC__
#pragma optimize("t", on)
#define NOEXEPT noexcept(true)
#else
using size_t = decltype(sizeof(0));
#define NOEXEPT
#endif

extern "C" void* malloc(unsigned long size);
extern "C" void* realloc(void *ptr, unsigned long size);
extern "C" void* memcpy(void* destination, const void* source, size_t size) NOEXEPT;

extern "C" void exit(int code);
extern "C" void free(void* pointer);

// Default theme colors
color colors::active;
color colors::button;
color colors::form;
color colors::window;
color colors::text;
color colors::border;
color colors::header;
color colors::special;
color colors::tips::text;
color colors::tips::back;

long hparam, hparam2;
const void* hobject;
color fore, fore_stroke; // Current context colors
point hmouse = {-5000, -5000}, dragmouse, caret, camera, text_next, tips_pos; // Current context points (camera, caret, tips)
int	width, height, dialog_width = 500, fsize = 32;
unsigned hkey;
const sprite* font;
rect hilite, clipping, sys_static_area;
fnevent	domodal, ptips, pbeforemodal, pleavemodal;
unsigned char alpha = 255;
const void* hilite_object;
cursor hcursor;
bool hpressed;
bool text_clipped;
bool button_pressed, button_executed, button_hilited, input_disabled;
double linw = 1.0;
color* palt;
fnevent last_scene;
char tips_text[2048];

static surface default_surface;
surface* canvas = &default_surface;

int	metrics::padding = 2, metrics::border = 4;
sprite* metrics::font;
sprite* metrics::h1;
sprite* metrics::h2;
sprite* metrics::h3;
sprite* metrics::small;
sprite* metrics::icons;

static bool	break_modal;
static long break_result;
static bool drag_active;
static fnevent next_proc;

extern rect	sys_static_area;

static void correct(int& x1, int& y1, int& x2, int& y2) {
	if(x1 > x2)
		iswap(x1, x2);
	if(y1 > y2)
		iswap(y1, y2);
}

static bool correct(int& x1, int& y1, int& x2, int& y2, const rect& clip, bool include_edge = true) {
	correct(x1, y1, x2, y2);
	if(x2 < clip.x1 || x1 >= clip.x2 || y2 < clip.y1 || y1 >= clip.y2)
		return false;
	if(x1 < clip.x1)
		x1 = clip.x1;
	if(y1 < clip.y1)
		y1 = clip.y1;
	if(include_edge) {
		if(x2 > clip.x2)
			x2 = clip.x2;
		if(y2 > clip.y2)
			y2 = clip.y2;
	} else {
		if(x2 >= clip.x2)
			x2 = clip.x2 - 1;
		if(y2 >= clip.y2)
			y2 = clip.y2 - 1;
	}
	return true;
}

static bool corrects(const surface& dc, int& x, int& y, int& width, int& height) {
	if(x + width > dc.width)
		width = dc.width - x;
	if(y + height > dc.height)
		height = dc.height - y;
	if(width <= 0 || height <= 0)
		return false;
	return true;
}

static bool correctb(int& x1, int& y1, int& w, int& h, int& ox) {
	int x11 = x1;
	int x2 = x1 + w;
	int y2 = y1 + h;
	if(!correct(x1, y1, x2, y2, clipping))
		return false;
	ox = x1 - x11;
	w = x2 - x1;
	h = y2 - y1;
	return true;
}

static void set32(color* p, unsigned count) {
	color* p2;
	switch(alpha) {
	case 0: // Opaque image
		break;
	case 255:
#ifdef _MSC_VER
		_asm {
			mov edi, p
			mov eax, fore
			mov ecx, count
			rep stosd
		}
#else
		p2 = p + count;
		while(p < p2)
			*p++ = fore;
#endif
		break;
	case 128:
		p2 = p + count;
		while(p < p2) {
			p->r = (p->r + fore.r) >> 1;
			p->g = (p->g + fore.g) >> 1;
			p->b = (p->b + fore.b) >> 1;
			p++;
		}
		break;
	default:
		p2 = p + count;
		while(p < p2) {
			p->r = (p->r * (255 - alpha) + fore.r * alpha) >> 8;
			p->g = (p->g * (255 - alpha) + fore.g * alpha) >> 8;
			p->b = (p->b * (255 - alpha) + fore.b * alpha) >> 8;
			p++;
		}
		break;
	}
}

static void set32(color* p) {
	switch(alpha) {
	case 0:
		break;
	case 255:
		p[0] = fore;
		break;
	case 128:
		p->r = (p->r + fore.r) >> 1;
		p->g = (p->g + fore.g) >> 1;
		p->b = (p->b + fore.b) >> 1;
		break;
	default:
		p->r = (p->r * (255 - alpha) + fore.r * alpha) >> 8;
		p->g = (p->g * (255 - alpha) + fore.g * alpha) >> 8;
		p->b = (p->b * (255 - alpha) + fore.b * alpha) >> 8;
		break;
	}
}

static void set32x(unsigned char* d, int d_scan, int width, int height) {
	while(height-- > 0) {
		set32((color*)d, width);
		d += d_scan;
	}
}

static void set32h(unsigned char* d, int height) {
	while(height-- > 0) {
		set32((color*)d);
		d += canvas->scanline;
	}
}

static void raw832(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, const color* pallette) {
	const int cbd = 4;
	while(height-- > 0) {
		unsigned char* p1 = d;
		unsigned char* sb = s;
		unsigned char* se = s + width;
		while(sb < se) {
			*((color*)p1) = pallette[*sb++];
			p1 += cbd;
		}
		s += s_scan;
		d += d_scan;
	}
}

static void raw832m(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, const color* pallette) {
	const int cbd = 4;
	while(height-- > 0) {
		unsigned char* p1 = d;
		unsigned char* sb = s;
		unsigned char* se = s + width;
		while(sb < se) {
			*((color*)p1) = pallette[*sb++];
			p1 -= cbd;
		}
		s += s_scan;
		d += d_scan;
	}
}

static void raw32(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height) {
	const int cbs = 3;
	const int cbd = 4;
	if(width <= 0)
		return;
	if(!alpha)
		return;
	if(alpha == 255) {
		while(height-- > 0) {
			unsigned char* sb = s;
			unsigned char* se = s + width * cbs;
			unsigned char* p1 = d;
			while(sb < se) {
				p1[0] = sb[0];
				p1[1] = sb[1];
				p1[2] = sb[2];
				sb += cbs;
				p1 += cbd;
			}
			s += s_scan;
			d += d_scan;
		}
	} else {
		while(height-- > 0) {
			unsigned char* sb = s;
			unsigned char* se = s + width * cbs;
			unsigned char* p1 = d;
			while(sb < se) {
				p1[0] = (((int)p1[0] * (255 - alpha)) + ((sb[0]) * (alpha))) >> 8;
				p1[1] = (((int)p1[1] * (255 - alpha)) + ((sb[1]) * (alpha))) >> 8;
				p1[2] = (((int)p1[2] * (255 - alpha)) + ((sb[2]) * (alpha))) >> 8;
				sb += cbs;
				p1 += cbd;
			}
			s += s_scan;
			d += d_scan;
		}
	}
}

static void raw32m(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height) {
	const int cbs = 3;
	const int cbd = 4;
	if(width <= 0)
		return;
	while(height-- > 0) {
		unsigned char* sb = s;
		unsigned char* se = s + width * cbs;
		unsigned char* p1 = d;
		while(sb < se) {
			p1[0] = sb[0];
			p1[1] = sb[1];
			p1[2] = sb[2];
			sb += cbs;
			p1 -= cbd;
		}
		s += s_scan;
		d += d_scan;
	}
}

// (00) end of line
// (01 - 7F) draw count of pixels
// (80, XX, AA) draw count of XX bytes of alpha AA pixels
// (81 - 9F, AA) draw count of (B-0xC0) bytes of alpha AA pixels
// (A0, XX) skip count of XX pixels
// A1 - FF skip count of (b-0xB0) pixels
// each pixel has b,g,r value
static void rle32(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha) {
	const int cbs = 3;
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			p1 += d1;
			s1 += d1;
			s2 += d1;
			d = p1;
			if(--h == 0)
				break;
		} else if(c <= 0x9F) {
			unsigned char ap, cb;
			// count
			if(c <= 0x7F) {
				cb = c;
				ap = 0xFF;
			} else if(c == 0x80) {
				cb = *s++;
				ap = *s++;
			} else {
				cb = c - 0x80;
				ap = *s++;
			}
			// clip left invisible part
			if(d + cb * cbd <= s1 || d > s2) {
				d += cb * cbd;
				s += cb * cbs;
				continue;
			} else if(d < s1) {
				unsigned char sk = (unsigned char)((s1 - d) / cbd);
				d += sk * cbd;
				s += sk * cbs;
				cb -= sk;
			}
			// visible part
			if(ap == 0xFF && alpha == 0xFF) {
				do {
					if(d >= s2)
						break;
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
					s += cbs;
					d += cbd;
				} while(--cb);
			} else {
				ap = (ap * alpha) / 256;
				do {
					if(d >= s2)
						break;
					d[0] = (((int)d[0] * (255 - ap)) + ((s[0]) * (ap))) >> 8;
					d[1] = (((int)d[1] * (255 - ap)) + ((s[1]) * (ap))) >> 8;
					d[2] = (((int)d[2] * (255 - ap)) + ((s[2]) * (ap))) >> 8;
					s += cbs;
					d += cbd;
				} while(--cb);
			}
			// right clip part
			if(cb) {
				s += cb * cbs;
				d += cb * cbd;
			}
		} else {
			if(c == 0xA0)
				d += (*s++) * cbd;
			else
				d += (c - 0xA0) * cbd;
		}
	}
}

static void rle32m(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha) {
	const int cbs = 3;
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			p1 += d1;
			s1 += d1;
			s2 += d1;
			d = p1;
			if(--h == 0)
				break;
		} else if(c <= 0x9F) {
			unsigned char ap, cb;
			// count
			if(c <= 0x7F) {
				cb = c;
				ap = 0xFF;
			} else if(c == 0x80) {
				cb = *s++;
				ap = *s++;
			} else {
				cb = c - 0x80;
				ap = *s++;
			}
			// clip left invisible part
			if(d - (cb * cbd) >= s2 || d < s1) {
				s += cb * cbs;
				d -= cb * cbd;
				continue;
			} else if(d >= s2) {
				unsigned char sk = (unsigned char)(1 + (d - s2) / cbd);
				d -= sk * cbd;
				s += sk * cbs;
				cb -= sk;
				if(!cb)
					continue;
			}
			// visible part
			if(ap == 0xFF && alpha == 0xFF) {
				// no alpha or modification
				do {
					if(d < s1)
						break;
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
					s += cbs;
					d -= cbd;
				} while(--cb);
			} else {
				// alpha channel
				ap = (ap * alpha) / 256;
				do {
					if(d < s1)
						break;
					d[0] = (((int)d[0] * (255 - ap)) + ((s[0]) * (ap))) >> 8;
					d[1] = (((int)d[1] * (255 - ap)) + ((s[1]) * (ap))) >> 8;
					d[2] = (((int)d[2] * (255 - ap)) + ((s[2]) * (ap))) >> 8;
					d -= cbd;
					s += cbs;
				} while(--cb);
			}
			// right clip part
			if(cb) {
				d -= cb * cbd;
				s += cb * cbs;
			}
		} else {
			if(c == 0xA0)
				d -= (*s++) * cbd;
			else
				d -= (c - 0xA0) * cbd;
		}
	}
}

static void rle832(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha, const color* pallette) {
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			p1 += d1;
			s1 += d1;
			s2 += d1;
			if(--h == 0)
				break;
			d = p1;
		} else if(c <= 0x9F) {
			unsigned char ap = alpha, cb;
			bool need_correct_s = false;
			// count
			if(c <= 0x7F) {
				need_correct_s = true;
				cb = c;
			} else if(c == 0x80) {
				cb = *s++;
				ap >>= 1;
			} else {
				cb = c - 0x80;
				ap >>= 1;
			}
			// clip left invisible part
			if(d + cb * cbd <= s1 || d > s2) {
				d += cb * cbd;
				if(need_correct_s)
					s += cb;
				continue;
			} else if(d < s1) {
				unsigned char sk = (unsigned char)((s1 - d) / cbd);
				d += sk * cbd;
				if(need_correct_s)
					s += sk;
				cb -= sk;
			}
			// visible part
			if(ap == alpha) {
				if(ap == 0xFF) {
					do {
						if(d >= s2)
							break;
						*((color*)d) = pallette[*s++];
						d += cbd;
					} while(--cb);
				} else {
					do {
						if(d >= s2)
							break;
						unsigned char* s1 = (unsigned char*)&pallette[*s++];
						d[0] = (((int)d[0] * (255 - ap)) + ((s1[0]) * (ap))) >> 8;
						d[1] = (((int)d[1] * (255 - ap)) + ((s1[1]) * (ap))) >> 8;
						d[2] = (((int)d[2] * (255 - ap)) + ((s1[2]) * (ap))) >> 8;
						d += cbd;
					} while(--cb);
				}
			} else if(ap == 0x7F) {
				do {
					if(d >= s2)
						break;
					d[0] >>= 1;
					d[1] >>= 1;
					d[2] >>= 1;
					d += cbd;
				} while(--cb);
			} else {
				ap = 255 - ap;
				do {
					if(d >= s2)
						break;
					d[0] = (((int)d[0] * ap)) >> 8;
					d[1] = (((int)d[1] * ap)) >> 8;
					d[2] = (((int)d[2] * ap)) >> 8;
					d += cbd;
				} while(--cb);
			}
			// right clip part
			if(cb) {
				if(need_correct_s)
					s += cb;
				d += cb * cbd;
			}
		} else {
			if(c == 0xA0)
				d += (*s++) * cbd;
			else
				d += (c - 0xA0) * cbd;
		}
	}
}

static void rle832m(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha, const color* pallette) {
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			p1 += d1;
			s1 += d1;
			s2 += d1;
			d = p1;
			if(--h == 0)
				break;
		} else if(c <= 0x9F) {
			unsigned char ap = alpha, cb;
			bool need_correct_s = false;
			// count
			if(c <= 0x7F) {
				need_correct_s = true;
				cb = c;
			} else if(c == 0x80) {
				cb = *s++;
				ap >>= 1;
			} else {
				cb = c - 0x80;
				ap >>= 1;
			}
			// clip left invisible part
			if(d - (cb * cbd) >= s2 || d < s1) {
				d -= cb * cbd;
				if(need_correct_s)
					s += cb;
				continue;
			} else if(d >= s2) {
				unsigned char sk = (unsigned char)((d - s2) / cbd);
				d -= sk * cbd;
				if(need_correct_s)
					s += sk;
				cb -= sk;
			}
			// visible part
			if(ap == alpha) {
				if(ap == 0xFF) {
					do {
						if(d < s1)
							break;
						*((color*)d) = pallette[*s++];
						d -= cbd;
					} while(--cb);
				} else {
					do {
						if(d < s1)
							break;
						unsigned char* s1 = (unsigned char*)&pallette[*s++];
						d[0] = (((int)d[0] * (255 - ap)) + ((s1[0]) * (ap))) >> 8;
						d[1] = (((int)d[1] * (255 - ap)) + ((s1[1]) * (ap))) >> 8;
						d[2] = (((int)d[2] * (255 - ap)) + ((s1[2]) * (ap))) >> 8;
						d -= cbd;
					} while(--cb);
				}
			} else if(ap == 0x7F) {
				do {
					if(d < s1)
						break;
					d[0] >>= 1;
					d[1] >>= 1;
					d[2] >>= 1;
					d -= cbd;
				} while(--cb);
			} else {
				ap = 255 - ap;
				do {
					if(d < s1)
						break;
					d[0] = (((int)d[0] * ap)) >> 8;
					d[1] = (((int)d[1] * ap)) >> 8;
					d[2] = (((int)d[2] * ap)) >> 8;
					d -= cbd;
				} while(--cb);
			}
			// right clip part
			if(cb) {
				if(need_correct_s)
					s += cb;
				d -= cb * cbd;
			}
		} else {
			if(c == 0xA0)
				d -= (*s++) * cbd;
			else
				d -= (c - 0xA0) * cbd;
		}
	}
}

static void alc32(unsigned char* d, int d_scan, const unsigned char* s, int height, const unsigned char* clip_x1, const unsigned char* clip_x2, color c1, bool italic) {
	const int cbs = 3;
	const int cbd = 4;
	unsigned char* p = d;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			d += d_scan;
			clip_x1 += d_scan;
			clip_x2 += d_scan;
			if(italic && (height & 1) != 0)
				d -= cbd;
			p = d;
			if(--height == 0)
				break;
		} else if(c <= 0x7F) {
			// clip left invisible part
			if(p + (c * cbd) <= clip_x1 || p > clip_x2) {
				p += c * cbd;
				s += c * cbs;
				continue;
			} else if(p < clip_x1) {
				unsigned char sk = (unsigned char)((clip_x1 - p) / cbd);
				p += sk * cbd;
				s += sk * cbs;
				c -= sk;
			}
			// visible part
			if(alpha == 255) {
				do {
					if(p >= clip_x2)
						break;
					p[0] = ((p[0] * (255 - s[0])) + (c1.b * (s[0]))) >> 8;
					p[1] = ((p[1] * (255 - s[1])) + (c1.g * (s[1]))) >> 8;
					p[2] = ((p[2] * (255 - s[2])) + (c1.r * (s[2]))) >> 8;
					p += cbd;
					s += cbs;
				} while(--c);
			} else {
				do {
					if(p >= clip_x2)
						break;
					unsigned char b = (s[0] * alpha) >> 8;
					unsigned char g = (s[1] * alpha) >> 8;
					unsigned char r = (s[2] * alpha) >> 8;
					p[0] = ((p[0] * (255 - b)) + (c1.b * b)) >> 8;
					p[1] = ((p[1] * (255 - g)) + (c1.g * g)) >> 8;
					p[2] = ((p[2] * (255 - r)) + (c1.r * r)) >> 8;
					p += cbd;
					s += cbs;
				} while(--c);
			}
			// right clip part
			if(c) {
				p += c * cbd;
				s += c * cbs;
			}
		} else {
			if(c == 0x80)
				p += (*s++) * cbd;
			else
				p += (c - 0x80) * cbd;
		}
	}
}

static void alc132(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha) {
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	auto fb = fore.b;
	auto fg = fore.g;
	auto fr = fore.r;
	auto ap = alpha;
	if(fore.a && fore.a != 255)
		ap = (unsigned char)((ap * fore.a) >> 8);
	if(!ap)
		return;
	while(true) {
		unsigned char c = *s++;
		if(c == 0xFF) {
			p1 += d1;
			s1 += d1;
			s2 += d1;
			if(--h == 0)
				break;
			d = p1;
			continue; // New line
		}
		d += c * cbd; // Skip c bytes
		c = *s++;
		if(c == 0) {
			// Skip visible part
			continue;
		} else if(d >= s2 || (d + c * cbd) <= s1) {
			d += c * cbd; // Total invisible
			continue;
		} else if(d < s1) {
			auto n = (s1 - d) / cbd;
			c -= n;
			d += n * cbd;
		} else if(d + c * cbd > s2)
			c = (s2 - d) / cbd;
		if(ap >= 255) {
			while(c--) {
				*((color*)d) = fore;
//				d[0] = fb;
//				d[1] = fg;
//				d[2] = fr;
				d += cbd;
			}
		} else {
			while(c--) {
				d[0] = (((int)d[0] * (255 - ap)) + ((fb) * (ap))) >> 8;
				d[1] = (((int)d[1] * (255 - ap)) + ((fg) * (ap))) >> 8;
				d[2] = (((int)d[2] * (255 - ap)) + ((fr) * (ap))) >> 8;
				d += cbd;
			}
		}
	}
}

static unsigned char* skip_v3(unsigned char* s, int h) {
	const int cbs = 1;
	if(!s || !h)
		return s;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			if(--h == 0)
				return s;
		} else if(c <= 0x9F) {
			if(c <= 0x7F)
				s += c * cbs;
			else {
				if(c == 0x80)
					c = *s++;
				else
					c -= 0x80;
				s++;
				s += c * cbs;
			}
		} else if(c == 0xA0)
			s++;
	}
}

static unsigned char* skip_rle32(unsigned char* s, int h) {
	const int cbs = 3;
	if(!s || !h)
		return s;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			if(--h == 0)
				return s;
		} else if(c <= 0x9F) {
			if(c <= 0x7F)
				s += c * cbs;
			else {
				if(c == 0x80)
					c = *s++;
				else
					c -= 0x80;
				s++;
				s += c * cbs;
			}
		} else if(c == 0xA0)
			s++;
	}
}

static unsigned char* skip_alc(unsigned char* s, int h) {
	const int cbs = 3;
	if(!s || !h)
		return s;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			if(--h == 0)
				return s;
		} else if(c <= 0x7F)
			s += c * cbs;
		else if(c == 0x80)
			s++;
	}
}

static void scale_line_32(unsigned char* dst, unsigned char* src, int sw, int tw) {
	const int cbd = 4;
	int NumPixels = tw;
	int IntPart = (sw / tw) * cbd;
	int FractPart = sw % tw;
	int E = 0;
	while(NumPixels-- > 0) {
		*((unsigned*)dst) = *((unsigned*)src);
		dst += cbd;
		src += IntPart;
		E += FractPart;
		if(E >= tw) {
			E -= tw;
			src += cbd;
		}
	}
}

static void scale32(
	unsigned char* d, int d_scan, int d_width, int d_height,
	unsigned char* s, int s_scan, int s_width, int s_height) {
	if(!d_width || !d_height || !s_width || !s_height)
		return;
	const int cbd = 4;
	int NumPixels = d_height;
	int IntPart = (s_height / d_height) * s_scan;
	int FractPart = s_height % d_height;
	int E = 0;
	unsigned char* PrevSource = 0;
	while(NumPixels-- > 0) {
		if(s == PrevSource)
			memcpy(d, d - d_scan, d_width * cbd);
		else {
			scale_line_32(d, s, s_width, d_width);
			PrevSource = s;
		}
		d += d_scan;
		s += IntPart;
		E += FractPart;
		if(E >= d_height) {
			E -= d_height;
			s += s_scan;
		}
	}
}

static void cpy(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, int bytes_per_pixel) {
	if(height <= 0 || width <= 0)
		return;
	int width_bytes = width * bytes_per_pixel;
	do {
		memcpy(d, s, width_bytes);
		s += s_scan;
		d += d_scan;
	} while(--height);
}

static void cpy32a(unsigned char* d, int d_scan,
	const unsigned char* s, int s_scan,
	int width, int height, unsigned char ap) {
	if(height <= 0 || width <= 0)
		return;
	do {
		auto sb = (color*)s;
		auto pe = sb + width;
		for(auto d2 = (color*)d; sb < pe; sb++) {
			d2->r = (((int)d2->r * (255 - ap)) + ((sb->r) * (ap))) >> 8;
			d2->g = (((int)d2->g * (255 - ap)) + ((sb->g) * (ap))) >> 8;
			d2->b = (((int)d2->b * (255 - ap)) + ((sb->b) * (ap))) >> 8;
			d2++;
		}
		s += s_scan;
		d += d_scan;
	} while(--height);
}

static void cpy32t(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height) {
	if(height <= 0 || width <= 0)
		return;
	do {
		color* d2 = (color*)d;
		color* sb = (color*)s;
		color* se = sb + width;
		while(sb < se) {
			if(!sb->a) {
				d2++;
				sb++;
			} else if(sb->a == 0xFF)
				*d2++ = *sb++;
			else {
				auto ap = sb->a;
				d2->r = (((int)d2->r * (255 - ap)) + ((sb->r) * (ap))) >> 8;
				d2->g = (((int)d2->g * (255 - ap)) + ((sb->g) * (ap))) >> 8;
				d2->b = (((int)d2->b * (255 - ap)) + ((sb->b) * (ap))) >> 8;
				d2++; sb++;
			}
		}
		s += s_scan;
		d += d_scan;
	} while(--height);
}

bool dragactive() {
	return drag_active;
}

int getbpp() {
	return canvas ? canvas->bpp : 1;
}

int getwidth() {
	return canvas ? canvas->width : 0;
}

int getheight() {
	return canvas ? canvas->height : 0;
}

unsigned char* ptr(int x, int y) {
	return canvas ? (canvas->bits + y * canvas->scanline + x * canvas->bpp / 8) : 0;
}

void pixel(int x, int y) {
	if(x >= clipping.x1 && x < clipping.x2 && y >= clipping.y1 && y < clipping.y2)
		*((color*)((char*)canvas->bits + y * canvas->scanline + x * 4)) = fore;
}

void pixel(int x, int y, unsigned char a) {
	if(x < clipping.x1 || x >= clipping.x2 || y < clipping.y1 || y >= clipping.y2 || a == 0xFF)
		return;
	color* p = (color*)ptr(x, y);
	if(a == 0)
		*p = fore;
	else {
		p->b = (((unsigned)p->b * (a)) + (fore.b * (255 - a))) >> 8;
		p->g = (((unsigned)p->g * (a)) + (fore.g * (255 - a))) >> 8;
		p->r = (((unsigned)p->r * (a)) + (fore.r * (255 - a))) >> 8;
		p->a = 0;
	}
}

static void linew(int x1, int y1, double wd) {
	int x0 = caret.x, y0 = caret.y;
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx - dy, e2, x2, y2; /* error value e_xy */
	float ed = dx + dy == 0 ? 1 : sqrt((float)dx * dx + (float)dy * dy);
	for(wd = (wd + 1) / 2; ; ) {                                    /* pixel loop */
		pixel(x0, y0, (unsigned char)imax((int)0, (int)(255 * (iabs(err - dx + dy) / ed - wd + 1))));
		e2 = err; x2 = x0;
		if(2 * e2 >= -dx) {                                            /* x step */
			for(e2 += dy, y2 = y0; e2 < ed * wd && (y1 != y2 || dx > dy); e2 += dx)
				pixel(x0, y2 += sy, (unsigned char)imax((int)0, (int)(255 * (iabs(e2) / ed - wd + 1))));
			if(x0 == x1) break;
			e2 = err; err -= dy; x0 += sx;
		}
		if(2 * e2 <= dy) {                                             /* y step */
			for(e2 = dx - e2; e2 < ed * wd && (x1 != x2 || dx < dy); e2 += dy)
				pixel(x2 += sx, y0, (unsigned char)imax((int)0, (int)(255 * (iabs(e2) / ed - wd + 1))));
			if(y0 == y1) break;
			err += dx; y0 += sy;
		}
	}
}

void line(int xt, int yt) {
	int x0 = caret.x, y0 = caret.y, x1 = xt, y1 = yt;
	if(linw != 1.0)
		linew(x1, y1, linw);
	else if(caret.x == x1) {
		if(correct(x0, y0, x1, y1, clipping, false))
			set32h(canvas->ptr(x0, y0), y1 - y0 + 1);
	} else if(caret.y == y1) {
		if(correct(x0, y0, x1, y1, clipping, false))
			set32x(canvas->ptr(x0, y0), canvas->scanline, x1 - x0 + 1, 1);
	} else {
		int x0 = caret.x, y0 = caret.y;
		int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx - dy, e2, x2; // error value e_xy
		int ed = dx + dy == 0 ? 1 : isqrt(dx * dx + dy * dy);
		for(;;) {
			pixel(x0, y0, alpha * iabs(err - dx + dy) / ed);
			e2 = err; x2 = x0;
			if(2 * e2 >= -dx) {// x step
				if(x0 == x1)
					break;
				if(e2 + dy < ed)
					pixel(x0, y0 + sy, alpha * (e2 + dy) / ed);
				err -= dy; x0 += sx;
			}
			if(2 * e2 <= dy) {// y step
				if(y0 == y1)
					break;
				if(dx - e2 < ed)
					pixel(x2 + sx, y0, alpha * (dx - e2) / ed);
				err += dx; y0 += sy;
			}
		}
	}
	caret.x = xt;
	caret.y = yt;
}

void linet(int x1, int y1) {
	int x0 = caret.x, y0 = caret.y;
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2; // error value e_xy
	for(;;) {
		if((x0 + y0) & 1)
			pixel(x0, y0);
		if(x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if(e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0
		if(e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0
	}
	caret.x = x1;
	caret.y = y1;
}

void paint_border_color(fnevent proc) {
	pushfore push(colors::border);
	proc();
}

void paint_active_color(fnevent proc) {
	pushfore push(colors::border);
	proc();
}

void rectb() {
	line(caret.x + (width - 1), caret.y);
	line(caret.x, caret.y + (height - 1));
	line(caret.x - (width - 1), caret.y);
	line(caret.x, caret.y - (height - 1));
}

void rectf() {
	pushrect push;
	int x1 = caret.x, y1 = caret.y, x2 = caret.x + width, y2 = caret.y + height;
	if(correct(x1, y1, x2, y2, clipping))
		set32x(ptr(x1, y1), canvas->scanline, x2 - x1, y2 - y1);
}

void setpos(int x, int y) {
	caret.x = x;
	caret.y = y;
}

void setpos(int x, int y, int w, int h) {
	caret.x = x;
	caret.y = y;
	width = w;
	height = h;
}

void setoffset(int x, int y) {
	caret.x += x; width -= x * 2;
	caret.y += y; height -= y * 2;
}

void rectx() {
	linet(caret.x, caret.y + height);
	linet(caret.x + width, caret.y);
	linet(caret.x, caret.y - height);
	linet(caret.x - width, caret.y);
}

void rectfocus() {
	pushrect push;
	setoffset(1, 1);
	rectx();
}

void gradv(const color c1, const color c2, int skip) {
	if(!canvas)
		return;
	if(skip > height)
		skip = height;
	float k3 = (float)height;
	if(!k3)
		return;
	int y0 = caret.y;
	int y2 = caret.y + height;
	auto pf = fore;
	for(int y1 = caret.y + skip; y1 < y2; y1++) {
		auto k2 = (float)(y1 - y0) / k3;
		auto k1 = 1.00f - k2;
		fore.r = (unsigned char)(c1.r * k1 + c2.r * k2);
		fore.g = (unsigned char)(c1.g * k1 + c2.g * k2);
		fore.b = (unsigned char)(c1.b * k1 + c2.b * k2);
		if(y1 >= clipping.y2 || y1 < clipping.y1)
			continue;
		set32((color*)canvas->ptr(caret.x, y1), width);
	}
	fore = pf;
}

void gradh(const color c1, const color c2, int skip) {
	if(!canvas)
		return;
	if(skip > width)
		skip = width;
	float k3 = (float)width;
	if(!k3)
		return;
	int x0 = caret.x;
	int x2 = caret.x + width;
	auto pf = fore;
	for(int x1 = caret.x + skip; x1 < x2; x1++) {
		auto k2 = (float)(x0 - x1) / k3;
		auto k1 = 1.00f - k2;
		fore.r = (unsigned char)(c1.r * k1 + c2.r * k2);
		fore.g = (unsigned char)(c1.g * k1 + c2.g * k2);
		fore.b = (unsigned char)(c1.b * k1 + c2.b * k2);
		if(x1 >= clipping.x2 || x1 < clipping.x1)
			continue;
		set32h(canvas->ptr(x1, caret.y), height);
	}
	fore = pf;
}

static void set32fl(int x, int y, int w) {
	if(y < clipping.y1 || y >= clipping.y2)
		return;
	auto x2 = x + w;
	if(x < clipping.x1)
		x = clipping.x1;
	if(x2 > clipping.x2)
		x2 = clipping.x2;
	w = x2 - x;
	if(w > 0)
		set32((color*)canvas->ptr(x, y), w);
}

void circlef(int r) {
	int xm = caret.x, ym = caret.y;
	if(xm - r >= clipping.x2 || xm + r < clipping.x1 || ym - r >= clipping.y2 || ym + r < clipping.y1)
		return;
	int x = -r, y = 0, err = 2 - 2 * r, y2 = -1000;
	do {
		if(y2 != y) {
			y2 = y;
			set32fl(xm + x, ym + y, x * -2);
			if(y != 0)
				set32fl(xm + x, ym - y, x * -2);
		}
		r = err;
		if(r <= y)
			err += ++y * 2 + 1;
		if(r > x || err > y)
			err += ++x * 2 + 1;
	} while(x < 0);
}

void circle(int r) {
	int xm = caret.x, ym = caret.y;
	int x = r, y = 0; // II. quadrant from bottom left to top right
	int x2, e2, err = 2 - 2 * r; // error of 1.step
	r = 1 - err;
	for(;;) {
		int i = 255 * iabs(err + 2 * (x + y) - 2) / r; // get blend value of pixel
		pixel(xm + x, ym - y, i); // I. Quadrant
		pixel(xm + y, ym + x, i); // II. Quadrant
		pixel(xm - x, ym + y, i); // III. Quadrant
		pixel(xm - y, ym - x, i); // IV. Quadrant
		if(x == 0)
			break;
		e2 = err; x2 = x; // remember values
		if(err > y) { // x step
			int i = 255 * (err + 2 * x - 1) / r; // outward pixel
			if(i < 255) {
				pixel(xm + x, ym - y + 1, i);
				pixel(xm + y - 1, ym + x, i);
				pixel(xm - x, ym + y - 1, i);
				pixel(xm - y + 1, ym - x, i);
			}
			err -= --x * 2 - 1;
		}
		if(e2 <= x2--) { // y step
			int i = 255 * (1 - 2 * y - e2) / r;
			if(i < 255) {
				pixel(xm + x2, ym - y, i);
				pixel(xm + y, ym + x2, i);
				pixel(xm - x2, ym + y, i);
				pixel(xm - y, ym - x2, i);
			}
			err -= --y * 2 - 1;
		}
	}
}

void fhexagon() {
	const double cos_30 = 0.86602540378;
	point points[6] = {
		{(short)(caret.x + fsize), caret.y},
		{(short)(caret.x + fsize / 2), (short)(caret.y + fsize * cos_30)},
		{(short)(caret.x - fsize / 2), (short)(caret.y + fsize * cos_30)},
		{(short)(caret.x - fsize), caret.y},
		{(short)(caret.x - fsize / 2), (short)(caret.y - fsize * cos_30)},
		{(short)(caret.x + fsize / 2), (short)(caret.y - fsize * cos_30)},
	};
	auto push_caret = caret;
	caret = points[0];
	for(auto i = 1; i < 6; i++)
		line(points[i].x, points[i].y);
	line(points[0].x, points[0].y);
	caret = push_caret;
}

void hexagon() {
	const double cos_30 = 0.86602540378;
	point points[6] = {
		{(short)(caret.x + fsize * cos_30), (short)(caret.y - fsize / 2)},
		{(short)(caret.x + fsize * cos_30), (short)(caret.y + fsize / 2)},
		{(short)caret.x, (short)(caret.y + fsize)},
		{(short)(caret.x - fsize * cos_30), (short)(caret.y + fsize / 2)},
		{(short)(caret.x - fsize * cos_30), (short)(caret.y - fsize / 2)},
		{(short)caret.x, (short)(caret.y - fsize)},
	};
	auto push_caret = caret;
	caret = points[0];
	for(auto i = 1; i < 6; i++)
		line(points[i].x, points[i].y);
	line(points[0].x, points[0].y);
	caret = push_caret;
}

void setclip(rect v) {
	if(clipping.x1 < v.x1)
		clipping.x1 = v.x1;
	if(clipping.y1 < v.y1)
		clipping.y1 = v.y1;
	if(clipping.x2 > v.x2)
		clipping.x2 = v.x2;
	if(clipping.y2 > v.y2)
		clipping.y2 = v.y2;
}

static void intersect_rect(rect& r1, const rect& r2) {
	if(!r1.intersect(r2))
		return;
	if(hmouse.in(r2)) {
		if(r2.y1 > r1.y1)
			r1.y1 = r2.y1;
		if(r2.x1 > r1.x1)
			r1.x1 = r2.x1;
		if(r2.y2 < r1.y2)
			r1.y2 = r2.y2;
		if(r2.x2 < r1.x2)
			r1.x2 = r2.x2;
	} else {
		if(hmouse.y > r2.y2 && r2.y2 > r1.y1)
			r1.y1 = r2.y2;
		else if(hmouse.y < r2.y1 && r2.y1 < r1.y2)
			r1.y2 = r2.y1;
		else if(hmouse.x > r2.x2 && r2.x2 > r1.x1)
			r1.x1 = r2.x2;
		else if(hmouse.x < r2.x1 && r2.x1 < r1.x2)
			r1.x2 = r2.x1;
	}
}

bool ishilite(const rect& rc) {
	if(hkey == InputNoUpdate)
		return false;
	intersect_rect(sys_static_area, rc);
	if(!hmouse.in(clipping))
		return false;
	if(hmouse.in(rc)) {
		hilite = rc;
		return true;
	}
	return false;
}

int	aligned(int x, int width, unsigned flags, int dx) {
	switch(flags & AlignMask) {
	case AlignRightBottom:
	case AlignRightCenter:
	case AlignRight: return x + width - dx;
	case AlignCenterBottom:
	case AlignCenterCenter:
	case AlignCenter: return x + (width - dx) / 2;
	default: return x;
	}
}

static int alignedh(const char* string, unsigned state) {
	int ty;
	switch(state & AlignMask) {
	case AlignCenterCenter:
	case AlignRightCenter:
	case AlignLeftCenter:
		if(state & TextSingleLine)
			ty = texth();
		else
			ty = texth(string, width);
		return (height - ty) / 2;
	case AlignCenterBottom:
	case AlignRightBottom:
	case AlignLeftBottom:
		if(state & TextSingleLine)
			ty = texth();
		else
			ty = texth(string, width);
		return height - ty;
	default:
		return 0;
	}
}

const char* skiptr(const char* string) {
	// skiping trail symbols
	for(; *string && *string == 0x20; string++);
	if(*string == 13) {
		string++;
		if(*string == 10)
			string++;
	} else if(*string == 10) {
		string++;
		if(*string == 13)
			string++;
	}
	return string;
}

int texth(const char* string, int width) {
	int dy = texth();
	int y1 = 0;
	while(*string) {
		int c = textbc(string, width);
		if(!c)
			break;
		y1 += dy;
		string = skiptr(string + c);
	}
	return y1;
}

int textw(const char* string, int count) {
	if(!font)
		return 0;
	auto result = 0;
	if(count == -1) {
		for(auto p = string; *p; p++)
			result += textw((unsigned char)*p);
	} else {
		auto pe = string + count;
		for(auto p = string; p < pe; p++)
			result += textw((unsigned char)*p);
	}
	return result;
}

void text(const char* string, int count, unsigned flags) {
	if(!font || caret.y >= clipping.y2 || caret.y + texth() < clipping.y1)
		return;
	auto push_caret = caret;
	const char *s1 = string;
	if(count == -1) {
		while(*s1) {
			unsigned char s = *s1++;
			glyph(s, flags);
			caret.x += textw(s);
		}
	} else {
		auto s2 = string + count;
		while(s1 < s2) {
			unsigned char s = *s1++;
			glyph(s, flags);
			caret.x += textw(s);
		}
	}
	text_next = caret;
	caret = push_caret;
}

void textc(const char* string, int count, unsigned flags) {
	auto push_clip = clipping;
	setclip({caret.x, caret.y, caret.x + width, caret.y + texth()});
	auto w = textw(string, count);
	text_clipped = w > width;
	auto push_caret = caret;
	caret.x = aligned(caret.x, width, flags, w);
	text(string, count, flags);
	caret = push_caret;
	clipping = push_clip;
	caret.y += texth();
}

int textbc(const char* string, int width) {
	if(!font)
		return 0;
	int p = -1;
	int w = 0;
	const char* s1 = string;
	while(true) {
		unsigned char s = *s1++;
		if(s == 0x20 || s == 9) {
			p = s1 - string - 1;
		} else if(s == 0) {
			p = s1 - string - 1;
			break;
		} else if(s == 10 || s == 13) {
			p = s1 - string - 1;
			break;
		}
		w += textw(s);
		if(w > width)
			break;
	}
	if(p == -1)
		p = s1 - string;
	return p;
}

void textas(const char* string) {
	auto mx = 0, my = 0;
	while(string[0]) {
		auto c = textbc(string, width);
		if(!c)
			break;
		auto m = textw(string, c);
		if(mx < m)
			mx = m;
		my += texth();
		string = skiptr(string + c);
	}
	width = mx;
	height = my;
}

void texta(const char* string, unsigned state) {
	if(!string || string[0] == 0)
		return;
	auto push_caret = caret;
	auto y2 = caret.y + height;
	caret.y += alignedh(string, state);
	if(state & TextSingleLine) {
		auto push_clip = clipping; setclip(getrect());
		caret.x = aligned(caret.x, width, state, textw(string));
		text(string, -1, state);
		clipping = push_clip;
		caret.y += texth();
	} else {
		auto dy = texth();
		while(caret.y < y2) {
			int c = textbc(string, width);
			if(!c)
				break;
			int w = textw(string, c);
			caret.x = aligned(push_caret.x, width, state, w);
			text(string, c, state);
			caret.y += dy;
			string = skiptr(string + c);
		}
	}
	caret = push_caret;
}

void image(int x, int y, const sprite* e, int id, int flags) {
	int x2, y2;
	color* pal;
	if(!e)
		return;
	const sprite::frame& f = e->get(id);
	if(!f.offset)
		return;
	if(!canvas)
		return;
	if(flags & ImageMirrorH) {
		x2 = x;
		if((flags & ImageNoOffset) == 0)
			x2 += f.ox;
		x = x2 - f.sx;
	} else {
		if((flags & ImageNoOffset) == 0)
			x -= f.ox;
		x2 = x + f.sx;
	}
	if(flags & ImageMirrorV) {
		y2 = y;
		if((flags & ImageNoOffset) == 0)
			y2 += f.oy;
		y = y2 - f.sy;
	} else {
		if((flags & ImageNoOffset) == 0)
			y -= f.oy;
		y2 = y + f.sy;
	}
	unsigned char* s = (unsigned char*)e + f.offset;
	if(y2<clipping.y1 || y>clipping.y2 || x2<clipping.x1 || x>clipping.x2)
		return;
	if(y < clipping.y1) {
		if((flags & ImageMirrorV) == 0) {
			switch(f.encode) {
			case sprite::ALC: s = skip_alc(s, clipping.y1 - y); break;
			case sprite::RAW: s += (clipping.y1 - y) * f.sx * 3; break;
			case sprite::RAW8: s += (clipping.y1 - y) * f.sx; break;
			case sprite::RLE8: s = skip_v3(s, clipping.y1 - y); break;
			case sprite::RLE: s = skip_rle32(s, clipping.y1 - y); break;
			default: break;
			}
		}
		y = clipping.y1;
	}
	if(y2 > clipping.y2) {
		if(flags & ImageMirrorV) {
			switch(f.encode) {
			case sprite::ALC: s = skip_alc(s, y2 - clipping.y2); break;
			case sprite::RAW: s += (y2 - clipping.y2) * f.sx * 3; break;
			case sprite::RAW8: s += (y2 - clipping.y2) * f.sx; break;
			case sprite::RLE8: s = skip_v3(s, y2 - clipping.y2); break;
			case sprite::RLE: s = skip_rle32(s, y2 - clipping.y2); break;
			default: break;
			}
		}
		y2 = clipping.y2;
	}
	if(y >= y2)
		return;
	int wd = (flags & ImageMirrorV) ? -canvas->scanline : canvas->scanline;
	int sy = (flags & ImageMirrorV) ? y2 - 1 : y;
	switch(f.encode) {
	case sprite::RAW:
		if(x < clipping.x1) {
			if((flags & ImageMirrorH) == 0)
				s += (clipping.x1 - x) * 3;
			x = clipping.x1;
		}
		if(x2 > clipping.x2) {
			if(flags & ImageMirrorH)
				s += (x2 - clipping.x2) * 3;
			x2 = clipping.x2;
		}
		if(x >= x2)
			return;
		if(flags & ImageMirrorH)
			raw32m(ptr(x2 - 1, sy), wd, s, f.sx * 3,
				x2 - x,
				y2 - y);
		else
			raw32(ptr(x, sy), wd, s, f.sx * 3,
				x2 - x,
				y2 - y);
		break;
	case sprite::RAW8:
		if(!f.pallette || (flags & ImagePallette))
			pal = palt;
		else
			pal = (color*)e->ptr(f.pallette);
		if(!pal)
			return;
		if(x < clipping.x1) {
			s += clipping.x1 - x;
			x = clipping.x1;
		}
		if(x2 > clipping.x2)
			x2 = clipping.x2;
		if(x >= x2)
			return;
		if(flags & ImageMirrorH)
			raw832m(ptr(x2 - 1, y), wd, s, f.sx,
				x2 - x,
				y2 - y,
				pal);
		else
			raw832(ptr(x, y), wd, s, f.sx, x2 - x, y2 - y, pal);
		break;
	case sprite::RLE:
		if(flags & ImageMirrorH)
			rle32m(ptr(x2 - 1, sy), wd, s, y2 - y,
				ptr(clipping.x1, sy),
				ptr(clipping.x2, sy),
				alpha);
		else
			rle32(ptr(x, sy), wd, s, y2 - y,
				ptr(clipping.x1, sy),
				ptr(clipping.x2, sy),
				alpha);
		break;
	case sprite::RLE8:
		if(!f.pallette || (flags & ImagePallette))
			pal = palt;
		else
			pal = (color*)e->ptr(f.pallette);
		if(!pal)
			return;
		if(flags & ImageMirrorH)
			rle832m(ptr(x2 - 1, sy), wd, s, y2 - y,
				ptr(clipping.x1, sy),
				ptr(clipping.x2, sy),
				alpha, pal);
		else
			rle832(ptr(x, sy), wd, s, y2 - y,
				ptr(clipping.x1, sy),
				ptr(clipping.x2, sy),
				alpha, pal);
		break;
	case sprite::ALC:
		if(flags & TextBold)
			alc32(ptr(x, sy - 1), wd, s, y2 - y,
				ptr(clipping.x1, sy - 1),
				ptr(clipping.x2, sy - 1),
				fore, (flags & TextItalic) != 0);
		alc32(ptr(x, sy), wd, s, y2 - y,
			ptr(clipping.x1, sy), ptr(clipping.x2, sy),
			fore, (flags & TextItalic) != 0);
		break;
	case sprite::ALC1:
		alc132(ptr(x, sy), wd, s, y2 - y,
			ptr(clipping.x1, sy),
			ptr(clipping.x2, sy),
			alpha);
		break;
	default:
		break;
	}
}

static void rectfall() {
	pushrect push;
	caret.x = caret.y = 0;
	width = getwidth();
	height = getheight();
	rectf();
}

void stroke(int x, int y, const sprite* e, int id, int flags, unsigned char thin, unsigned char* koeff) {
	color tr;
	tr.a = 0;
	tr.r = 255;
	tr.g = 255;
	tr.b = 255;
	auto& fr = e->get(id);
	rect rc = fr.getrect(x, y, flags);
	surface sf(rc.width() + 2, rc.height() + 2, 32);
	x--; y--;
	auto push_clip = clipping;
	auto push_canvas = canvas; canvas = &sf;
	setclip();
	auto push_fore = fore;
	fore = tr;
	rectfall();
	image(1, 1, e, id, ImageNoOffset);
	canvas = push_canvas;
	clipping = push_clip;
	fore = fore_stroke;
	for(int y1 = 0; y1 < sf.height; y1++) {
		bool inside = false;
		for(int x1 = 0; x1 < sf.width; x1++) {
			auto m = (color*)sf.ptr(x1, y1);
			if(!inside) {
				if(*m == tr)
					continue;
				auto px = rc.x1 + x1 - 1;
				auto py = rc.y1 + y1 - 1;
				for(auto n = 0; n < thin; n++, px--) {
					if(koeff)
						pixel(px, py, koeff[n]);
					else
						pixel(px, py);
				}
				inside = true;
			} else {
				if(*m != tr)
					continue;
				auto px = rc.x1 + x1 - 2;
				auto py = rc.y1 + y1 - 1;
				for(auto n = 0; n < thin; n++, px++) {
					if(koeff)
						pixel(px, py, koeff[n]);
					else
						pixel(px, py);
				}
				inside = false;
			}
		}
	}
	for(int x1 = 0; x1 < sf.width; x1++) {
		bool inside = false;
		for(int y1 = 0; y1 < sf.height; y1++) {
			auto m = (color*)sf.ptr(x1, y1);
			if(!inside) {
				if(*m == tr)
					continue;
				auto px = rc.x1 + x1 - 1;
				auto py = rc.y1 + y1 - 1;
				for(auto n = 0; n < thin; n++, py--) {
					if(koeff)
						pixel(px, py, koeff[n]);
					else
						pixel(px, py);
				}
				inside = true;
			} else {
				if(*m != tr)
					continue;
				auto px = rc.x1 + x1 - 1;
				auto py = rc.y1 + y1 - 2;
				for(auto n = 0; n < thin; n++, py++) {
					if(koeff)
						pixel(px, py, koeff[n]);
					else
						pixel(px, py);
				}
				inside = false;
			}
		}
	}
	fore = push_fore;
}

void surface::blend(const surface& source, int alpha) {
	if(bpp != 32 || bpp != source.bpp || height != source.height || width != source.width)
		return;
	cpy32a(ptr(0, 0), scanline,
		const_cast<surface&>(source).ptr(0, 0), source.scanline,
		width, height, alpha);
}

void blit(surface& ds, int x1, int y1, int w, int h, unsigned flags, const surface& ss, int xs, int ys) {
	if(ss.bpp != ds.bpp)
		return;
	int ox;
	if(!correctb(x1, y1, w, h, ox))
		return;
	if(h > ss.height)
		h = ss.height;
	if(w > ss.width)
		w = ss.width;
	if(flags & ImageTransparent)
		cpy32t(
			ds.ptr(x1, y1), ds.scanline,
			const_cast<surface&>(ss).ptr(xs, ys) + ox * 4, ss.scanline,
			w, h);
	else
		cpy(
			ds.ptr(x1, y1), ds.scanline,
			const_cast<surface&>(ss).ptr(xs, ys) + ox * 4, ss.scanline,
			w, h, 4);
}

void blit(surface& dest, int x, int y, int width, int height, unsigned flags, const surface& source, int x_source, int y_source, int width_source, int height_source) {
	if(width == width_source && height == height_source) {
		blit(dest, x, y, width, height, flags, source, x_source, y_source);
		return;
	}
	if(source.bpp != dest.bpp)
		return;
	if(!corrects(dest, x, y, width, height))
		return;
	if(!corrects(source, x_source, y_source, width_source, height_source))
		return;
	int ox;
	if(!correctb(x_source, y_source, width, height, ox))
		return;
	scale32(
		dest.ptr(x, y), dest.scanline, width, height,
		const_cast<surface&>(source).ptr(x_source, y_source) + ox * 4, source.scanline, width_source, height_source);
}

const pma* pma::getheader(const char* id) const {
	auto p = this;
	while(p->name[0]) {
		if(p->name[0] == id[0]
			&& p->name[1] == id[1]
			&& p->name[2] == id[2])
			return p;
		p = (pma*)((char*)p + p->size);
	}
	return 0;
}

const char* pma::getstring(int id) const {
	auto p = getheader("STR");
	if(!p || id > p->count)
		return "";
	return (char*)p + ((unsigned*)((char*)p + sizeof(*p)))[id];
}

int sprite::ganim(int index, int tick) {
	if(!cicles)
		return 0;
	cicle* c = gcicle(index);
	if(!c->count)
		return 0;
	return c->start + tick % c->count;
}

int sprite::glyph(unsigned sym) const {
	// First interval (latin plus number plus ASCII)
	unsigned* pi = (unsigned*)edata();
	unsigned* p2 = pi + esize() / sizeof(unsigned);
	unsigned n = 0;
	while(pi < p2) {
		if(sym >= pi[0] && sym <= pi[1])
			return sym - pi[0] + n;
		n += pi[1] - pi[0] + 1;
		pi += 2;
	}
	return 't' - 0x21; // Unknown symbol is question mark
}

rect sprite::frame::getrect(int x, int y, unsigned flags) const {
	int x2, y2;
	if(!offset)
		return{0, 0, 0, 0};
	if(flags & ImageMirrorH) {
		x2 = x;
		if((flags & ImageNoOffset) == 0)
			x2 += ox;
		x = x2 - sx;
	} else {
		if((flags & ImageNoOffset) == 0)
			x -= ox;
		x2 = x + sx;
	}
	if(flags & ImageMirrorV) {
		y2 = y;
		if((flags & ImageNoOffset) == 0)
			y2 += oy;
		y = y2 - sy;
	} else {
		if((flags & ImageNoOffset) == 0)
			y -= oy;
		y2 = y + sy;
	}
	return{x, y, x2, y2};
}

surface::plugin* surface::plugin::first;

surface::surface() : width(0), height(0), scanline(0), bpp(32), bits(0) {
}

surface::surface(int width, int height, int bpp) : surface() {
	resize(width, height, bpp, true);
}

static surface::plugin* seqlast(surface::plugin* p) {
	while(p->next)
		p = p->next;
	return p;
}

static void seqlink(surface::plugin* p) {
	p->next = 0;
	if(!p->first)
		p->first = p;
	else
		seqlast(p->first)->next = p;
}

surface::plugin::plugin(const char* name, const char* filter) : name(name), filter(filter), next(0) {
	seqlink(this);
}

unsigned char* surface::ptr(int x, int y) {
	return bits + y * scanline + x * (bpp / 8);
}

surface::~surface() {
	resize(0, 0, 0, true);
}

unsigned char* surface::allocator(unsigned char* bits, unsigned size) {
	if(!size) {
		if(bits)
			free(bits);
		bits = 0;
	} else {
		if(!bits)
			bits = (unsigned char*)malloc(size);
		else {
			auto pn = (unsigned char*)realloc(bits, size);
			if(pn)
				bits = pn;
		}
	}
	return bits;
}

void surface::resize(int width, int height, int bpp, bool alloc_memory) {
	if(this->width == width && this->height == height && this->bpp == bpp)
		return;
	this->bpp = bpp;
	this->width = width;
	this->height = height;
	this->scanline = color_scanline(width, bpp);
	if(alloc_memory)
		bits = allocator(bits, height ? (height + 1) * scanline : 0);
}

void surface::flipv() {
	color_flipv(bits, scanline, height);
}

void surface::convert(int new_bpp, color* pallette) {
	if(bpp == new_bpp) {
		bpp = iabs(new_bpp);
		return;
	}
	auto old_scanline = scanline;
	scanline = color_scanline(width, new_bpp);
	if(iabs(new_bpp) <= bpp)
		color_convert(bits, width, height, new_bpp, 0, bits, bpp, pallette, old_scanline);
	else {
		unsigned char* new_bits = (unsigned char*)allocator(0, (height + 1) * scanline);
		color_convert(
			new_bits, width, height, new_bpp, pallette,
			bits, bpp, pallette, old_scanline);
		allocator(bits, 0);
		bits = new_bits;
	}
	bpp = iabs(new_bpp);
}

static unsigned char* rotate32(unsigned char* src, int width, int height) {
	auto m = sizeof(color);
	auto dst = (unsigned char*)malloc(height * width * m);
	for(auto x = 0; x < width; x++) {
		for(auto y = 0; y < height; y++) {
			auto p1 = dst + (x * height + y) * m;
			auto p2 = src + (y * width + x) * m;
			*((color*)p1) = *((color*)p2);
		}
	}
	return dst;
}

void surface::rotate() {
	if(iabs(bpp) != 32)
		return;
	auto pn = rotate32(bits, width, height);
	free(bits);
	bits = pn;
	iswap(width, height);
	scanline = color_scanline(width, bpp);
}

void strokeout(fnevent proc, int dx) {
	pushrect push;
	if(!dx)
		dx = metrics::border;
	caret.x -= dx;
	caret.y -= dx;
	width += dx * 2;
	height += dx * 2;
	proc();
}

void lineup() {
	auto push_caret = caret;
	pushfore push_fore(colors::border);
	line(caret.x + width, caret.y);
	caret = push_caret;
}

void linedown() {
	auto push_caret = caret;
	pushfore push_fore(colors::border);
	caret.y += height; line(caret.x + width, caret.y);
	caret = push_caret;
}

void lineright() {
	auto push_caret = caret;
	pushfore push_fore(colors::border);
	caret.x += width; line(caret.x, caret.y + height);
	caret = push_caret;
}

void caretright() {
	caret.x += width + metrics::padding + metrics::border * 2;
}

void set_need_update() {
	hkey = InputNeedUpdate;
}

void set(int x, int y) {
	caret.x = x - camera.x;
	caret.y = y - camera.y;
}

bool running_scene() {
	return next_proc == 0;
}

void next_scene(fnevent v) {
	next_proc = v;
}

void start_scene() {
	while(next_proc) {
		auto p = next_proc;
		next_proc = 0; p();
	}
}

long scene(fnevent proc) {
	pushscene push(proc);
	while(ismodal()) {
		proc();
		domodal();
	}
	return getresult();
}

void dragdrop(fnevent proc) {
	auto push_drag = drag_active; drag_active = true;
	dragmouse = hmouse;
	while(ismodal()) {
		proc();
		last_scene();
		domodal();
		if(hkey == KeyEscape || hkey == MouseLeft || hkey == MouseRight) {
			breakmodal(0);
			hkey = 0;
			hpressed = false;
		}
	}
	drag_active = push_drag;
}

void execute(fnevent proc, long value, long value2, const void* object) {
	domodal = proc;
	hkey = 0; // Это важно, так как мы никогда не обнуляем эту переменную при исполнении не стандартной команды. Если не делать, будет зацикливание.
	hparam = value;
	hparam2 = value2;
	hobject = object;
}

static void standart_domodal() {
	if(ptips)
		ptips();
	sys_input();
	if(!hkey)
		exit(0);
}

static void beforemodal() {
	caret = {0, 0};
	width = getwidth();
	height = getheight();
	hilite.clear();
	hilite_object = 0;
	tips_text[0] = 0;
	tips_pos.clear();
	hcursor = cursor::Arrow;
	if(hkey == InputNeedUpdate)
		hkey = InputUpdate;
	else
		domodal = standart_domodal;
	if(hmouse.x < 0 || hmouse.y < 0)
		sys_static_area.clear();
	else
		sys_static_area = {0, 0, width, height};
}

bool ismodal() {
	beforemodal();
	if(pbeforemodal)
		pbeforemodal();
	if(!next_proc && !break_modal)
		return true;
	break_modal = false;
	if(pleavemodal)
		pleavemodal();
	return break_modal;
}

void breakmodal(long result) {
	break_modal = true;
	break_result = result;
}

void buttoncancel() {
	breakmodal(0);
}

void buttonok() {
	breakmodal(1);
}

void buttonparam() {
	breakmodal(hparam);
}

long getresult() {
	return break_result;
}

void cbsetint() {
	auto p = (int*)hobject;
	*p = hparam;
}

void cbsetsht() {
	auto p = (short*)hobject;
	*p = (short)hparam;
}

void cbsetuc() {
	auto p = (unsigned char*)hobject;
	*p = (unsigned char)hparam;
}

void cbsetptr() {
	auto p = (void**)hobject;
	*p = (void*)hparam;
}

void button_clear() {
	button_pressed = false;
	button_hilited = false;
	button_executed = false;
}

void button_check(unsigned key, bool execute_by_press) {
	static rect	button_rect;
	button_clear();
	if(input_disabled)
		return;
	rect rc = {caret.x, caret.y, caret.x + width, caret.y + height};
	if(ishilite(rc)) {
		button_hilited = true;
		if(hkey == MouseLeft) {
			if(hpressed)
				button_rect = rc;
			if(hpressed == execute_by_press)
				button_executed = true;
		}
	}
	if(hkey == MouseLeft && !hpressed)
		button_rect.clear();
	if(key && hkey == key)
		button_rect = rc;
	else if(hkey == InputKeyUp) {
		if(button_rect == rc) {
			button_executed = true;
			button_rect.clear();
		}
	}
	if(button_rect == rc)
		button_pressed = true;
}

void fire(fnevent proc, long param, long param2, const void* object) {
	if(button_executed && proc)
		execute(proc, param, param2, object);
}

void fillform() {
	pushfore push(colors::form);
	rectf();
}

void fillwindow() {
	pushfore push(colors::window);
	rectf();
}

void strokeborder() {
	pushfore push(colors::border);
	rectb();
}

void strokeactive() {
	pushfore push(colors::active);
	rectb();
}
