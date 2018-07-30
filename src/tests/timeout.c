#include <unistd.h>

#if defined(__clang__)
# pragma clang diagnostic ignored "-Wunused-parameter"
#elif (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
# pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


int
main(int arc, char *argv[])
{
   sleep(60);
   return 0;
}
