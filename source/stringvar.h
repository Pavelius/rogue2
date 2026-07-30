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

#include "stringbuilder.h"

#pragma once

struct stringvari {
	const char*	id;
	fnprint		proc;
};

extern const char* string_params[4];

class pushstring {
	const char* p[4];
	void set(const char** v1, const char** v2) {
		for(unsigned i = 0; i < sizeof(string_params) / sizeof(string_params[0]); i++)
			v1[i] = v2[i];
	}
public:
	pushstring(const char* v1, const char* v2 = 0, const char* v3 = 0, const char* v4 = 0) {
		set(p, string_params);
		string_params[0] = v1;
		string_params[1] = v2;
		string_params[2] = v3;
		string_params[3] = v4;
	}
	~pushstring() { set(string_params, p); }
};

bool stringvar_identifier(stringbuilder& sb, const char* identifier);
bool stringparam_identifier(stringbuilder& sb, const char* identifier);