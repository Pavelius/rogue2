#pragma once

enum abilityn : unsigned char;
enum topicn : unsigned char {
	Manual, SkillsGeneral, SkillList, SkillAddNew,
};
extern const char* topic_info[];

void open_manual();
void open_manual(abilityn page);
void open_manual(topicn page);
