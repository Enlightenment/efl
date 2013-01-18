#ifndef _EUNIT_TESTS_H
#define _EUNIT_TESTS_H

#define _EUNIT_EXIT_CODE 1

#define fail_if(x) \
   do { \
        if (x) \
          { \
             fprintf(stderr, "%s:%d - Failure '%s' is TRUE.\n", \
                   __FILE__, __LINE__, # x); \
             exit(_EUNIT_EXIT_CODE); \
          } \
   } while (0)

#endif

