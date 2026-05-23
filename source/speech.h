#pragma once

enum speechn : unsigned char {
	SayHello,
};
const char* getspeech(speechn v, int index);