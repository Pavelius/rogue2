#include "bsdata.h"
#include "io_stream.h"
#include "resid.h"
#include "stringbuilder.h"

struct residi {
	const char*	id;
	sprite*		data;
	bool		error;
};

BSDATA(residi) = {
	{"pc_body"},
	{"pc_arms"},
	{"pc_accessories"},
	{"fow"},
	{"los"},
	{"floor"},
	{"borders"},
	{"decals"},
	{"features"},
	{"items"},
	{"monsters"},
	{"walls"},
	{"shadows"},
};
static_assert(lenghtof(bsdata<residi>::elements)==(ResShadows+1), "Invalid resources count");

sprite* gres(resid v) {
	auto& ei = bsdata<residi>::elements[v];
	if(ei.data)
		return ei.data;
	if(ei.error)
		return 0;
	ei.data = (sprite*)loadb(str("art/%1.pma", ei.id));
	if(!ei.data)
		ei.error = true;
	return ei.data;
}