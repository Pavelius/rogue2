#pragma once

#include "draw.h"

namespace util {

typedef void(*oppr)(sprite* p, point pt, surface& bm);
extern const char* image_source;
void add(sprite* p, const char* folder, const char* name, int index, point pt, oppr proc);
void add_image(sprite* p, point pt, surface& bm);
void close(const sprite* p, const char* name, const char* folder = 0);
void normalize(point& pt, const surface& bm);

sprite*	create(unsigned frames, unsigned size = 1024 * 1024);
const char* getn(char* result, const char* base, const char* type, const char* name, int index = -1, const char* ext = "pma");

}

void* sprite_add(sprite* p, const void* data, int dsize);
int	sprite_store(sprite* ps, const unsigned char* p, int width, int w, int h, int ox, int oy, sprite::encodes mode = sprite::Auto, unsigned char shadow_index = 0, color* original_pallette = 0, int explicit_frame = -1, unsigned char transparent_index = 0);

void font_write(const char* url, const char* name, int size, sprite::encodes encode = sprite::ALC);
void font_write_ascii(const char* url, const char* name, int size, sprite::encodes encode);
void pma_write(const char* url, pma** pp);
void sprite_create(sprite* p, int count, int cicles = 0, int additional_bytes = 0);
void sprite_write(const char* url, const sprite* p);