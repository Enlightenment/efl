#include "ecore_xcb_private.h"

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
ecore_x_gc_new(Ecore_X_Drawable      drawable,
               Ecore_X_GC_Value_Mask value_mask,
               const unsigned int   *value_list)
{
   xcb_gcontext_t gc;
   uint32_t vmask = 0;
   int i = 0, mask = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!drawable) drawable = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   for (i = 0, mask = 1; i <= 22; i++, mask <<= 1)
     {
        switch (mask & value_mask)
          {
           case ECORE_X_GC_VALUE_MASK_FUNCTION:
             vmask |= XCB_GC_FUNCTION;
             break;

           case ECORE_X_GC_VALUE_MASK_PLANE_MASK:
             vmask |= XCB_GC_PLANE_MASK;
             break;

           case ECORE_X_GC_VALUE_MASK_FOREGROUND:
             vmask |= XCB_GC_FOREGROUND;
             break;

           case ECORE_X_GC_VALUE_MASK_BACKGROUND:
             vmask |= XCB_GC_BACKGROUND;
             break;

           case ECORE_X_GC_VALUE_MASK_LINE_WIDTH:
             vmask |= XCB_GC_LINE_WIDTH;
             break;

           case ECORE_X_GC_VALUE_MASK_LINE_STYLE:
             vmask |= XCB_GC_LINE_STYLE;
             break;

           case ECORE_X_GC_VALUE_MASK_CAP_STYLE:
             vmask |= XCB_GC_CAP_STYLE;
             break;

           case ECORE_X_GC_VALUE_MASK_JOIN_STYLE:
             vmask |= XCB_GC_JOIN_STYLE;
             break;

           case ECORE_X_GC_VALUE_MASK_FILL_STYLE:
             vmask |= XCB_GC_FILL_STYLE;
             break;

           case ECORE_X_GC_VALUE_MASK_FILL_RULE:
             vmask |= XCB_GC_FILL_RULE;
             break;

           case ECORE_X_GC_VALUE_MASK_TILE:
             vmask |= XCB_GC_TILE;
             break;

           case ECORE_X_GC_VALUE_MASK_STIPPLE:
             vmask |= XCB_GC_STIPPLE;
             break;

           case ECORE_X_GC_VALUE_MASK_TILE_STIPPLE_ORIGIN_X:
             vmask |= XCB_GC_TILE_STIPPLE_ORIGIN_X;
             break;

           case ECORE_X_GC_VALUE_MASK_TILE_STIPPLE_ORIGIN_Y:
             vmask |= XCB_GC_TILE_STIPPLE_ORIGIN_Y;
             break;

           case ECORE_X_GC_VALUE_MASK_FONT:
             vmask |= XCB_GC_FONT;
             break;

           case ECORE_X_GC_VALUE_MASK_SUBWINDOW_MODE:
             vmask |= XCB_GC_SUBWINDOW_MODE;
             break;

           case ECORE_X_GC_VALUE_MASK_GRAPHICS_EXPOSURES:
             vmask |= XCB_GC_GRAPHICS_EXPOSURES;
             break;

           case ECORE_X_GC_VALUE_MASK_CLIP_ORIGIN_X:
             vmask |= XCB_GC_CLIP_ORIGIN_X;
             break;

           case ECORE_X_GC_VALUE_MASK_CLIP_ORIGIN_Y:
             vmask |= XCB_GC_CLIP_ORIGIN_Y;
             break;

           case ECORE_X_GC_VALUE_MASK_CLIP_MASK:
             vmask |= XCB_GC_CLIP_MASK;
             break;

           case ECORE_X_GC_VALUE_MASK_DASH_OFFSET:
             vmask |= XCB_GC_DASH_OFFSET;
             break;

           case ECORE_X_GC_VALUE_MASK_DASH_LIST:
             vmask |= XCB_GC_DASH_LIST;
             break;

           case ECORE_X_GC_VALUE_MASK_ARC_MODE:
             vmask |= XCB_GC_ARC_MODE;
             break;
          }
     }

   gc = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_gc(_ecore_xcb_conn, gc, drawable, vmask, value_list);

//   ecore_x_flush();
   return gc;
}

/**
 * Deletes and frees the given graphics context.
 * @param gc The given graphics context.
 */
EAPI void
ecore_x_gc_free(Ecore_X_GC gc)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   xcb_free_gc(_ecore_xcb_conn, gc);
//   ecore_x_flush();
}

EAPI void
ecore_x_gc_foreground_set(Ecore_X_GC    gc,
                          unsigned long foreground)
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   list = foreground;
   xcb_change_gc(_ecore_xcb_conn, gc, XCB_GC_FOREGROUND, &list);
//   ecore_x_flush();
}

EAPI void
ecore_x_gc_background_set(Ecore_X_GC    gc,
                          unsigned long background)
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   list = background;
   xcb_change_gc(_ecore_xcb_conn, gc, XCB_GC_BACKGROUND, &list);
//   ecore_x_flush();
}

