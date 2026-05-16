#include "draw.h"
#include "io_stream.h"
#include "math.h"
#include "stringbuilder.h"
#include "adat.h"

struct resei {
	const char*	name;
	const char*	folder;
	unsigned	flags;
	sprite*		data;
	bool		notfound;
};
static adat<resei, 512> source;

bool is_dynamic_image(const void* data) {
	return source.have(data);
}

static resei* find(const char* name, const char* folder) {
	for(auto& e : source) {
		if(!equal(e.name, name))
			continue;
		if(folder && !equal(e.folder, folder))
			continue;
		return &e;
	}
	return 0;
}

static unsigned char* rotate(unsigned char* src, int width, int height) {
	auto dst = new unsigned char[width * height * 3];
	for(auto x = 0; x < width; x++) {
		for(auto y = 0; y < height; y++) {
			auto p1 = dst + (x * height + y) * 3;
			auto p2 = src + (y * width + x) * 3;
			memcpy(p1, p2, 3);
		}
	}
	return dst;
}

const sprite* gres(const char* name, const char* folder, point maxsize, int ox, int oy) {
	if(!name)
		return 0;
	if(!folder)
		folder = "images";
	auto p = find(name, folder);
	if(!p) {
		p = source.add();
		p->name = szdup(name);
		p->folder = folder;
		p->data = 0;
		p->notfound = false;
	}
	if(!p || p->notfound)
		return 0;
	if(!p->data) {
		char temp[260];
		bool need_rotate = false;
		while(*name) {
			if(*name == '-') {
				name++;
				need_rotate = true;
				continue;
			}
			break;
		}
		p->data = (sprite*)loadb(szurl(temp, 0, p->folder, name, "pma"));
		if(!p->data) {
			surface dc;
			for(auto pg = surface::plugin::first; pg; pg = pg->next) {
				szurl(temp, 0, p->folder, name, pg->name);
				if(dc.read(temp, 0, 32)) {
					rect rc = {0, 0, dc.width, dc.height};
					if(maxsize.x && rc.width() > maxsize.x) {
						rc.x1 = (rc.width() - maxsize.x) / 2;
						rc.x2 = rc.x1 + maxsize.x;
					}
					if(maxsize.y && rc.height() > maxsize.y) {
						rc.y1 = (rc.width() - maxsize.y) / 2;
						rc.y2 = rc.y1 + maxsize.y;
					}
					unsigned size = sizeof(sprite) + rc.height() * rc.width() * 3;
					// Вывод на экран
					p->data = (sprite*)new char[size]; memset(p->data, 0, size);
					p->data->width = rc.width();
					p->data->height = rc.height();
					p->data->count = 1;
					auto& ef = p->data->frames[0];
					ef.encode = sprite::RAW;;
					ef.sx = rc.width();
					ef.sy = rc.height();
					ef.offset = sizeof(sprite);
					// Cheap and simple method encoding without alpha channel
					auto pb = (unsigned char*)p->data->ptr(p->data->frames[0].offset);
					auto pd = pb;
					for(auto y = rc.y1; y < rc.y2; y++) {
						for(auto x = rc.x1; x < rc.x2; x++) {
							auto input = dc.ptr(x, y);
							pd[0] = input[0];
							pd[1] = input[1];
							pd[2] = input[2];
							pd += 3;
						}
					}
					if(need_rotate) {
						auto pn = rotate(pb, rc.width(), rc.height());
						memcpy(pb, pn, rc.width() * rc.height() * 3);
						iswap(p->data->width, p->data->height);
						iswap(ef.sx, ef.sy);
						delete pn;
					}
					if(ox == -10000)
						ox = ef.sx / 2;
					if(oy == -10000)
						oy = ef.sy / 2;
					ef.ox = ox;
					ef.oy = oy;
					break;
				}
			}
		}
	}
	if(!p->data)
		p->notfound = true;
	return p->data;
}