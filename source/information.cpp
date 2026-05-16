#include "bsdata.h"
#include "dice.h"
#include "stringbuilder.h"
#include "stringvar.h"

static void list_of_feats(stringbuilder& sb) {
}

BSDATA(stringvari) = {
	{"ListOfFeats", list_of_feats},
};
BSDATAF(stringvari)