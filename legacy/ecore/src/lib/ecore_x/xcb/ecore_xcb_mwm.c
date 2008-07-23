/*
 * Various MWM related functions.
 * 
 * This is ALL the code involving anything MWM related. for both WM and
 * client.
 */

#include "ecore_xcb_private.h"
#include "Ecore_X_Atoms.h"


/**
 * @defgroup Ecore_X_MWM_Group MWM related functions.
 *
 * Functions related to MWM.
 */

#define ECORE_X_MWM_HINTS_FUNCTIONS           (1 << 0)
#define ECORE_X_MWM_HINTS_DECORATIONS         (1 << 1)
#define ECORE_X_MWM_HINTS_INPUT_MODE          (1 << 2)
#define ECORE_X_MWM_HINTS_STATUS              (1 << 3)

typedef struct _mwmhints
{
   uint32_t flags;
   uint32_t functions;
   uint32_t decorations;
   int32_t  inputmode;
   uint32_t status;
}
MWMHints;


/**
 * Sends the GetProperty request.
 * @param window Window whose MWM hints are requested.
 * @ingroup Ecore_X_MWM_Group
 */
EAPI void
ecore_x_mwm_hints_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window ? window : ((xcb_screen_t *)_ecore_xcb_screen)->root,
                                       ECORE_X_ATOM_MOTIF_WM_HINTS,
                                       ECORE_X_ATOM_MOTIF_WM_HINTS,
                                       0, LONG_MAX);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetProperty request sent by ecore_x_mwm_hints_get_prefetch().
 * @ingroup Ecore_X_MWM_Group
 */
EAPI void
ecore_x_mwm_hints_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * To document.
 * @param  window Unused.
 * @param  fhint To document.
 * @param  dhint To document.
 * @param  ihint To document.
 * @return       1 on success, 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_mwm_hints_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_mwm_hints_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_MWM_Group
 */
EAPI int
ecore_x_mwm_hints_get(Ecore_X_Window          window __UNUSED__,
		      Ecore_X_MWM_Hint_Func  *fhint,
		      Ecore_X_MWM_Hint_Decor *dhint,
		      Ecore_X_MWM_Hint_Input *ihint)
{
   MWMHints                 *mwmhints = NULL;
   int                       ret = 0;
   xcb_get_property_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return 0;

   if ((reply->format != 32) ||
       (reply->value_len == 0))
      return 0;

   mwmhints = xcb_get_property_value(reply);
   if (reply->value_len >= 4)
     {
        if (dhint)
          {
             if (mwmhints->flags & ECORE_X_MWM_HINTS_DECORATIONS)
               *dhint = mwmhints->decorations;
             else
               *dhint = ECORE_X_MWM_HINT_DECOR_ALL;
          }
        if (fhint)
          {
             if (mwmhints->flags & ECORE_X_MWM_HINTS_FUNCTIONS)
               *fhint = mwmhints->functions;
             else
               *fhint = ECORE_X_MWM_HINT_FUNC_ALL;
          }
        if (ihint)
          {
             if (mwmhints->flags & ECORE_X_MWM_HINTS_INPUT_MODE)
               *ihint = mwmhints->inputmode;
             else
               *ihint = ECORE_X_MWM_HINT_INPUT_MODELESS;
          }
        ret = 1;
     }

   return ret;
}

/**
 * Sets the borderless flag of a window using MWM.
 * @param window     The window.
 * @param borderless The borderless flag.
 * @ingroup Ecore_X_MWM_Group
 */
EAPI void
ecore_x_mwm_borderless_set(Ecore_X_Window window,
                           int            borderless)
{
   uint32_t data[5] = {0, 0, 0, 0, 0};

   data[0] = 2; /* just set the decorations hint! */
   data[2] = !borderless;
   
   if (window == 0) window = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       ECORE_X_ATOM_MOTIF_WM_HINTS, ECORE_X_ATOM_MOTIF_WM_HINTS,
                       32, 5, data);
}

