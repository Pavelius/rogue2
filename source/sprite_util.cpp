#include "io_stream.h"
#include "sprite_util.h"
#include "stringbuilder.h"
#include "slice.h"

sprite::frame& sprite_add(sprite* p) {
	for(int i = 0; i < p->count; i++) {
		if(!p->frames[i].offset)
			return p->frames[i];
	}
	return p->frames[0];
}

void* sprite_add(sprite* p, const void* data, int dsize) {
	void* pointer = ((unsigned char*)p) + p->size;
	if(data)
		memcpy(pointer, data, dsize);
	p->size += dsize;
	return pointer;
}

void sprite_create(sprite* p, int count, int cicles, int additional_bytes) {
	// Инициализируем значения под данные спрайтов.
	// Никогда не добавляем сюда дополнительные байты.
	// Подтому что дизайн API одразумевает такие вызовы:
	// setup(sprite_count);
	// add(data1, sizeof(data1));
	// add(data2, sizeof(data2));
	// ...
	// add(dataN, sizeof(dataN));
	// Таким образом size будет правильным
	memset(p, 0, sizeof(sprite) + sizeof(sprite::frame)*(count - 1));
	p->name[0] = 'P'; p->name[1] = 'M'; p->name[2] = 'A';
	p->count = count;
	p->size = sizeof(sprite) + sizeof(sprite::frame)*(count - 1);
	if(cicles) {
		p->cicles = cicles;
		p->cicles_offset = p->size;
		p->size += p->cicles*sizeof(sprite::cicle) + additional_bytes;
	}
}

void pma_write(const char* url, pma** pp) {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	for(auto p = pp; *p; p++)
		file.write(*p, (*p)->size);
	pma trail = {0};
	file.write(&trail, sizeof(trail));
}

void sprite_write(const char* url, const sprite* p) {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	file.write(p, p->size);
	pma trail = {0};
	file.write(&trail, sizeof(trail));
}

// Return count of repeated 'alpha' values in alpha channel
static int skpe(const color* s, const color* s2, int max, unsigned char alpha) {
	if(s2 > s + max)
		s2 = s + max;
	for(const color* s1 = s; s1 < s2; s1++) {
		if(s1->a != alpha)
			return s1 - s;
	}
	return s2 - s;
}

// Return count of repeated 'index' values
static int skpe8(const unsigned char* s, const unsigned char* s2, int max, unsigned char index) {
	if(s2 > s + max)
		s2 = s + max;
	for(const unsigned char* s1 = s; s1 < s2; s1++) {
		if(*s1 != index)
			return s1 - s;
	}
	return s2 - s;
}

// Return first occuence of 'index' values
static int skpne8(const unsigned char* s, const unsigned char* s2, int max, unsigned char index, unsigned char index2) {
	if(s2 > s + max)
		s2 = s + max;
	for(const unsigned char* s1 = s; s1 < s2; s1++) {
		if(*s1 == index || *s1 == index2)
			return s1 - s;
	}
	return s2 - s;
}

static int skp3e(const unsigned char* s, const unsigned char* s2, int max) {
	const int cbs = 3;
	if(s2 > s + max * cbs)
		s2 = s + max * cbs;
	for(const unsigned char* s1 = s; s1 < s2; s1 += cbs) {
		if(s1[0] || s1[1] || s1[2])
			return (s1 - s) / cbs;
	}
	return (s2 - s) / cbs;
}

static int skp3ne(const unsigned char* s, const unsigned char* s2, int max) {
	const int cbs = 3;
	if(s2 > s + max * cbs)
		s2 = s + max * cbs;
	for(const unsigned char* s1 = s; s1 < s2; s1 += cbs) {
		if(s1[0] == 0 && s1[1] == 0 && s1[2] == 0)
			return (s1 - s) / cbs;
	}
	return (s2 - s) / cbs;
}

static const color* skv(int sign, const color* s, int sn, int& w, int& h, int& oy, int& ox) {
	do {
		const color* p2 = &s[h*sn];
		for(const color* p1 = s; p1 < p2; p1 += sn) {
			if(p1->a)
				return s;
		}
		if(sign > 0) {
			s++;
			ox--;
		} else
			s--;
		if(--w == 0)
			return 0;
	} while(true);
}

static const unsigned char* skv8(int sign, const unsigned char* s, int sn, int& w, int& h, int& oy, int& ox, unsigned char transparent_index) {
	do {
		const unsigned char* p2 = s + h * sn;
		for(const unsigned char* p1 = s; p1 < p2; p1 += sn) {
			if(*p1 != transparent_index)
				return s;
		}
		if(sign > 0) {
			s++;
			ox--;
		} else
			s--;
		if(--w == 0)
			return 0;
	} while(true);
}

static const color* skh(int sign, const color* s, int sn, int& w, int& h, int& oy, int& ox) {
	do {
		const color* p2 = &s[w];
		for(const color* p1 = s; p1 < p2; p1++) {
			if(p1->a)
				return s;
		}
		if(sign > 0) {
			s += sn;
			oy--;
		} else
			s -= sn;
		if(--h == 0)
			return 0;
	} while(true);
}

static const unsigned char* skh8(int sign, const unsigned char* s, int sn, int& w, int& h, int& oy, int& ox, unsigned char transparent_index) {
	do {
		const unsigned char* p2 = s + w;
		for(const unsigned char* p1 = s; p1 < p2; p1++) {
			if(*p1 != transparent_index)
				return s;
		}
		if(sign > 0) {
			s += sn;
			oy--;
		} else
			s -= sn;
		if(--h == 0)
			return 0;
	} while(true);
}

//static const unsigned char* skhne8(int sign, const unsigned char* s, int sn, int& w, int& h, int& oy, int& ox, unsigned char image_index) {
//	do {
//		const unsigned char* p2 = s + w;
//		for(const unsigned char* p1 = s; p1 < p2; p1++) {
//			if(*p1 == image_index)
//				return s;
//		}
//		if(sign > 0) {
//			s += sn;
//			oy--;
//		} else
//			s -= sn;
//		if(--h == 0)
//			return 0;
//	} while(true);
//}

// Return true if there is no alpha channel (plain texture)
static bool tstv(color* s, int sn1, int w, int h) {
	color* s1 = &s[h*sn1];
	while(s < s1) {
		if(skpe(s, &s[w], w, 0xFF) != w)
			return false;
		s += sn1;
	}
	return true;
}

// (00) end of line
// (01 - 7F) draw count of pixels
// (80, XX) skip count of XX pixels
// 81 - FF skip count of (b-0x80) pixels
// each pixel has a value
static int store_alc(unsigned char* d, const unsigned char* s, int sn, int& w, int& h, int& ox, int& oy) {
	const int cbs = 3;
	unsigned char* ds = d;
	unsigned char sk;
	// clip upper line
	if(!s || !h)
		return 0;
	// save body
	int h1 = h;
	do {
		const unsigned char* p1 = s;
		const unsigned char* p2 = &s[w];
		while(p1 < p2) {
			// 1) skip
			sk = skp3e(p1, p2, 255);
			if(sk) {
				if(p1 + sk * cbs < p2) {
					if(sk >= 1 && sk <= (0xFF - 0x80))
						*d++ = 0x80 + sk;
					else {
						*d++ = 0x80;
						*d++ = sk;
					}
				}
				p1 += sk * cbs;
				continue;
			}
			// 2) pixels
			sk = skp3ne(p1, p2, 0x7F);
			*d++ = sk;
			do {
				d[0] = p1[2];
				d[1] = p1[1];
				d[2] = p1[0];
				d += cbs;
				p1 += cbs;
			} while(--sk);
		}
		*d++ = 0;
		s += sn;
	} while(--h1);
	return d - ds;
}

// When p is pointer to readed iostream, then when p[0] is:
// (00) end of line
// (01 - 7F) draw p[0] of pixels leaded after
// (80, XX) draw count of XX bytes of alpha AA pixels
// (81 - 9F) draw count of (B-0x80) bytes of alpha AA pixels
// (A0, XX) skip count of XX pixels
// A1 - FF skip count of (b-0xA0) pixels
// each pixel has b,g,r value
static int store_rle8(unsigned char* d, const unsigned char* s, int sn, int& w, int& h, int& ox, int& oy, unsigned char transparent_index, unsigned char shadow_index) {
	unsigned char* ds = d;
	unsigned char sk;
	// clip upper line
	if(!s || !h)
		return 0;
	s = skh8(1, s, sn, w, h, oy, ox, transparent_index);
	if(!s || !h)
		return 0;
	skh8(-1, &s[(h - 1)*sn], sn, w, h, oy, ox, transparent_index);
	if(!s || !h)
		return 0;
	s = skv8(1, s, sn, w, h, oy, ox, transparent_index);
	if(!s || !w)
		return 0;
	skv8(-1, &s[w - 1], sn, w, h, oy, ox, transparent_index);
	if(!s || !w)
		return 0;
	// save body
	int h1 = h;
	do {
		const unsigned char* p1 = s;
		const unsigned char* p2 = s + w;
		while(p1 < p2) {
			// 1) image bits
			sk = skpne8(p1, p2, 0x7F, transparent_index, shadow_index);
			if(sk) {
				*d++ = sk;
				do {
					*d++ = *p1++;
				} while(--sk);
				continue;
			}
			// 2) skip
			sk = skpe8(p1, p2, 255, transparent_index);
			if(sk) {
				if(p1 + sk < p2) {
					if(sk >= 1 && sk <= (0xFF - 0xA0))
						*d++ = 0xA0 + sk;
					else {
						*d++ = 0xA0;
						*d++ = sk;
					}
				}
				p1 += sk;
				continue;
			}
			// 3) alpha
			sk = skpe8(p1, p2, 255, shadow_index);
			if(sk >= 1 && sk <= (0x9F - 0x80))
				*d++ = 0x80 + sk;
			else {
				*d++ = 0x80;
				*d++ = sk;
			}
			p1 += sk;
		}
		*d++ = 0;
		s += sn;
	} while(--h1);
	return d - ds;
}

// When p is pointer to readed iostream, then when p[0] is:
// (00) end of line
// (01 - 9F) draw p[0] of pixels leaded after
// (A0, XX) skip count of XX pixels
// A1 - FF skip count of (b-0xA0) pixels
// each pixel has b,g,r value
static int store_alc8(unsigned char* d, const unsigned char* s, int sn, int& w, int& h, int& ox, int& oy, unsigned char transparent_index) {
	unsigned char* ds = d;
	unsigned char sk;
	// clip upper line
	if(!s || !h)
		return 0;
	s = skh8(1, s, sn, w, h, oy, ox, transparent_index);
	if(!s || !h)
		return 0;
	skh8(-1, &s[(h - 1) * sn], sn, w, h, oy, ox, transparent_index);
	if(!s || !h)
		return 0;
	s = skv8(1, s, sn, w, h, oy, ox, transparent_index);
	if(!s || !w)
		return 0;
	skv8(-1, &s[w - 1], sn, w, h, oy, ox, transparent_index);
	if(!s || !w)
		return 0;
	// save body
	int h1 = h;
	do {
		const unsigned char* p1 = s;
		const unsigned char* p2 = s + w;
		while(p1 < p2) {
			// 1) image bits
			sk = skpne8(p1, p2, 0x9F, transparent_index, transparent_index);
			if(sk) {
				*d++ = sk;
				do {
					*d++ = *p1++;
				} while(--sk);
				continue;
			}
			// 2) skip
			sk = skpe8(p1, p2, 255, transparent_index);
			if(sk) {
				if(p1 + sk < p2) {
					if(sk >= 1 && sk <= (0xFF - 0xA0))
						*d++ = 0xA0 + sk;
					else {
						*d++ = 0xA0;
						*d++ = sk;
					}
				}
				p1 += sk;
				continue;
			}
			p1 += sk;
		}
		*d++ = 0;
		s += sn;
	} while(--h1);
	return d - ds;
}

static int store_alc1(unsigned char* d, const unsigned char* s, int sn, int& w, int& h, int& ox, int& oy, unsigned char transparent_index) {
	unsigned char* ds = d;
	unsigned char sk;
	// clip upper line
	if(!s || !h)
		return 0;
	s = skh8(1, s, sn, w, h, oy, ox, transparent_index);
	if(!s || !h)
		return 0;
	skh8(-1, &s[(h - 1) * sn], sn, w, h, oy, ox, transparent_index);
	if(!s || !h)
		return 0;
	s = skv8(1, s, sn, w, h, oy, ox, transparent_index);
	if(!s || !w)
		return 0;
	skv8(-1, &s[w - 1], sn, w, h, oy, ox, transparent_index);
	if(!s || !w)
		return 0;
	// save body
	int h1 = h;
	do {
		const unsigned char* p1 = s;
		const unsigned char* p2 = s + w;
		while(p1 < p2) {
			// 1) skip
			sk = skpe8(p1, p2, 0xFF, transparent_index);
			*d++ = sk;
			p1 += sk;
			// 2) image bits
			sk = skpne8(p1, p2, 0xFF, transparent_index, transparent_index);
			if(!sk) {
				d--; // Remove last skip value (would be 0xFF)
				break; // This is end line - too big skip image not support
			}
			*d++ = sk;
			p1 += sk;
		}
		*d++ = 0xFF;
		s += sn;
	} while(--h1);
	return d - ds;
}

static int store_raw8(unsigned char* d, const unsigned char* s, int sn, int w, int h) {
	const unsigned char* se = s + h * sn;
	while(s < se) {
		memcpy(d, s, w);
		d += w;
		s += sn;
	}
	return w * h;
}

static int store_raw(unsigned char* d, const unsigned char* s, int sn, int w, int h) {
	const int cbs = 3;
	unsigned char* ds = d;
	// clip upper line
	if(!s || !h)
		return 0;
	// save body
	int h1 = h;
	while(h1) {
		const color* p1 = (color*)s;
		const color* p2 = (color*)s + w;
		while(p1 < p2) {
			d[0] = p1->b;
			d[1] = p1->g;
			d[2] = p1->r;
			d += cbs;
			p1++;
		}
		s += sn;
		h1--;
	}
	return d - ds;
}

// (00) end of line
// (01 - 7F) draw count of pixels
// (80, XX, AA) draw count of XX bytes of alpha AA pixels
// (81 - 9F, AA) draw count of (B-0xC0) bytes of alpha AA pixels
// (A0, XX) skip count of XX pixels
// A1 - FF skip count of (b-0xB0) pixels
// each pixel has b,g,r value
static int store_rle(unsigned char* d, const unsigned char* input, int sn, int& w, int& h, int& ox, int& oy) {
	unsigned char* ds = d;
	unsigned char sk;
	const color* s = (color*)input;
	sn = sn / sizeof(color);
	// clip upper line
	if(!s || !h)
		return 0;
	s = skh(1, s, sn, w, h, oy, ox);
	if(!s || !h)
		return 0;
	skh(-1, &s[(h - 1)*sn], sn, w, h, oy, ox);
	if(!s || !h)
		return 0;
	s = skv(1, s, sn, w, h, oy, ox);
	if(!s || !w)
		return 0;
	skv(-1, &s[w - 1], sn, w, h, oy, ox);
	if(!s || !w)
		return 0;
	// save body
	int h1 = h;
	do {
		const color* p1 = s;
		const color* p2 = &s[w];
		while(p1 < p2) {
			// 1) image bits
			sk = skpe(p1, p2, 0x7F, 255);
			if(sk) {
				*d++ = sk;
				do {
					*d++ = p1->b;
					*d++ = p1->g;
					*d++ = p1->r;
					p1++;
				} while(--sk);
				continue;
			}
			// 2) skip
			sk = skpe(p1, p2, 255, 0);
			if(sk) {
				if(p1 + sk < p2) {
					if(sk >= 1 && sk <= (0xFF - 0xA0))
						*d++ = 0xA0 + sk;
					else {
						*d++ = 0xA0;
						*d++ = sk;
					}
				}
				p1 += sk;
				continue;
			}
			// 3) alpha
			sk = skpe(p1, p2, 255, p1->a);
			if(sk >= 1 && sk <= (0x9F - 0x80))
				*d++ = sk + 0x80;
			else {
				*d++ = 0x80;
				*d++ = sk;
			}
			*d++ = p1->a;
			do {
				*d++ = p1->b;
				*d++ = p1->g;
				*d++ = p1->r;
				p1++;
			} while(--sk);
		}
		*d++ = 0;
		s += sn;
	} while(--h1);
	return d - ds;
}

int sprite_store(sprite* ps, const unsigned char* p, int width, int w, int h, int ox, int oy, sprite::encodes mode, unsigned char shadow_index, color* original_pallette, int explicit_frame, unsigned char transparent_index) {
	const int cbs = 3;
	sprite::frame& f = ((explicit_frame == -1) ? sprite_add(ps) : ps->frames[explicit_frame]);
	f.offset = ps->size;
	if(mode == sprite::Auto) {
		if(tstv((color*)p, w, w, h))
			mode = sprite::RAW;
		else
			mode = sprite::RLE;
	}
	auto custom_data = (unsigned char*)ps->ptr(ps->size);
	int output_size = 0;
	switch(mode) {
	case sprite::RAW:
		output_size = store_raw(custom_data, p, width, w, h);
		f.sx = w;
		f.sy = h;
		break;
	case sprite::RLE:
		output_size = store_rle(custom_data, p, width, w, h, ox, oy);
		f.sx = w;
		f.sy = h;
		break;
	case sprite::ALC:
		output_size = store_alc(custom_data, p, width, w, h, ox, oy);
		f.sx = w / cbs;
		f.sy = h;
		break;
	case sprite::ALC8:
		output_size = store_alc8(custom_data, p, width, w, h, ox, oy, transparent_index);
		f.sx = w;
		f.sy = h;
		break;
	case sprite::ALC1:
		output_size = store_alc1(custom_data, p, width, w, h, ox, oy, transparent_index);
		f.sx = w;
		f.sy = h;
		break;
	case sprite::RLE8:
		output_size = store_rle8(custom_data, p, width, w, h, ox, oy, transparent_index, shadow_index);
		f.sx = w;
		f.sy = h;
		break;
	case sprite::RAW8:
		output_size = store_raw8(custom_data, p, width, w, h);
		f.sx = w;
		f.sy = h;
		break;
	default:
		break;
	}
	f.encode = mode;
	f.ox = ox;
	f.oy = oy;
	if(!output_size) {
		f.offset = 0;
		return 0;
	}
	// test unique
	for(int i = 0; i < ps->count; i++) {
		if(&f == &ps->frames[i])
			continue;
		if(!ps->frames[i].offset)
			continue;
		if(memcmp(custom_data, ps->ptr(ps->frames[i].offset), output_size) == 0) {
			f.offset = ps->frames[i].offset;
			f.pallette = ps->frames[i].pallette;
			return output_size;
		}
	}
	ps->size += output_size;
	f.pallette = 0;
	// test unique pallette
	if(original_pallette) {
		for(int i = 0; i < ps->count; i++) {
			if(!ps->frames[i].offset)
				continue;
			if(!ps->frames[i].pallette)
				continue;
			color* p = (color*)ps->ptr(ps->frames[i].pallette);
			if(memcmp(original_pallette, p, 256 * sizeof(color)) == 0) {
				// Find pallette
				f.pallette = ps->frames[i].pallette;
				break;
			}
		}
		// Add new pallette
		if(!f.pallette) {
			f.pallette = ps->size;
			sprite_add(ps, original_pallette, 256 * sizeof(color));
		}
	}
	return output_size;
}

rect sprite_get_border_rect(const sprite* ps) {
	rect result = {};
	auto count = ps->count;
	for(auto i = 0; i < count; i++) {
		auto rc = ps->get(i).getrect(0, 0, 0);
		if(result.x1 > rc.x1)
			result.x1 = rc.x1;
		if(result.x2 < rc.x2)
			result.x2 = rc.x2;
		if(result.y1 > rc.y1)
			result.y1 = rc.y1;
		if(result.y2 < rc.y2)
			result.y2 = rc.y2;
	}
	return result;
}

void util::normalize(point& pt, const surface& bm) {
	if(pt.x == -1000)
		pt.x = bm.width / 2;
	if(pt.y == -1000)
		pt.y = bm.height / 2;
	else if(pt.y < 0)
		pt.y = bm.height + pt.y;
}

const char* util::getn(char* result, const char* base, const char* type, const char* name, int index, const char* ext) {
	stringbuilder sb(result, result + 259);
	if(base)
		sb.add("%1/", base);
	if(type)
		sb.add("%1/", type);
	if(index == -1)
		sb.add("%3.%4", base, type, name, ext);
	else
		sb.add("%3%5i.%4", base, type, name, ext, index);
	return result;
}

void util::close(const sprite* p, const char* name, const char* folder) {
	if(!folder)
		folder = "art/objects";
	char temp[260]; getn(temp, 0, folder, name);
	sprite_write(temp, p);
	delete p;
}

sprite* util::create(unsigned frames, unsigned size) {
	auto p = (sprite*)new char[size];
	sprite_create(p, frames, 0, 0);
	return p;
}

void util::add_image(sprite* p, point pt, surface& bm) {
	sprite_store(p, bm.ptr(0, 0), bm.scanline, bm.width, bm.height,
		pt.x, pt.y, sprite::Auto, 0, 0, -1, 0);
}

void util::add(sprite* p, const char* folder, const char* name, int index, point pt, util::oppr proc) {
	char temp[260]; getn(temp, image_source, folder, name, index, "png");
	surface bm(temp);
	if(!bm)
		return;
	normalize(pt, bm);
	proc(p, pt, bm);
}