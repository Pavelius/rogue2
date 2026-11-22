#pragma once

enum abilityn : unsigned char;
enum topicn : unsigned char {
	Manual,
	SkillsGeneral, SkillList, SkillAddNew,
	RaceGeneral,
	LastTopic = RaceGeneral
};
extern const char* topic_names[LastTopic + 1];
extern const char* topic_info[LastTopic + 1];

void open_manual();
void open_manual(abilityn page);
void open_manual(topicn page);

inline const char* getnm(topicn v) { return topic_names[v]; }
