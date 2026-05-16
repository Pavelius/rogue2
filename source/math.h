#pragma once

template<class T> inline void iswap(T& a, T& b) { T i = a; a = b; b = i; }

inline int iabs(int a) { return a > 0 ? a : -a; }
inline int imax(int a, int b) { return a > b ? a : b; }
inline int imin(int a, int b) { return a < b ? a : b; }

int isqrt(int num);
float sqrt(const float x);

#define maptbl(t, id) (t[imax((unsigned long)0, imin((unsigned long)id, (unsigned long)(sizeof(t)/sizeof(t[0])-1)))])