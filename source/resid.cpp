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

#include "resid.h"

extern unsigned char bin_borders[];
extern unsigned char bin_conditions[];
extern unsigned char bin_cursor[];
extern unsigned char bin_decals[];
extern unsigned char bin_features[];
extern unsigned char bin_floor[];
extern unsigned char bin_font[];
extern unsigned char bin_fow[];
extern unsigned char bin_h1[];
extern unsigned char bin_h2[];
extern unsigned char bin_h3[];
extern unsigned char bin_items[];
extern unsigned char bin_los[];
extern unsigned char bin_missiles[];
extern unsigned char bin_monsters[];
extern unsigned char bin_pc_body[];
extern unsigned char bin_pc_arms[];
extern unsigned char bin_pc_accessories[];
extern unsigned char bin_shadows[];
extern unsigned char bin_splash[];
extern unsigned char bin_status[];
extern unsigned char bin_walls[];

static sprite* resid_data[] = {
	(sprite*)bin_font,
	(sprite*)bin_h1,
	(sprite*)bin_h2,
	(sprite*)bin_h3,
	(sprite*)bin_pc_body,
	(sprite*)bin_pc_arms,
	(sprite*)bin_pc_accessories,
	(sprite*)bin_fow,
	(sprite*)bin_los,
	(sprite*)bin_cursor,
	(sprite*)bin_floor,
	(sprite*)bin_borders,
	(sprite*)bin_decals,
	(sprite*)bin_features,
	(sprite*)bin_items,
	(sprite*)bin_monsters,
	(sprite*)bin_status,
	(sprite*)bin_conditions,
	(sprite*)bin_splash,
	(sprite*)bin_missiles,
	(sprite*)bin_walls,
	(sprite*)bin_shadows,
};
static_assert((sizeof(resid_data) / sizeof(resid_data[0])) == (ResShadows + 1), "Invalid resources count");

sprite* gres(resid v) {
	return resid_data[v];
}