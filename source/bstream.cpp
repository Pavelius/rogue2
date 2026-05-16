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

#include "bstream.h"

bstream::bstream(unsigned char* data, unsigned maximum_bytes) : data(data), pos(0), maximum(maximum_bytes * 8) {
}

void bstream::put(int value, int size) {
	for(int j = size - 1; j >= 0; j--, pos++) {
		if(value & (1 << j))
			data[pos >> 3] |= 1 << (7 - (pos & (8 - 1)));
		else
			data[pos >> 3] &= ~(1 << (7 - (pos & (8 - 1))));
	}
}

unsigned bstream::get(int size) {
	int r = 0;
	for(int j = size - 1; j >= 0; j--, pos++) {
		r <<= 1;
		if(data[pos / 8] & (1 << (7 - (pos & (8 - 1)))))
			r |= 1;
	}
	return r;
}

unsigned bstream::get() {
	unsigned r = ((data[pos >> 3] >> (pos & 0x7)) & (unsigned char)1);
	pos++;
	return r;
}

unsigned bstream::bit(unsigned pos) {
	return ((data[pos >> 3] >> (pos & 0x7)) & (unsigned char)1);
}