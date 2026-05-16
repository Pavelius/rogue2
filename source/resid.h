#pragma once

struct sprite;

enum resid : unsigned char {
	ResPCBody, ResPCArms, ResPCAccessories,
	ResFow, ResLos,
	ResFloor, ResBorders, ResDecals, ResFeatures, ResItems, ResMonsters,
	ResWalls, ResShadows,
};

sprite* gres(resid id);