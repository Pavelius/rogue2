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

#include "stringbuilder.h"

#define assert_enum(T,N) static_assert(sizeof(T)/sizeof(T[0])==(N+1), "Invalid count " #T);

#define BSENUM(T) template<> const char* bsenum<T>::names[]
#define BFENUM(T) template<> const int bsenum<T>::count = sizeof(bsenum<T>::names)/sizeof(bsenum<T>::names[0]);
#define BAENUM(N) assert_enum(bsenum<decltype(N)>::names, N) BFENUM(decltype(N))
