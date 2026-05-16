#include "bsdata.h"
#include "stringvar.h"

bool stringvar_identifier(stringbuilder& sb, const char* identifier) {
	auto pn = bsdata<stringvari>::find(identifier);
	if(pn) {
		pn->proc(sb);
		return true;
	}
	return false;
}