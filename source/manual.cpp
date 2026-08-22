#include "answers.h"
#include "creature.h"
#include "game.h"
#include "manual.h"
#include "message.h"

static void add_answers(unsigned char f1, unsigned char f2, const char** names) {
	for(auto i = f1; i <= f2; i++)
		an.add(i, names[i]);
}

static void add_content(topicn page) {
	switch(page) {
	case SkillList:
		add_answers(Strenght, BalisticSkill, bsenum<abilityn>::names);
		add_answers(FirstSkill, LastSkill, bsenum<abilityn>::names);
		break;
	default:
		break;
	}
}

void open_manual(topicn page) {
	an.clear();
	add_content(page);
	show_message(getname(page), 0, bsenum<topicn>::info[page], getname(Cancel));
}