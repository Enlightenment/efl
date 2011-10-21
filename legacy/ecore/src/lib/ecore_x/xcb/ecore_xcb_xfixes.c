#include "ecore_xcb_private.h"
# ifdef ECORE_XCB_XFIXES
#  include <xcb/xfixes.h>
# endif

/* local function prototypes */
static xcb_rectangle_t   *_ecore_xcb_rect_to_xcb(Ecore_X_Rectangle *rects,
                                                 int                num);
static Ecore_X_Rectangle *_ecore_xcb_rect_to_ecore(xcb_rectangle_t *rects,
                                                   int              num);

/* local variables */
static Eina_Bool _xfixes_avail = EINA_FALSE;

/* external variables */
int _ecore_xcb_event_xfixes = -1;

void
_ecore_xcb_xfixes_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_XFIXES
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_xfixes_id);
#endif
}

void
_ecore_xcb_xfixes_finalize(void)
{
#ifdef ECORE_XCB_XFIXES
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_XFIXES
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_xfixes_id);
   if ((ext_reply) && (ext_reply->present))
     {
        xcb_xfixes_query_version_cookie_t cookie;
        xcb_xfixes_query_version_reply_t *reply;

        cookie =
          xcb_xfixes_query_version_unchecked(_ecore_xcb_conn,
                                             XCB_XFIXES_MAJOR_VERSION,
                                             XCB_XFIXES_MINOR_VERSION);
        reply = xcb_xfixes_query_version_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply)
          {
             /* NB: XFixes Extension >= 3 needed for shape stuff.
              * for now, I am removing this check so that it matches the
              * xlib code closer. If the extension version ends up being
              * that important, then re-enable this */

     /* if (reply->major_version >= 3) */
                _xfixes_avail = EINA_TRUE;
                free(reply);
          }

        if (_xfixes_avail)
          _ecore_xcb_event_xfixes = ext_reply->first_event;
     }
#endif
}

EAPI Eina_Bool
ecore_x_fixes_selection_notification_request(Ecore_X_Atom selection)
{
#ifdef ECORE_XCB_XFIXES
   Ecore_X_Window root = 0;
   xcb_void_cookie_t cookie;
   xcb_generic_error_t *err;
   int mask = 0;
#endif

   CHECK_XCB_CONN;

   if (!_xfixes_avail) return EINA_FALSE;

#ifdef ECORE_XCB_XFIXES
   root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   mask = (XCB_XFIXES_SELECTION_EVENT_MASK_SET_SELECTION_OWNER |
           XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_WINDOW_DESTROY |
           XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_CLIENT_CLOSE);

   cookie =
     xcb_xfixes_select_selection_input_checked(_ecore_xcb_conn, root,
                                               selection, mask);
   err = xcb_request_check(_ecore_xcb_conn, cookie);
   if (err)
     {
        free(err);
        return EINA_FALSE;
     }

   return EINA_TRUE;
#endif
   return EINA_FALSE;
}

Eina_Bool
_ecore_xcb_xfixes_avail_get(void)
{
   return _xfixes_avail;
}

/**
 * @defgroup Ecore_X_Fixes_Group X Fixes Extension Functions
 *
 * Functions related to the X Fixes extension.
 */

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
   Ecore_X_Region region = 0;
#ifdef ECORE_XCB_XFIXES
   xcb_rectangle_t *xrects;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return 0;

#ifdef ECORE_XCB_XFIXES
   xrects = _ecore_xcb_rect_to_xcb(rects, num);
   region = xcb_generate_id(_ecore_xcb_conn);
   xcb_xfixes_create_region(_ecore_xcb_conn, region, num, xrects);
   free(xrects);
//   ecore_x_flush();
#endif

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
   Ecore_X_Region region = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return 0;

#ifdef ECORE_XCB_XFIXES
   region = xcb_generate_id(_ecore_xcb_conn);
   xcb_xfixes_create_region_from_bitmap(_ecore_xcb_conn, region, bitmap);
//   ecore_x_flush();
#endif

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
ecore_x_region_new_from_window(Ecore_X_Window      win,
                               Ecore_X_Region_Type type)
{
   Ecore_X_Region region = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return 0;

#ifdef ECORE_XCB_XFIXES
   region = xcb_generate_id(_ecore_xcb_conn);
   xcb_xfixes_create_region_from_window(_ecore_xcb_conn, region, win, type);
//   ecore_x_flush();
#endif

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
   Ecore_X_Region region = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return 0;

#ifdef ECORE_XCB_XFIXES
   region = xcb_generate_id(_ecore_xcb_conn);
   xcb_xfixes_create_region_from_gc(_ecore_xcb_conn, region, gc);
//   ecore_x_flush();
#endif

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
   Ecore_X_Region region = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return 0;

#ifdef ECORE_XCB_XFIXES
   region = xcb_generate_id(_ecore_xcb_conn);
   xcb_xfixes_create_region_from_picture(_ecore_xcb_conn, region, picture);
//   ecore_x_flush();
#endif

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
ecore_x_region_free(Ecore_X_Region region)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_destroy_region(_ecore_xcb_conn, region);
//   ecore_x_flush();
#endif
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
#ifdef ECORE_XCB_XFIXES
   xcb_rectangle_t *xrects;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xrects = _ecore_xcb_rect_to_xcb(rects, num);
   xcb_xfixes_set_region(_ecore_xcb_conn, region, num, xrects);
   free(xrects);
//   ecore_x_flush();
#endif
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

   // NB: Hmmmm...this may need converting to/fro xcb_rectangle_t
#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_copy_region(_ecore_xcb_conn, source, dest);
//   ecore_x_flush();
#endif
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_union_region(_ecore_xcb_conn, source1, source2, dest);
//   ecore_x_flush();
#endif
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_intersect_region(_ecore_xcb_conn, source1, source2, dest);
//   ecore_x_flush();
#endif
}

/**
 * Make the subtraction of two regions.
 * @param dest    The destination region.
 * @param source1 The first source region.
 * @param source2 The second source region.
 *
 * Replace the contents of @p dest with the subtraction of @p source1 by
 * @p source2.
 * @ingroup Ecore_X_Fixes_Group
 */
EAPI void
ecore_x_region_subtract(Ecore_X_Region dest,
                        Ecore_X_Region source1,
                        Ecore_X_Region source2)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_subtract_region(_ecore_xcb_conn, source1, source2, dest);
//   ecore_x_flush();
#endif
}

/**
 * Make the subtraction of regions by bounds.
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
#ifdef ECORE_XCB_XFIXES
   xcb_rectangle_t xrects;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xrects.x = bounds->x;
   xrects.y = bounds->y;
   xrects.width = bounds->width;
   xrects.height = bounds->height;

   xcb_xfixes_invert_region(_ecore_xcb_conn, source, xrects, dest);
//   ecore_x_flush();
#endif
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_translate_region(_ecore_xcb_conn, region, dx, dy);
//   ecore_x_flush();
#endif
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_region_extents(_ecore_xcb_conn, source, dest);
//   ecore_x_flush();
#endif
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
ecore_x_region_fetch(Ecore_X_Region     region,
                     int               *num,
                     Ecore_X_Rectangle *bounds)
{
   Ecore_X_Rectangle extents = { 0, 0, 0, 0 };
   Ecore_X_Rectangle *rects = NULL;
#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_fetch_region_cookie_t cookie;
   xcb_xfixes_fetch_region_reply_t *reply;
   xcb_rectangle_t *r;
   int n = 0;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (num) *num = 0;
   if (bounds) *bounds = extents;
   if (!_xfixes_avail) return NULL;

#ifdef ECORE_XCB_XFIXES
   cookie = xcb_xfixes_fetch_region_unchecked(_ecore_xcb_conn, region);
   reply = xcb_xfixes_fetch_region_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return NULL;

   r = xcb_xfixes_fetch_region_rectangles(reply);
   n = xcb_xfixes_fetch_region_rectangles_length(reply);
   rects = _ecore_xcb_rect_to_ecore(r, n);
   if (num) *num = n;

   /* rects = (Ecore_X_Rectangle *)malloc(n * sizeof(Ecore_X_Rectangle)); */
   /* if (!rects)  */
   /*   { */
   /*      free(reply); */
   /*      return NULL; */
   /*   } */

   /* for (i = 0; i < n; i++)  */
   /*   { */
   /*      rects[i].x = r[i].x; */
   /*      rects[i].y = r[i].y; */
   /*      rects[i].width = r[i].width; */
   /*      rects[i].height = r[i].height; */
   /*   } */

   (*bounds).x = reply->extents.x;
   (*bounds).y = reply->extents.y;
   (*bounds).width = reply->extents.width;
   (*bounds).height = reply->extents.height;

   free(reply);
#endif

   return rects;
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_expand_region(_ecore_xcb_conn, source, dest, left, right, top, bottom);
//   ecore_x_flush();
#endif
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
                           int            x,
                           int            y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_set_gc_clip_region(_ecore_xcb_conn, gc, region, x, y);
//   ecore_x_flush();
#endif
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
                                int                x,
                                int                y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_set_window_shape_region(_ecore_xcb_conn, dest, type, x, y, region);
//   ecore_x_flush();
#endif
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
                                int             x,
                                int             y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_xfixes_avail) return;

#ifdef ECORE_XCB_XFIXES
   xcb_xfixes_set_picture_clip_region(_ecore_xcb_conn, picture, region, x, y);
//   ecore_x_flush();
#endif
}

/* local function prototypes */
static xcb_rectangle_t *
_ecore_xcb_rect_to_xcb(Ecore_X_Rectangle *rects,
                       int                num)
{
   xcb_rectangle_t *xrect;
   int i = 0;

   if (!num) return NULL;

   xrect = malloc(sizeof(xcb_rectangle_t) * num);
   if (!xrect) return NULL;

   for (i = 0; i < num; i++)
     {
        xrect[i].x = rects[i].x;
        xrect[i].y = rects[i].y;
        xrect[i].width = rects[i].width;
        xrect[i].height = rects[i].height;
     }

   return xrect;
}

static Ecore_X_Rectangle *
_ecore_xcb_rect_to_ecore(xcb_rectangle_t *rects,
                         int              num)
{
   Ecore_X_Rectangle *erect;
   int i = 0;

   if (!num) return NULL;

   erect = malloc(sizeof(Ecore_X_Rectangle) * num);
   if (!erect) return NULL;

   for (i = 0; i < num; i++)
     {
        erect[i].x = rects[i].x;
        erect[i].y = rects[i].y;
        erect[i].width = rects[i].width;
        erect[i].height = rects[i].height;
     }

   return erect;
}

