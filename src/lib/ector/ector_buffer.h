#include <Ector.h>

#include "ector_generic_buffer.eo.h"
#include "software/ector_software_buffer_base.eo.h"

typedef struct _Ector_Generic_Buffer_Data
{
   Eo                 *eo;
   unsigned int        w, h;
   unsigned char       l, r, t, b;
   Efl_Gfx_Colorspace  cspace;
} Ector_Generic_Buffer_Data;

typedef struct _Ector_Software_Buffer_Base_Data
{
   Ector_Generic_Buffer_Data *generic; /* ugly */
   union {
      unsigned int     *u32;
      unsigned char    *u8;
   } pixels;
   unsigned int         stride;
   unsigned int         pixel_size; // in bytes
   unsigned int         map_count;
   Eina_Bool            writable : 1;
   Eina_Bool            nofree : 1; // pixel data should not be free()'ed
   Eina_Bool            span_free : 1;
} Ector_Software_Buffer_Base_Data;
