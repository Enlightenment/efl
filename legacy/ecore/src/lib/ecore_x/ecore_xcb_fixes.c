/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"


/**
 * @defgroup Ecore_X_Fixes_Group X Fixes Extension Functions
 *
 * Functions related to the X Fixes extension.
 */


#ifdef ECORE_XCB_FIXES
static int _xfixes_available = 0;
static xcb_xfixes_query_version_cookie_t _ecore_xcb_xfixes_init_cookie;
#endif /* ECORE_XCB_FIXES */


/* To avoid round trips, the initialization is separated in 2
   functions: _ecore_xcb_xfixes_init and
   _ecore_xcb_xfixes_init_finalize. The first one gets the cookies and
   the second one gets the replies and set the atoms. */

void
_ecore_x_xfixes_init(const xcb_query_extension_reply_t *reply)
{
#ifdef ECORE_XCB_FIXES
   if (reply && (reply->present))
      _ecore_xcb_xfixes_init_cookie = xcb_xfixes_query_version_unchecked(_ecore_xcb_conn, 4, 0);
#endif /* ECORE_XCB_FIXES */
}

void
_ecore_x_xfixes_init_finalize(void)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_query_version_reply_t *reply;

   reply = xcb_xfixes_query_version_reply(_ecore_xcb_conn,
                                          _ecore_xcb_xfixes_init_cookie, NULL);

   if (reply)
     {
        if (reply->major_version >= 3)
          _xfixes_available = 1;
        free(reply);
     }
#endif /* ECORE_XCB_FIXES */
}


/**
 * Return whether the X server supports the Fixes Extension.
 * @return 1 if the X Fixes Extension is available, 0 otherwise.
 *
 * Return 1 if the X server supports the Fixes Extension version 3.0,
 * 0 otherwise.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI int
ecore_x_xfixes_query(void)
{
#ifdef ECORE_XCB_FIXES
   return _xfixes_available;
#else
   return 0;
#endif /* ECORE_XCB_FIXES */
}


/**
 * Create a region from rectangles.
 * @param rects The rectangles used to initialize the region.
 * @param num   The number of rectangles.
 * @return      The newly created region.
 *
 * Create a region initialized to the specified list of rectangles
 * @p rects. The rectangles may be specified in any order, their union
 * becomes the region.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI Ecore_X_Region
ecore_x_region_new(Ecore_X_Rectangle *rects,
                   int                num)
{
   Ecore_X_Region region = XCB_NONE;

#ifdef ECORE_XCB_FIXES
   region = xcb_generate_id(_ecore_xcb_conn);
   xcb_xfixes_create_region(_ecore_xcb_conn, region, num, (xcb_rectangle_t *)rects);
#endif /* ECORE_XCB_FIXES */

   return region;
}


/**
 * Create a region from a pixmap.
 * @param bitmap The bitmap used to initialize the region.
 * @return       The newly created region.
 *
 * Creates a region initialized to the set of 'one' pixels in @p bitmap
 * (which must be of depth 1, else Match error).
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI Ecore_X_Region
ecore_x_region_new_from_bitmap(Ecore_X_Pixmap bitmap)
{
   Ecore_X_Region region = XCB_NONE;

#ifdef ECORE_XCB_FIXES
   region = xcb_generate_id(_ecore_xcb_conn);
   xcb_xfixes_create_region_from_bitmap(_ecore_xcb_conn, region, bitmap);
#endif /* ECORE_XCB_FIXES */

   return region;
}


/**
 * Create a region from a window.
 * @param window The window used to initialize the region.
 * @param type   The type of the region.
 * @return       The newly created region.
 *
 * Creates a region initialized to the specified @p window region. See
 * the Shape extension for the definition of Bounding and Clip
 * regions.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI Ecore_X_Region
ecore_x_region_new_from_window(Ecore_X_Window      window,
                               Ecore_X_Region_Type type)
{
   Ecore_X_Region region = XCB_NONE;

#ifdef ECORE_XCB_FIXES
   region = xcb_generate_id(_ecore_xcb_conn);
   xcb_xfixes_create_region_from_window(_ecore_xcb_conn, region, window, type);
#endif /* ECORE_XCB_FIXES */

   return region;
}


/**
 * Create a region from a graphic context.
 * @param gc The graphic context used to initialize the region.
 * @return   The newly created region.
 *
 * Creates a region initialized from the clip list of @p gc.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI Ecore_X_Region
ecore_x_region_new_from_gc(Ecore_X_GC gc)
{
   Ecore_X_Region region = XCB_NONE;

#ifdef ECORE_XCB_FIXES
   region = xcb_generate_id(_ecore_xcb_conn);
   xcb_xfixes_create_region_from_gc(_ecore_xcb_conn, region, gc);
#endif /* ECORE_XCB_FIXES */

   return region;
}


/**
 * Create a region from a picture.
 * @param picture The picture used to initialize the region.
 * @return        The newly created region.
 *
 * Creates a region initialized from the clip list of @p picture.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI Ecore_X_Region
ecore_x_region_new_from_picture(Ecore_X_Picture picture)
{
   Ecore_X_Region region = XCB_NONE;

#ifdef ECORE_XCB_FIXES
   region = xcb_generate_id(_ecore_xcb_conn);
   xcb_xfixes_create_region_from_picture(_ecore_xcb_conn, region, picture);
#endif /* ECORE_XCB_FIXES */

   return region;
}


/**
 * Destroy a region.
 * @param region The region to destroy.
 *
 * Destroy the specified @p region.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_del(Ecore_X_Region region)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_destroy_region(_ecore_xcb_conn, region);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Set the content of a region.
 * @param region The region to destroy.
 * @param rects  The rectangles used to set the region.
 * @param num    The number of rectangles.
 *
 * Replace the current contents of @p region with the region formed
 * by the union of the rectangles @p rects.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_set(Ecore_X_Region     region,
                   Ecore_X_Rectangle *rects,
                   int                num)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_set_region(_ecore_xcb_conn, region, num, (xcb_rectangle_t *)rects);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Copy the content of a region.
 * @param dest   The destination region.
 * @param source The source region.
 *
 * Replace the contents of @p dest with the contents of @p source.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_copy(Ecore_X_Region dest,
                    Ecore_X_Region source)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_copy_region(_ecore_xcb_conn, source, dest);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Make the union of two regions.
 * @param dest    The destination region.
 * @param source1 The first source region.
 * @param source2 The second source region.
 *
 * Replace the contents of @p dest with the union of @p source1 and
 * @p source2.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_combine(Ecore_X_Region dest,
                       Ecore_X_Region source1,
                       Ecore_X_Region source2)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_union_region(_ecore_xcb_conn, source1, source2, dest);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Make the intersection of two regions.
 * @param dest    The destination region.
 * @param source1 The first source region.
 * @param source2 The second source region.
 *
 * Replace the contents of @p dest with the intersection of @p source1 and
 * @p source2.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_intersect(Ecore_X_Region dest,
                         Ecore_X_Region source1,
                         Ecore_X_Region source2)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_intersect_region(_ecore_xcb_conn, source1, source2, dest);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Make the substraction of two regions.
 * @param dest    The destination region.
 * @param source1 The first source region.
 * @param source2 The second source region.
 *
 * Replace the contents of @p dest with the substraction of @p source1 by
 * @p source2.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_subtract(Ecore_X_Region dest,
                        Ecore_X_Region source1,
                        Ecore_X_Region source2)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_subtract_region(_ecore_xcb_conn, source1, source2, dest);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Make the substraction of regions by bounds.
 * @param dest   The destination region.
 * @param bounds The bounds.
 * @param source The source region.
 *
 * The @p source region is subtracted from the region specified by
 * @p bounds.  The result is placed in @p dest, replacing its
 * contents.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_invert(Ecore_X_Region     dest,
                      Ecore_X_Rectangle *bounds,
                      Ecore_X_Region     source)
{
#ifdef ECORE_XCB_FIXES
   xcb_rectangle_t rect;

   rect.x = bounds->x;
   rect.y = bounds->y;
   rect.width = bounds->width;
   rect.height = bounds->height;
   xcb_xfixes_invert_region(_ecore_xcb_conn, source, rect, dest);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Translate a region.
 * @param region The region to translate.
 * @param dx     The horizontal translation.
 * @param dy     The vertical translation.
 *
 * The @p region is translated by @p dx and @p dy in place.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_translate(Ecore_X_Region region,
                         int            dx,
                         int            dy)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_translate_region(_ecore_xcb_conn, region, dx, dy);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Extent a region.
 * @param dest   The destination region.
 * @param source The source region.
 *
 * The extents of the @p source region are placed in @p dest.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_extents(Ecore_X_Region dest,
                       Ecore_X_Region source)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_region_extents(_ecore_xcb_conn, source, dest);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Sends the XFixesFetchRegion request.
 * @param region Requested region.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_fetch_prefetch(Ecore_X_Region region)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_fetch_region_cookie_t cookie;

   cookie = xcb_xfixes_fetch_region_unchecked(_ecore_xcb_conn, region);
   _ecore_xcb_cookie_cache(cookie.sequence);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Gets the reply of the XFixesFetchRegion request sent by ecore_xcb_region_fetch_prefetch().
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_fetch_fetch(void)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_fetch_region_cookie_t cookie;
   xcb_xfixes_fetch_region_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_xfixes_fetch_region_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Return the rectangles that compose a region.
 * @param  region The region (Unused).
 * @param  num    The number of returned rectangles.
 * @param  bounds The returned bounds of the region.
 * @return        The returned rectangles.
 *
 * The @p region passed to ecore_xcb_region_fetch_prefetch() is
 * returned as a list of rectagles in XY-banded order.
 *
 * To use this function, you must call before, and in order,
 * ecore_xcb_region_fetch_prefetch(), which sends the XFixesFetchRegion request,
 * then ecore_xcb_region_fetch_fetch(), which gets the reply.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI Ecore_X_Rectangle *
ecore_x_region_fetch(Ecore_X_Region     region __UNUSED__,
                     int               *num,
                     Ecore_X_Rectangle *bounds)
{
   Ecore_X_Rectangle  extents = { 0, 0, 0, 0};
   Ecore_X_Rectangle *rects = NULL;
#ifdef ECORE_XCB_FIXES
   int                n;
   xcb_xfixes_fetch_region_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     {
       if (num) *num = 0;
       if (bounds) *bounds = extents;
       return NULL;
     }
   n = xcb_xfixes_fetch_region_rectangles_length(reply);
   rects = (Ecore_X_Rectangle *)malloc(n * sizeof(Ecore_X_Rectangle));
   if (!rects)
     {
       if (num) *num = 0;
       if (bounds) *bounds = extents;

       return NULL;
     }

   if (num) *num = n;
   if (bounds)
     {
        bounds->x = reply->extents.x;
        bounds->y = reply->extents.y;
        bounds->width = reply->extents.width;
        bounds->height = reply->extents.height;
     }
   memcpy(rects,
          xcb_xfixes_fetch_region_rectangles(reply),
          sizeof(Ecore_X_Rectangle) * n);

   return rects;
#else
   if (num) *num = 0;
   if (bounds) *bounds = extents;
   return NULL;
#endif /* ECORE_XCB_FIXES */
}


/**
 * Expand a region.
 * @param dest   The destination region.
 * @param source The source region.
 * @param left   The number of pixels to add on the left.
 * @param right  The number of pixels to add on the right.
 * @param top    The number of pixels to add at the top.
 * @param bottom The number of pixels to add at the bottom.
 *
 * Put in @p dest the area specified by expanding each rectangle in
 * the @p source  region by the specified number of pixels to the
 * @p left, @p right, @p top and @p bottom.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_expand(Ecore_X_Region dest,
                      Ecore_X_Region source,
                      unsigned int   left,
                      unsigned int   right,
                      unsigned int   top,
                      unsigned int   bottom)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_expand_region(_ecore_xcb_conn, source, dest, left, right, top, bottom);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Change clip-mask in a graphic context to the specified region.
 * @param region   The region to change.
 * @param gc       The clip-mask graphic context.
 * @param x_origin The horizontal translation.
 * @param y_origin The vertical translation.
 *
 * Changes clip-mask in @p gc to the specified @p region and
 * sets the clip origin with the values of @p x_origin and @p y_origin.
 * Output will be clippped to remain contained within the region. The
 * clip origin is interpreted relative to the origin of whatever
 * destination drawable is specified in a graphics request. The
 * region is interpreted relative to the clip origin. Future changes
 * to region have no effect on the gc clip-mask.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_gc_clip_set(Ecore_X_Region region,
                           Ecore_X_GC     gc,
                           int            x_origin,
                           int            y_origin)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_set_gc_clip_region(_ecore_xcb_conn, gc, region, x_origin, y_origin);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Change the shape extension of a window.
 * @param region   The region.
 * @param dest     The window whose shape is changed.
 * @param type     The kind of shape.
 * @param x_offset The horizontal offset.
 * @param y_offset The vertical offset.
 *
 * Set the specified Shape extension region of @p window to @p region,
 * offset by @p x_offset and @p y_offset. Future changes to region
 * have no effect on the window shape.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_window_shape_set(Ecore_X_Region     region,
                                Ecore_X_Window     dest,
                                Ecore_X_Shape_Type type,
                                int                x_offset,
                                int                y_offset)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_set_window_shape_region(_ecore_xcb_conn, dest, type, x_offset, y_offset, region);
#endif /* ECORE_XCB_FIXES */
}


/**
 * Change clip-mask in picture to the specified region.
 * @param region   The region.
 * @param picture  The picture.
 * @param x_origin The X coordinate of the origin.
 * @param y_origin The Y coordinate of the origin.
 *
 * Changes clip-mask in picture to the specified @p region
 * and sets the clip origin.  Input and output will be clipped to
 * remain contained within the region.  The clip origin is interpreted
 * relative to the origin of the drawable associated with @p picture.  The
 * region is interpreted relative to the clip origin.  Future changes
 * to region have no effect on the picture clip-mask.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_picture_clip_set(Ecore_X_Region  region,
                                Ecore_X_Picture picture,
                                int             x_origin,
                                int             y_origin)
{
#ifdef ECORE_XCB_FIXES
   xcb_xfixes_set_picture_clip_region(_ecore_xcb_conn, picture, region, x_origin, y_origin);
#endif /* ECORE_XCB_FIXES */
}
