#include "evas_common.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

static void blend_init_pow_lut(void);

DATA8        pow_lut[256][256];
const DATA16 const_c1[4]       = {1, 1, 1, 1};

void
blend_init(void)
{
   static int initialised = 0;
   static int mmx = 0;
   static int sse = 0;
   static int sse2 = 0;
   
   if (initialised) return;
   initialised = 1;
   
   cpu_can_do(&mmx, &sse, &sse2);
   
   blend_init_pow_lut();   
}

void
blend_init_pow_lut(void)
{
   int i, j;
   
   for (i = 0; i < 256; i++)
     {
	for (j = 0; j < 256; j++)
	  {
	     int divisor;
	     
	     divisor = (i + (j * (255 - i)) / 255);
	     if (divisor > 0) pow_lut[i][j] = (i * 255) / divisor;
	     else             pow_lut[i][j] = 0;
	  }
     }
}
