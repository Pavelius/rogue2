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

struct treasurei {
	struct infoi {
		char chance;
		short from, to;
	};
	char symbol;
	infoi cooper, silver, gold, platinum, gems, art, magic;
};

static treasurei treasure_table[] = {
	{'A', {25, 1000, 3000}, {30, 200, 2000}, {40, 1000, 6000}, {35, 300, 1800}, {60, 10, 40}, {50, 2, 12}, {30, 3}},
	{'B', {50}, {25}, {}, {}, {}, {}, {}},
	{'C', {20}, {30}, {}, {}, {}, {}, {}},
	{'D', {10}, {15}, {}, {}, {}, {}, {}},
	{'E', {5}, {25}, {}, {}, {}, {}, {}},
	{'F', {}, {10}, {}, {}, {}, {}, {}},
	{'G', {}, {}, {}, {}, {}, {}, {}},
	{'H', {25}, {40}, {55}, {40}, {50}, {50}, {15}},
	{'I', {}, {}, {}, {}, {}, {}, {}},
	{'J', {}, {}, {}, {}, {}, {}, {}},
	{'K', {}, {}, {}, {}, {}, {}, {}},
	{'L', {}, {}, {}, {}, {}, {}, {}},
	{'M', {}, {}, {}, {}, {}, {}, {}},
	{'N', {}, {}, {}, {}, {}, {}, {}},
	{'O', {}, {}, {}, {}, {}, {}, {}},
	{'P', {}, {}, {}, {}, {}, {}, {}},
	{'Q', {}, {}, {}, {}, {}, {}, {}},
	{'R', {}, {}, {}, {}, {}, {}, {}},
	{'S', {}, {}, {}, {}, {}, {}, {}},
	{'T', {}, {}, {}, {}, {}, {}, {}},
	{'U', {}, {}, {}, {}, {}, {}, {}},
	{'V', {}, {}, {}, {}, {}, {}, {}},
	{'W', {}, {}, {}, {}, {}, {}, {}},
	{'X', {}, {}, {}, {}, {}, {}, {}},
	{'Y', {}, {}, {}, {}, {}, {}, {}},
	{'Z', {100, 100, 300}, {100, 100, 400}, {100, 100, 600}, {100, 100, 400}, {55, 1, 6}, {50, 2, 12}, {50, 3}},
};
