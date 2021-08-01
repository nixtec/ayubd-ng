/* Perfect hash definitions */
#ifndef STANDARD
#include "standard.h"
#endif /* STANDARD */
#ifndef PHASH
#define PHASH

#define PHASHNKEYS 0  /* How many keys were hashed */
#define PHASHRANGE 1  /* Range any input might map to */
#define PHASHSALT 0x9e3779b9 /* internal, initialize normal hash */

ub4 phash();

#endif  /* PHASH */

