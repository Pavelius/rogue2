#include "answers.h"
#include "creature.h"
#include "draw.h"
#include "game.h"
#include "manual.h"

struct topici {
	topicn	parent;
	topicn	element;
};

static topici topics[] = {
	{Manual, SkillsGeneral},
	{Manual, RaceGeneral},
	{SkillsGeneral, SkillAddNew},
	{SkillsGeneral, SkillList},
};

static void add_answers(unsigned char f1, unsigned char f2, const char** names) {
	for(auto i = f1; i <= f2; i++)
		an.add(i, names[i]);
}

static void add_answers(topicn parent) {
	for(auto& e : topics) {
		if(e.parent == parent)
			an.add(e.element, getnm(e.element));
	}
}

static void add_content(topicn page) {
	switch(page) {
	case SkillList:
		add_answers(WeaponSkill, BalisticSkill, bsenum<abilityn>::names);
		add_answers(FirstSkill, LastSkill, bsenum<abilityn>::names);
		break;
	case RaceGeneral:
		add_answers(Human, Elf, bsenum<monstern>::names);
		break;
	default:
		add_answers(page);
		break;
	}
}

static void add(stringbuilder& sb, const abilityfni& v) {
	if(v.multiplier == 1 && v.divider == 1)
		return;
	sb.add("[~");
	sb.adds(getnm(v.ability));
	if(v.multiplier != 1)
		sb.add(" x %1i", v.multiplier);
	if(v.divider != 1)
		sb.add("/%1i", v.divider);
	sb.add("]");
}

void open_manual(abilityn page) {
	char temp[2048]; stringbuilder sb(temp);
	if(ability_info[page]) {
		sb.addn(ability_info[page]);
		sb.add("\n\n");
	}
	for(auto i = (abilityfnn)0; i <= LastAbilityFormula; i = (abilityfnn)(i + 1)) {
		auto& e = ability_formula[i];
		if(e.ability == page) {
			sb.addn("* \t");
			sb.add(ability_formula_names[i]);
			sb.add(".");
			add(sb, e);
		}
	}
	show_message(getnm(page), 0, temp, getnm(Cancel));
}

void open_manual(topicn page) {
	while(true) {
		add_content(page);
		show_message(getnm(page), 0, topic_info[page], getnm(Cancel));
		an.clear();
		auto next = getresult();
		if(!next)
			break;
		// Open next page in stack
		switch(page) {
		case SkillList: open_manual((abilityn)next); break;
		default: open_manual((topicn)next); break;
		}
	}
}

void open_manual() {
	open_manual(Manual);
}