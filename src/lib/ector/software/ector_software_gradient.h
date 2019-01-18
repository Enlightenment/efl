#ifndef ECTOR_SOFTWARE_GRADIENT_H
# define ECTOR_SOFTWARE_GRADIENT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <math.h>

#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"
#include "draw.h"

#define GRADIENT_STOPTABLE_SIZE 1024
#define FIXPT_BITS 8
#define FIXPT_SIZE (1<<FIXPT_BITS)

static inline int
_gradient_clamp(const Ector_Renderer_Software_Gradient_Data *data, int ipos)
{
   int limit;

   if (data->gd->s == EFL_GFX_GRADIENT_SPREAD_REPEAT)
     {
        ipos = ipos % GRADIENT_STOPTABLE_SIZE;
        ipos = ipos < 0 ? GRADIENT_STOPTABLE_SIZE + ipos : ipos;
     }
   else if (data->gd->s == EFL_GFX_GRADIENT_SPREAD_REFLECT)
     {
        limit = GRADIENT_STOPTABLE_SIZE * 2;
        ipos = ipos % limit;
        ipos = ipos < 0 ? limit + ipos : ipos;
        ipos = ipos >= GRADIENT_STOPTABLE_SIZE ? limit - 1 - ipos : ipos;
     }
   else
     {
        if (ipos < 0) ipos = 0;
        else if (ipos >= GRADIENT_STOPTABLE_SIZE)
          ipos = GRADIENT_STOPTABLE_SIZE-1;
     }
   return ipos;
}

static inline uint32_t
_gradient_pixel_fixed(const Ector_Renderer_Software_Gradient_Data *data, int fixed_pos)
{
   int ipos = (fixed_pos + (FIXPT_SIZE / 2)) >> FIXPT_BITS;

   return data->color_table[_gradient_clamp(data, ipos)];
}

static inline uint32_t
_gradient_pixel(const Ector_Renderer_Software_Gradient_Data *data, float pos)
{
   int ipos = (int)(pos * (GRADIENT_STOPTABLE_SIZE - 1) + (float)(0.5));

   return data->color_table[_gradient_clamp(data, ipos)];
}

#endif
