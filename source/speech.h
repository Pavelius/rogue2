#pragma once

enum speechn : unsigned char {
	HumanNames, ElfNames, DwarfNames, OrcNames,
};
const char* getspeech(speechn v, int index);