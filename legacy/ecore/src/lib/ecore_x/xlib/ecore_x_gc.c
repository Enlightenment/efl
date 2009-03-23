/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/**
 * Creates a new default graphics context associated with the given
 * drawable.
 * @param  draw Drawable to create graphics context with.  If @c 0 is
 *              given instead, the default root window is used.
 * @param value_mask Bitmask values.
 * @param value_list List of values. The order of values must be the
 *                   same than the corresponding bitmaks.
 * @return The new default graphics context.
 */
EAPI Ecore_X_GC
ecore_x_gc_new(Ecore_X_Drawable draw, Ecore_X_GC_Value_Mask value_mask, const unsigned int *value_list)
{
   XGCValues gcv;
   int       mask;
   int       index;
   int       i;

   if (!draw) draw = DefaultRootWindow(_ecore_x_disp);

   for (i = 0, index = 0, mask = 1; i <= 22; i++, mask <<= 1)
     {
        switch (mask & value_mask)
          {
           case ECORE_X_GC_VALUE_MASK_FUNCTION:
              gcv.function = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_PLANE_MASK:
              gcv.plane_mask = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_FOREGROUND:
              gcv.foreground = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_BACKGROUND:
              gcv.background = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_LINE_WIDTH:
              gcv.line_width = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_LINE_STYLE:
              gcv.line_style = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_CAP_STYLE:
              gcv.cap_style = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_JOIN_STYLE:
              gcv.join_style = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_FILL_STYLE:
              gcv.fill_style = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_FILL_RULE:
              gcv.fill_rule = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_TILE:
              gcv.tile = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_STIPPLE:
              gcv.stipple = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_TILE_STIPPLE_ORIGIN_X:
              gcv.ts_x_origin = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_TILE_STIPPLE_ORIGIN_Y:
              gcv.ts_y_origin = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_FONT:
              gcv.font = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_SUBWINDOW_MODE:
              gcv.subwindow_mode = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_GRAPHICS_EXPOSURES:
              gcv.graphics_exposures = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_CLIP_ORIGIN_X:
              gcv.clip_x_origin = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_CLIP_ORIGIN_Y:
              gcv.clip_y_origin = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_CLIP_MASK:
              gcv.clip_mask = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_DASH_OFFSET:
              gcv.dash_offset = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_DASH_LIST:
              gcv.dashes = value_list[index];
              index++;
              break;
           case ECORE_X_GC_VALUE_MASK_ARC_MODE:
              gcv.arc_mode = value_list[index];
              index++;
              break;
          }
     }

   return XCreateGC(_ecore_x_disp, draw, value_mask, &gcv);
}

/**
 * Deletes and frees the given graphics context.
 * @param gc The given graphics context.
 */
EAPI void
ecore_x_gc_del(Ecore_X_GC gc)
{
   XFreeGC(_ecore_x_disp, gc);
}
