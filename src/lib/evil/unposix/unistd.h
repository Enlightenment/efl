#ifndef UNISTD_H
#define UNISTD_H

#include "unimplemented.h"

#include <evil_unistd.h>

#define F_OK 0
#define W_OK 2
#define R_OK 4
#define X_OK 0

UNIMPLEMENTED extern char *optarg;
UNIMPLEMENTED extern int optind, opterr, optopt;

#endif
