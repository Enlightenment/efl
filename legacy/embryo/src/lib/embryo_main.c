#include "embryo_private.h"
#include <time.h>

static int _embryo_init_count = 0;

/*** EXPORTED CALLS ***/
   
int
embryo_init(void)
{
   _embryo_init_count++;
   if (_embryo_init_count > 1) return _embryo_init_count;
   
   srand(time(NULL));
   
   return _embryo_init_count;
}

int
embryo_shutdown(void)
{
   _embryo_init_count--;
   if (_embryo_init_count > 0) return _embryo_init_count;
   
   return _embryo_init_count;
}
