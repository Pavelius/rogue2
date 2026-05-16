#pragma once

extern short unsigned current_area;

struct posable {
	short unsigned index, area_index;
	bool ispresent() const { return area_index == current_area; }
};