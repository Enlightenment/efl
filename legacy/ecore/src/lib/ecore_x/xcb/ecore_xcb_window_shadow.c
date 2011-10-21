#include "ecore_xcb_private.h"

typedef struct _Shadow Shadow;
struct _Shadow
{
   Shadow        *parent, **children;
   Ecore_X_Window win;
   int            children_num;
   short          x, y;
   unsigned short w, h;
};

static Eina_Bool _inside_rects(Shadow            *s,
                               int                x,
                               int                y,
                               int                bx,
                               int                by,
                               Ecore_X_Rectangle *rects,
                               int                num);

//static int shadow_count = 0;
static Shadow **shadow_base = NULL;
static int shadow_num = 0;

/* FIXME: round trips */
static Shadow *
_ecore_x_window_tree_walk(Ecore_X_Window window)
{
   Shadow *s, **sl;
   xcb_get_window_attributes_reply_t *reply_attr;
   xcb_get_geometry_reply_t *reply_geom;
   xcb_query_tree_reply_t *reply_tree;
   xcb_get_window_attributes_cookie_t cookie_attr;
   xcb_get_geometry_cookie_t cookie_geom;
   xcb_query_tree_cookie_t cookie_tree;
   int i, j;

   CHECK_XCB_CONN;

   cookie_attr = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, window);
   reply_attr = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie_attr, NULL);
   if (!reply_attr) return NULL;
   if (reply_attr->map_state != XCB_MAP_STATE_VIEWABLE)
     {
        free(reply_attr);
        return NULL;
     }

   free(reply_attr);

   cookie_geom = xcb_get_geometry_unchecked(_ecore_xcb_conn, window);
   reply_geom = xcb_get_geometry_reply(_ecore_xcb_conn, cookie_geom, NULL);
   if (!reply_geom) return NULL;

   if (!(s = calloc(1, sizeof(Shadow))))
     {
        free(reply_geom);
        return NULL;
     }

   s->win = window;
   s->x = reply_geom->x;
   s->y = reply_geom->y;
   s->w = reply_geom->width;
   s->h = reply_geom->height;

   free(reply_geom);

   cookie_tree = xcb_query_tree_unchecked(_ecore_xcb_conn, window);
   reply_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_tree, NULL);
   if (reply_tree)
     {
        xcb_window_t *list;
        int num;

        num = xcb_query_tree_children_length(reply_tree);
        list = xcb_query_tree_children(reply_tree);

        s->children = calloc(1, sizeof(Shadow *) * num);
        if (s->children)
          {
             s->children_num = num;
             for (i = 0; i < num; i++)
               {
                  s->children[i] = _ecore_x_window_tree_walk(list[i]);
                  if (s->children[i])
                    s->children[i]->parent = s;
               }
             /* compress list down */
             j = 0;
             for (i = 0; i < num; i++)
               {
                  if (s->children[i])
                    {
                       s->children[j] = s->children[i];
                       j++;
                    }
               }
             if (j == 0)
               {
                  free(s->children);
                  s->children = NULL;
                  s->children_num = 0;
               }
             else
               {
                  s->children_num = j;
                  sl = realloc(s->children, sizeof(Shadow *) * j);
                  if (sl) s->children = sl;
               }
          }

        free(reply_tree);
     }

   return s;
}

static void
_ecore_x_window_tree_shadow_free1(Shadow *s)
{
   int i = 0;

   if (!s) return;
   if (s->children)
     {
        for (i = 0; i < s->children_num; i++)
          {
             if (s->children[i])
               _ecore_x_window_tree_shadow_free1(s->children[i]);
          }
        free(s->children);
     }

   free(s);
}

static void
_ecore_x_window_tree_shadow_free(void)
{
   int i = 0;

   if (!shadow_base) return;

   for (i = 0; i < shadow_num; i++)
     {
        if (!shadow_base[i]) continue;
        _ecore_x_window_tree_shadow_free1(shadow_base[i]);
     }
   free(shadow_base);
   shadow_base = NULL;
   shadow_num = 0;
}

static void
_ecore_x_window_tree_shadow_populate(void)
{
   Ecore_X_Window *roots = NULL;
   int i = 0, num = 0;

   if ((roots = ecore_x_window_root_list(&num)))
     {
        shadow_base = calloc(1, sizeof(Shadow *) * num);
        if (shadow_base)
          {
             shadow_num = num;
             for (i = 0; i < num; i++)
               shadow_base[i] = _ecore_x_window_tree_walk(roots[i]);
          }

        free(roots);
     }
}

/*
   static void
   _ecore_x_window_tree_shadow_start(void)
   {
   shadow_count++;
   if (shadow_count > 1) return;
   _ecore_x_window_tree_shadow_populate();
   }

   static void
   _ecore_x_window_tree_shadow_stop(void)
   {
   shadow_count--;
   if (shadow_count != 0) return;
   _ecore_x_window_tree_shadow_free();
   }
 */

Shadow *
_ecore_x_window_shadow_tree_find_shadow(Shadow        *s,
                                        Ecore_X_Window win)
{
   Shadow *ss;
   int i = 0;

   if (s->win == win) return s;

   if (s->children)
     {
        for (i = 0; i < s->children_num; i++)
          {
             if (!s->children[i]) continue;

             if ((ss =
                    _ecore_x_window_shadow_tree_find_shadow(s->children[i], win)))
               return ss;
          }
     }

   return NULL;
}

Shadow *
_ecore_x_window_shadow_tree_find(Ecore_X_Window base)
{
   Shadow *s;
   int i = 0;

   for (i = 0; i < shadow_num; i++)
     {
        if (!shadow_base[i]) continue;

        if ((s =
               _ecore_x_window_shadow_tree_find_shadow(shadow_base[i], base)))
          return s;
     }
   return NULL;
}

static Ecore_X_Window
_ecore_x_window_shadow_tree_at_xy_get_shadow(Shadow         *s,
                                             int             bx,
                                             int             by,
                                             int             x,
                                             int             y,
                                             Ecore_X_Window *skip,
                                             int             skip_num)
{
   Ecore_X_Window child;
   Ecore_X_Rectangle *rects;
   int i = 0, j = 0, wx = 0, wy = 0, num = 0;

   wx = s->x + bx;
   wy = s->y + by;
   if (!((x >= wx) && (y >= wy) && (x < (wx + s->w)) && (y < (wy + s->h))))
     return 0;

   rects = ecore_x_window_shape_rectangles_get(s->win, &num);
   if (!_inside_rects(s, x, y, bx, by, rects, num)) return 0;
   num = 0;
   rects = ecore_x_window_shape_input_rectangles_get(s->win, &num);
   if (!_inside_rects(s, x, y, bx, by, rects, num)) return 0;

   if (s->children)
     {
        int skipit = 0;

        for (i = s->children_num - 1; i >= 0; --i)
          {
             if (!s->children[i]) continue;

             skipit = 0;
             if (skip)
               {
                  for (j = 0; j < skip_num; j++)
                    {
                       if (s->children[i]->win == skip[j])
                         {
                            skipit = 1;
                            goto onward;
                         }
                    }
               }
onward:
             if (!skipit)
               {
                  if ((child =
                         _ecore_x_window_shadow_tree_at_xy_get_shadow(s->children[i], wx, wy, x, y, skip, skip_num)))
                    return child;
               }
          }
     }

   return s->win;
}

static Ecore_X_Window
_ecore_x_window_shadow_tree_at_xy_get(Ecore_X_Window  base,
                                      int             bx,
                                      int             by,
                                      int             x,
                                      int             y,
                                      Ecore_X_Window *skip,
                                      int             skip_num)
{
   Shadow *s;

   if (!shadow_base)
     {
        _ecore_x_window_tree_shadow_populate();
        if (!shadow_base) return 0;
     }

   s = _ecore_x_window_shadow_tree_find(base);
   if (!s) return 0;

   return _ecore_x_window_shadow_tree_at_xy_get_shadow(s, bx, by, x, y, skip, skip_num);
}

static Eina_Bool
_inside_rects(Shadow            *s,
              int                x,
              int                y,
              int                bx,
              int                by,
              Ecore_X_Rectangle *rects,
              int                num)
{
   Eina_Bool inside = EINA_FALSE;
   int i = 0;

   if (!rects) return EINA_FALSE;
   for (i = 0; i < num; i++)
     {
        if ((x >= s->x + bx + rects[i].x) &&
            (y >= s->y + by + rects[i].y) &&
            (x < (int)(s->x + bx + rects[i].x + rects[i].width)) &&
            (y < (int)(s->y + by + rects[i].y + rects[i].height)))
          {
             inside = EINA_TRUE;
             break;
          }
     }
   free(rects);
   return inside;
}

/**
 * Retrieves the top, visible window at the given location,
 * but skips the windows in the list. This uses a shadow tree built from the
 * window tree that is only updated the first time
 * ecore_x_window_shadow_tree_at_xy_with_skip_get() is called, or the next time
 * it is called after a  ecore_x_window_shadow_tree_flush()
 * @param   base The base window to start searching from (normally root).
 * @param   x The given X position.
 * @param   y The given Y position.
 * @return  The window at that position.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI Ecore_X_Window
ecore_x_window_shadow_tree_at_xy_with_skip_get(Ecore_X_Window  base,
                                               int             x,
                                               int             y,
                                               Ecore_X_Window *skip,
                                               int             skip_num)
{
   return _ecore_x_window_shadow_tree_at_xy_get(base, 0, 0, x, y, skip, skip_num);
}

/**
 * Retrieves the parent window a given window has. This uses the shadow window
 * tree.
 * @param   root The root window of @p win - if 0, this will be automatically determined with extra processing overhead
 * @param   win The window to get the parent window of
 * @return  The parent window of @p win
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI Ecore_X_Window
ecore_x_window_shadow_parent_get(Ecore_X_Window root __UNUSED__,
                                 Ecore_X_Window win)
{
   Shadow *s;
   int i = 0;

   if (!shadow_base)
     {
        _ecore_x_window_tree_shadow_populate();
        if (!shadow_base) return 0;
     }

   for (i = 0; i < shadow_num; i++)
     {
        if (!shadow_base[i]) continue;

        s = _ecore_x_window_shadow_tree_find_shadow(shadow_base[i], win);
        if (s)
          {
             if (!s->parent) return 0;
             return s->parent->win;
          }
     }
   return 0;
}

/**
 * Flushes the window shadow tree so nothing is stored.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void
ecore_x_window_shadow_tree_flush(void)
{
   _ecore_x_window_tree_shadow_free();
}

