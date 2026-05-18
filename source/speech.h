#pragma once

enum speechn : unsigned char {
	HumanNames, ElfNames, DwarfNames, OrcNames,
	SayHello,
};
const char* getspeech(speechn v, int index);