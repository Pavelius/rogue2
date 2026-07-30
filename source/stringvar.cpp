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
#include "stringvar.h"

const char* string_params[4];

bool stringvar_identifier(stringbuilder& sb, const char* identifier) {
	auto pn = bsdata<stringvari>::find(identifier);
	if(pn) {
		pn->proc(sb);
		return true;
	}
	return false;
}

bool stringparam_identifier(stringbuilder& sb, const char* identifier) {
	if(identifier[0] == 'P'
		&& identifier[1] >= '1' && identifier[1] <= '4'
		&& identifier[2] == 0) {
		sb.add(string_params[identifier[1] - '1']);
		return true;
	}
	return false;
}