#pragma once

enum directionn : unsigned char {
	North, East, South, West,
	NorthEast, SouthEast, SouthWest, NorthWest,
	Center
};

directionn round(directionn d, directionn v);

short unsigned to(short unsigned m, directionn d, short unsigned me = 0xFFFF);
