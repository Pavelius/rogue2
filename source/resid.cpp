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

#include "bsdata.h"
#include "io_stream.h"
#include "resid.h"
#include "stringbuilder.h"

struct residi {
	const char*	id;
	sprite*		data;
	bool		error;
};

BSDATA(residi) = {
	{"font"},
	{"h1"},
	{"h2"},
	{"h3"},
	{"pc_body"},
	{"pc_arms"},
	{"pc_accessories"},
	{"fow"},
	{"los"},
	{"cursor"},
	{"floor"},
	{"borders"},
	{"decals"},
	{"features"},
	{"items"},
	{"monsters"},
	{"status"},
	{"conditions"},
	{"splash"},
	{"walls"},
	{"shadows"},
};
static_assert(lenghtof(bsdata<residi>::elements)==(ResShadows+1), "Invalid resources count");

sprite* gres(resid v) {
	auto& ei = bsdata<residi>::elements[v];
	if(ei.data)
		return ei.data;
	if(ei.error)
		return 0;
	ei.data = (sprite*)loadb(str("art/%1.pma", ei.id));
	if(!ei.data)
		ei.error = true;
	return ei.data;
}