#include "bsdata.h"
#include "shape.h"

static char shape_cave[] =
"  XXXXXXX  "
"XXX.....XXX"
"X.........X"
"X....0....X"
"XXX.....XXX"
"  XXX1XXX  "
;

BSDATA(shapei) = {
	{{11, 6}, shape_cave},
};