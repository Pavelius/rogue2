#pragma once

#include "stringbuilder.h"

#define assert_enum(T,N) static_assert(sizeof(T)/sizeof(T[0])==(N+1), "Invalid count " #T);

#define BSENUM(T) template<> const char* bsenum<T>::names[]
#define BSINFO(T) template<> const char* bsenum<T>::info[]
#define BFENUM(T) template<> const int bsenum<T>::count = sizeof(bsenum<T>::names)/sizeof(bsenum<T>::names[0]);
#define BAENUM(N) assert_enum(bsenum<decltype(N)>::names, N) BFENUM(decltype(N))
