#ifndef UNPOSIX_GETOPT_H
#define UNPOSIX_GETOPT_H

#include "unimplemented.h"

#include <unistd.h>

UNIMPLEMENTED int getopt(int argc, char * const argv[],
          const char *optstring)
{
#warning getopt is not implemented
}

#include_next <getopt.h>

UNIMPLEMENTED int getopt_long(int argc, char * const argv[],
          const char *optstring,
          const struct option *longopts, int *longindex)
{
#warning getopt_long is not implemented
}

UNIMPLEMENTED int getopt_long_only(int argc, char * const argv[],
          const char *optstring,
          const struct option *longopts, int *longindex)
{
#warning getopt_long_only is not implemented
}
