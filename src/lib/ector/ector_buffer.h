#ifndef ECTOR_BUFFER_H
#define ECTOR_BUFFER_H

#include <Ector.h>

/**
 * @typedef Ector_Buffer
 * A generic pixel buffer type (2D). May be readable or writeable or both.
 */
typedef Eo Ector_Buffer;

/* Avoid type redefinition */
#define _ECTOR_BUFFER_EO_CLASS_TYPE

#include "ector_buffer.eo.h"
#include "software/ector_software_buffer_base.eo.h"


typedef struct _Ector_Buffer_Data Ector_Buffer_Data;
typedef struct _Ector_Software_Buffer_Base_Data Ector_Software_Buffer_Base_Data;

struct _Ector_Buffer_Data
{
   Ector_Buffer       *eo;
   unsigned int        w, h;
   Efl_Gfx_Colorspace  cspace;
   Eina_Bool           immutable : 1; // pixels_set is forbidden
};

struct _Ector_Software_Buffer_Base_Data
{
   Ector_Buffer_Data *generic;
   union {
      unsigned int     *u32;
      unsigned char    *u8;
   } pixels;
   unsigned int         stride;
   unsigned int         pixel_size; // in bytes
   struct {
      Eina_Inlist      *maps; // Ector_Software_Buffer_Map
   } internal;
   Eina_Bool            writable : 1; // pixels can be written to
   Eina_Bool            nofree : 1; // pixel data should not be free()'ed
};

#endif
