#include "rect.h"

#pragma once

struct point {
	union {
		struct {
			short int x, y;
		};
		unsigned u;
	};
	point() = default;
	constexpr point(int x, int y) : x((short)x), y((short)y) {}
	constexpr point(unsigned u) : u(u) {}
	constexpr operator long() const { return u; }
	constexpr explicit operator bool() const { return x >= 0; }
	bool operator!=(const point pt) const { return pt.x != x || pt.y != y; }
	bool operator==(const point pt) const { return pt.x == x && pt.y == y; }
	point operator-(const point pt) const { return{(short)(x - pt.x), (short)(y - pt.y)}; }
	point operator+(const point pt) const { return{(short)(x + pt.x), (short)(y + pt.y)}; }
	point operator/(int v) const { return point(x / v, y / v); }
	point operator*(int v) const { return point(x * v, y * v); }
	void clear() { x = -1000; y = -1000; }
	bool in(const rect& rc) const { return x >= rc.x1 && x <= rc.x2 && y >= rc.y1 && y <= rc.y2; }
	bool in(const point p1, const point p2, const point p3) const;
};

long distance(point from, point to);