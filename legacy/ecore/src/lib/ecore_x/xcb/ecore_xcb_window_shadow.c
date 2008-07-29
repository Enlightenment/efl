/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/* #include "Ecore.h" */
#include "ecore_xcb_private.h"
#include "Ecore_X.h"


typedef struct _Shadow Shadow;
struct _Shadow
{
   Shadow         *parent;
   Shadow        **children;
   Ecore_X_Window  win;
   int             children_num;
   short           x, y;
   unsigned short  w, h;
};

static int shadow_count = 0;
static Shadow **shadow_base = NULL;
static int shadow_num = 0;


/* FIXME: round trips */
static Shadow *
_ecore_x_window_tree_walk(Ecore_X_Window window)
{
   Shadow                            *s;
   Shadow                           **sl;
   xcb_get_window_attributes_reply_t *reply_attr;
   xcb_get_geometry_reply_t          *reply_geom;
   xcb_query_tree_reply_t            *reply_tree;
   xcb_get_window_attributes_cookie_t cookie_attr;
   xcb_get_geometry_cookie_t          cookie_geom;
   xcb_query_tree_cookie_t            cookie_tree;
   int                                i;
   int                                j;

   cookie_attr = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, window);
   cookie_geom = xcb_get_geometry_unchecked(_ecore_xcb_conn, window);
   cookie_tree = xcb_query_tree_unchecked(_ecore_xcb_conn, window);

   reply_attr = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie_attr, NULL);
   if (!reply_attr)
     {
        reply_geom = xcb_get_geometry_reply(_ecore_xcb_conn, cookie_geom, NULL);
        if (reply_geom) free(reply_geom);
        reply_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_tree, NULL);
        if (reply_tree) free(reply_tree);
        return NULL;
     }

   if (reply_attr->map_state != XCB_MAP_STATE_VIEWABLE)
     {
        reply_geom = xcb_get_geometry_reply(_ecore_xcb_conn, cookie_geom, NULL);
        if (reply_geom) free(reply_geom);
        reply_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_tree, NULL);
        if (reply_tree) free(reply_tree);
        return NULL;
     }

   free(reply_attr);

   s = calloc(1, sizeof(Shadow));
   if (!s) return NULL;

   reply_geom = xcb_get_geometry_reply(_ecore_xcb_conn, cookie_geom, NULL);
   if (!reply_geom)
     {
        reply_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_tree, NULL);
        if (reply_tree) free(reply_tree);
        return NULL;
     }

   s->win = window;
   s->x = reply_geom->x;
   s->y = reply_geom->y;
   s->w = reply_geom->width;
   s->h = reply_geom->height;

   free(reply_geom);

   reply_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_tree, NULL);
   if (reply_tree)
/*    if (XQueryTree(_ecore_xcb_conn, s->win, &root_win, &parent_win, */
/* 		   &list, &num)) */
     {
        xcb_window_t *list;
        int           num;

        num = xcb_query_tree_children_length(reply_tree);
        list = xcb_query_tree_children(reply_tree);

	s->children = calloc(1, sizeof(Shadow *) * num);
	if (s->children)
	  {
	     s->children_num = num;
	     for (i = 0; i < num; i++)
	       {
		  s->children[i] = _ecore_x_window_tree_walk(list[i]);
		  if (s->children[i]) s->children[i]->parent = s;
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
   int i;

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
   int i;

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
   Ecore_X_Window   *roots;
   int               i, num;

   roots = ecore_x_window_root_list(&num);
   if (roots)
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

Shadow *
_ecore_x_window_shadow_tree_find_shadow(Shadow *s, Ecore_X_Window win)
{
   Shadow *ss;
   int i;

   if (s->win == win) return s;
   if (s->children)
     {
	for (i = 0; i < s->children_num; i++)
	  {
	     if (!s->children[i]) continue;
	     if ((ss = _ecore_x_window_shadow_tree_find_shadow(s->children[i], win)))
	       return ss;
	  }
     }
   return NULL;
}

Shadow *
_ecore_x_window_shadow_tree_find(Ecore_X_Window base)
{
   Shadow *s;
   int i;

   for (i = 0; i < shadow_num; i++)
     {
	if (!shadow_base[i]) continue;
	if ((s = _ecore_x_window_shadow_tree_find_shadow(shadow_base[i], base)))
	  return s;
     }
   return NULL;
}

static Ecore_X_Window
_ecore_x_window_shadow_tree_at_xy_get_shadow(Shadow *s, int bx, int by, int x, int y,
					     Ecore_X_Window *skip, int skip_num)
{
   Ecore_X_Window child;
   int i, j;
   int wx, wy;

   wx = s->x + bx;
   wy = s->y + by;
   if (!((x >= wx) && (y >= wy) && (x < (wx + s->w)) && (y < (wy + s->h))))
     return 0;
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
		  if ((child = _ecore_x_window_shadow_tree_at_xy_get_shadow(s->children[i], wx, wy, x, y, skip, skip_num)))
		    {
		       return child;
		    }
	       }
	  }
     }
   return s->win;
}

static Ecore_X_Window
_ecore_x_window_shadow_tree_at_xy_get(Ecore_X_Window base, int bx, int by, int x, int y,
				      Ecore_X_Window *skip, int skip_num)
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
ecore_x_window_shadow_tree_at_xy_with_skip_get(Ecore_X_Window base, int x, int y, Ecore_X_Window *skip, int skip_num)
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
ecore_x_window_shadow_parent_get(Ecore_X_Window root, Ecore_X_Window win)
{
   Shadow *s;
   int     i;

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
