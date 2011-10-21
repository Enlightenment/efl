#include "ecore_xcb_private.h"
//#include "Ecore_X_Atoms.h"

#define ECORE_X_MWM_HINTS_FUNCTIONS   (1 << 0)
#define ECORE_X_MWM_HINTS_DECORATIONS (1 << 1)
#define ECORE_X_MWM_HINTS_INPUT_MODE  (1 << 2)
#define ECORE_X_MWM_HINTS_STATUS      (1 << 3)

typedef struct _mwmhints
{
   uint32_t flags;
   uint32_t functions;
   uint32_t decorations;
   int32_t  inputmode;
   uint32_t status;
} MWMHints;

/**
 * @defgroup Ecore_X_MWM_Group MWM related functions.
 *
 * Functions related to MWM.
 */

/**
 * Sets the borderless flag of a window using MWM.
 *
 * @param win The window.
 * @param borderless The borderless flag.
 *
 * @ingroup Ecore_X_MWM_Group
 */
EAPI void
ecore_x_mwm_borderless_set(Ecore_X_Window win,
                           Eina_Bool      borderless)
{
   uint32_t data[5] = { 0, 0, 0, 0, 0 };

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   data[0] = 2;
   data[2] = !borderless;

   ecore_x_window_prop_property_set(win,
                                    ECORE_X_ATOM_MOTIF_WM_HINTS,
                                    ECORE_X_ATOM_MOTIF_WM_HINTS, 32,
                                    (void *)data, 5);
}

EAPI Eina_Bool
ecore_x_mwm_hints_get(Ecore_X_Window          win,
                      Ecore_X_MWM_Hint_Func  *fhint,
                      Ecore_X_MWM_Hint_Decor *dhint,
                      Ecore_X_MWM_Hint_Input *ihint)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;
   MWMHints *mwmhints = NULL;
   int ret = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   cookie =
     xcb_get_property_unchecked(_ecore_xcb_conn, 0, win,
                                ECORE_X_ATOM_MOTIF_WM_HINTS,
                                ECORE_X_ATOM_MOTIF_WM_HINTS, 0, LONG_MAX);
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;
   if ((reply->format != 32) || (reply->value_len == 0))
     {
        free(reply);
        return EINA_FALSE;
     }

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
        ret = EINA_TRUE;
     }
   free(reply);
   return ret;
}

