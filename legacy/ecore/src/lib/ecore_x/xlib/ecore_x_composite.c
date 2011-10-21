#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "ecore_x_private.h"
#include "Ecore_X.h"

static Eina_Bool _composite_available = EINA_FALSE;

void
_ecore_x_composite_init(void)
{
   _composite_available = EINA_FALSE;

#ifdef ECORE_XCOMPOSITE
   int major, minor;

   if (XCompositeQueryVersion(_ecore_x_disp, &major, &minor))
     {
# ifdef ECORE_XRENDER
        if (XRenderQueryExtension(_ecore_x_disp, &major, &minor))
          {
#  ifdef ECORE_XFIXES
             if (XFixesQueryVersion(_ecore_x_disp, &major, &minor))
               {
                  _composite_available = EINA_TRUE;
               }
#  endif
          }
# endif
     }
#endif
} /* _ecore_x_composite_init */

EAPI Eina_Bool
ecore_x_composite_query(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return _composite_available;
} /* ecore_x_composite_query */

EAPI void
ecore_x_composite_redirect_window(Ecore_X_Window                win,
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
     } /* switch */
   XCompositeRedirectWindow(_ecore_x_disp, win, update);
#endif /* ifdef ECORE_XCOMPOSITE */
} /* ecore_x_composite_redirect_window */

EAPI void
ecore_x_composite_redirect_subwindows(Ecore_X_Window                win,
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
     } /* switch */
   XCompositeRedirectSubwindows(_ecore_x_disp, win, update);
#endif /* ifdef ECORE_XCOMPOSITE */
} /* ecore_x_composite_redirect_subwindows */

EAPI void
ecore_x_composite_unredirect_window(Ecore_X_Window                win,
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
     } /* switch */
   XCompositeUnredirectWindow(_ecore_x_disp, win, update);
#endif /* ifdef ECORE_XCOMPOSITE */
} /* ecore_x_composite_unredirect_window */

EAPI void
ecore_x_composite_unredirect_subwindows(Ecore_X_Window                win,
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
     } /* switch */
   XCompositeUnredirectSubwindows(_ecore_x_disp, win, update);
#endif /* ifdef ECORE_XCOMPOSITE */
} /* ecore_x_composite_unredirect_subwindows */

EAPI Ecore_X_Pixmap
ecore_x_composite_name_window_pixmap_get(Ecore_X_Window win)
{
   Ecore_X_Pixmap pixmap = None;
#ifdef ECORE_XCOMPOSITE
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   pixmap = XCompositeNameWindowPixmap(_ecore_x_disp, win);
#endif /* ifdef ECORE_XCOMPOSITE */
   return pixmap;
} /* ecore_x_composite_name_window_pixmap_get */

EAPI void
ecore_x_composite_window_events_disable(Ecore_X_Window win)
{
#ifdef ECORE_XCOMPOSITE
   ecore_x_window_shape_input_rectangle_set(win, -1, -1, 1, 1);
#endif /* ifdef ECORE_XCOMPOSITE */
}

EAPI void
ecore_x_composite_window_events_enable(Ecore_X_Window win)
{
#ifdef ECORE_XCOMPOSITE
   ecore_x_window_shape_input_rectangle_set(win, 0, 0, 65535, 65535);
#endif /* ifdef ECORE_XCOMPOSITE */
}

EAPI Ecore_X_Window
ecore_x_composite_render_window_enable(Ecore_X_Window root)
{
   Ecore_X_Window win = 0;
#ifdef ECORE_XCOMPOSITE
   win = XCompositeGetOverlayWindow(_ecore_x_disp, root);
   ecore_x_composite_window_events_disable(win);
#endif /* ifdef ECORE_XCOMPOSITE */
   return win;
} /* ecore_x_composite_render_window_enable */

EAPI void
ecore_x_composite_render_window_disable(Ecore_X_Window root)
{
#ifdef ECORE_XCOMPOSITE
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XCompositeReleaseOverlayWindow(_ecore_x_disp, root);
#endif /* ifdef ECORE_XCOMPOSITE */
} /* ecore_x_composite_render_window_disable */

