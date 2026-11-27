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
	limitations under the License.
*/

#pragma once

#define maprnd(t) t[xrand()%(sizeof(t)/sizeof(t[0]))]

extern unsigned int xrand_seed, xrand_origin_seed;

int xrand();

inline int xrand(int n1) { return n1 ? xrand() % n1 : 0; }
inline int xrand(int n1, int n2) { return n1 + xrand() % (n2 - n1 + 1); }

inline int d6() { return 1 + xrand() % 6; }
inline int d20() { return 1 + xrand() % 20; }
inline int d100() { return xrand() % 100; }

inline bool chance(int value) { return d100() < value; }

template<class T>
void zshuffle(T* data, int count) {
	for(int i = count - 1; i > 0; --i) {
		int j = xrand(i + 1);
		auto temporary = data[i];
		data[i] = data[j];
		data[j] = temporary;
	}
}