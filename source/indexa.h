/*
	Copyright 2026 by Pavel Chistyakov

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.77
*/

#pragma once

#include "adat.h"

enum areafn : unsigned char;

typedef bool(*fnareais)(short unsigned i);

struct indexa : adat<short unsigned, 512> {
	void match(areafn v, bool keep = true);
	void match(fnareais allow, bool keep = true);
	void select(short unsigned i, int radius);
	void select(short unsigned i, int radius, fnareais allow, bool keep);
	void select(fnareais allow, bool keep);
	void shuffle();
	void sort(short unsigned i);
};
extern indexa indecies;
