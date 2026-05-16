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