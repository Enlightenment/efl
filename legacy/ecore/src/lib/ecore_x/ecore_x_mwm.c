/*
 * Various MWM related functions.
 * 
 * This is ALL the code involving anything MWM related. for both WM and
 * client.
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

#define ECORE_X_MWM_HINTS_FUNCTIONS           (1 << 0)
#define ECORE_X_MWM_HINTS_DECORATIONS         (1 << 1)
#define ECORE_X_MWM_HINTS_INPUT_MODE          (1 << 2)
#define ECORE_X_MWM_HINTS_STATUS              (1 << 3)

typedef struct _mwmhints
{
   CARD32              flags;
   CARD32              functions;
   CARD32              decorations;
   INT32               inputmode;
   CARD32              status;
}
MWMHints;

EAPI int
ecore_x_mwm_hints_get(Ecore_X_Window win,
		      Ecore_X_MWM_Hint_Func * fhint,
		      Ecore_X_MWM_Hint_Decor * dhint,
		      Ecore_X_MWM_Hint_Input * ihint)
{
   unsigned char      *p = NULL;
   MWMHints           *mwmhints = NULL;
   int                 num;
   int                 ret;

   ret = 0;
   if (!ecore_x_window_prop_property_get(win,
					 ECORE_X_ATOM_MOTIF_WM_HINTS,
					 ECORE_X_ATOM_MOTIF_WM_HINTS,
					 32, &p, &num))
      return 0;
   mwmhints = (MWMHints *) p;
   if (mwmhints)
     {
	if (num >= 4)
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
	free(mwmhints);
     }
   return ret;
}

EAPI void
ecore_x_mwm_borderless_set(Ecore_X_Window win, int borderless)
{
   unsigned int data[5] = {0, 0, 0, 0, 0};

   data[0] = 2; /* just set the decorations hint! */
   data[2] = !borderless;
   
   ecore_x_window_prop_property_set(win, 
				    ECORE_X_ATOM_MOTIF_WM_HINTS,
				    ECORE_X_ATOM_MOTIF_WM_HINTS,
				    32, (void *)data, 5);
}

