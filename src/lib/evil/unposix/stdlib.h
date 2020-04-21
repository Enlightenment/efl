#ifndef UNPOSIX_STDLIB_H
#define UNPOSIX_STDLIB_H

#include "unimplemented.h"

#include_next <stdlib.h>

UNIMPLEMENTED inline int mkstemp(char* template)
{
    #warning mkstemp is not implemented
    return 0;
}
UNIMPLEMENTED inline int mkostemp(char* template, int flags)
{
    #warning mkostemp is not implemented
    return 0;
}

UNIMPLEMENTED inline int mkostemps(char* template, int suffixlen, int flags)
{
    #warning mkostemps is not implemented
    return 0;
}

#include_next <stdlib.h>

#endif
