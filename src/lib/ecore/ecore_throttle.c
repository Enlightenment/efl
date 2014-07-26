#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>

#include "Ecore.h"
#include "ecore_private.h"

static int throttle_val = 0;

EAPI void
ecore_throttle_adjust(double amount)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   int adj = amount * 1000000.0;
   throttle_val += adj;
   if (throttle_val < 0) throttle_val = 0;
}

EAPI double
ecore_throttle_get(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0.0);
   return (double)throttle_val / 1000000.0;
}

void
_ecore_throttle(void)
{
   if (throttle_val <= 0) return;
   usleep(throttle_val);
}

