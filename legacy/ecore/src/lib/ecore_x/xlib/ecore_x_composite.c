/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "ecore_x_private.h"
#include "Ecore_X.h"

static int _composite_available;

void
_ecore_x_composite_init(void)
{
   _composite_available = 0;

#ifdef ECORE_XCOMPOSITE
   int major, minor;

   if (XCompositeQueryVersion(_ecore_x_disp, &major, &minor))
      _composite_available = 1;

#endif
}

EAPI int
ecore_x_composite_query(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return _composite_available;
}

EAPI void
ecore_x_composite_redirect_window(Ecore_X_Window win,
                                  Ecore_X_Composite_Update_Type type)
{
#ifdef ECORE_XCOMPOSITE
   int update = CompositeRedirectAutomatic;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   switch(type)
     {
      case ECORE_X_COMPOSITE_UPDATE_AUTOMATIC:
         update = CompositeRedirectAutomatic;
         break;

      case ECORE_X_COMPOSITE_UPDATE_MANUAL:
         update = CompositeRedirectManual;
         break;
     }
   XCompositeRedirectWindow(_ecore_x_disp, win, update);
#endif
}

EAPI void
ecore_x_composite_redirect_subwindows(Ecore_X_Window win,
                                      Ecore_X_Composite_Update_Type type)
{
#ifdef ECORE_XCOMPOSITE
   int update = CompositeRedirectAutomatic;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   switch(type)
     {
      case ECORE_X_COMPOSITE_UPDATE_AUTOMATIC:
         update = CompositeRedirectAutomatic;
         break;

      case ECORE_X_COMPOSITE_UPDATE_MANUAL:
         update = CompositeRedirectManual;
         break;
     }
   XCompositeRedirectSubwindows(_ecore_x_disp, win, update);
#endif
}

EAPI void
ecore_x_composite_unredirect_window(Ecore_X_Window win,
                                    Ecore_X_Composite_Update_Type type)
{
#ifdef ECORE_XCOMPOSITE
   int update = CompositeRedirectAutomatic;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   switch(type)
     {
      case ECORE_X_COMPOSITE_UPDATE_AUTOMATIC:
         update = CompositeRedirectAutomatic;
         break;

      case ECORE_X_COMPOSITE_UPDATE_MANUAL:
         update = CompositeRedirectManual;
         break;
     }
   XCompositeUnredirectWindow(_ecore_x_disp, win, update);
#endif
}

EAPI void
ecore_x_composite_unredirect_subwindows(Ecore_X_Window win,
                                        Ecore_X_Composite_Update_Type type)
{
#ifdef ECORE_XCOMPOSITE
   int update = CompositeRedirectAutomatic;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   switch(type)
     {
      case ECORE_X_COMPOSITE_UPDATE_AUTOMATIC:
         update = CompositeRedirectAutomatic;
         break;

      case ECORE_X_COMPOSITE_UPDATE_MANUAL:
         update = CompositeRedirectManual;
         break;
     }
   XCompositeUnredirectSubwindows(_ecore_x_disp, win, update);
#endif
}

EAPI Ecore_X_Pixmap
ecore_x_composite_name_window_pixmap_get(Ecore_X_Window win)
{
   Ecore_X_Pixmap pixmap = None;

#ifdef ECORE_XCOMPOSITE
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   pixmap = XCompositeNameWindowPixmap(_ecore_x_disp, win);
#endif

   return pixmap;
}

EAPI Ecore_X_Window
ecore_x_composite_render_window_enable(Ecore_X_Window root)
{
   Ecore_X_Window win = 0;
#ifdef ECORE_XCOMPOSITE
   XRectangle rect;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   win = XCompositeGetOverlayWindow(_ecore_x_disp, root);
   rect.x = -1;
   rect.y = -1;
   rect.width = 1;
   rect.height = 1;
   XShapeCombineRectangles(_ecore_x_disp, win, ShapeInput, 0, 0, &rect, 1,
                           ShapeSet, Unsorted);
#endif
   return win;
}

EAPI void
ecore_x_composite_render_window_disable(Ecore_X_Window root)
{
#ifdef ECORE_XCOMPOSITE
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XCompositeReleaseOverlayWindow(_ecore_x_disp, root);
#endif
}
