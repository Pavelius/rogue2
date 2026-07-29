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

extern const char* string_params[10];
class pushstring {
	int			param;
	const char* value;
public:
	pushstring(int param, const char* value) : param(param), value(string_params[param]) { string_params[param] = value; }
	template<class T> pushstring(int param, T v) : pushstring(param, getname(v)) {}
	~pushstring() { string_params[param] = value; }
};

bool stringvar_identifier(stringbuilder& sb, const char* identifier);
bool stringparam_identifier(stringbuilder& sb, const char* identifier);