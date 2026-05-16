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

template<class T> inline void iswap(T& a, T& b) { T i = a; a = b; b = i; }

inline int iabs(int a) { return a > 0 ? a : -a; }
inline int imax(int a, int b) { return a > b ? a : b; }
inline int imin(int a, int b) { return a < b ? a : b; }

int isqrt(int num);
float sqrt(const float x);

#define maptbl(t, id) (t[imax((unsigned long)0, imin((unsigned long)id, (unsigned long)(sizeof(t)/sizeof(t[0])-1)))])