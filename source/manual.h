#pragma once

enum abilityn : unsigned char;
enum topicn : unsigned char {
	Manual, SkillList,
};

void open_manual(abilityn page);
void open_manual(topicn page);
