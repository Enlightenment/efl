/*
 * vim:ts=8:sw=3:sts=8:expandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_x_private.h"
#include "ecore_x_randr.h"

#define Ecore_X_Randr_None  0
#define Ecore_X_Randr_Unset -1

#ifdef ECORE_XRANDR

#define RANDR_1_3           ((1 << 16) | 3)
#define RANDR_CHECK_1_3_RET(ret) if(_randr_version < RANDR_1_3) return ret

extern XRRScreenResources *(*_ecore_x_randr_get_screen_resources)(Display *
                                                                  dpy,
                                                                  Window
                                                                  window);
extern int _randr_version;
#endif

/*
 * @param root window which's screen should be queried
 * @return Ecore_X_Randr_Ouptut_Id or - if query failed or none is set - Ecore_X_Randr_None
 */
EAPI Ecore_X_Randr_Output
ecore_x_randr_primary_output_get(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_3_RET(Ecore_X_Randr_None);
   if (!_ecore_x_randr_root_validate(root))
     return Ecore_X_Randr_None;

   return XRRGetOutputPrimary(_ecore_x_disp, root);
#else
   return Ecore_X_Randr_None;
#endif
}

/*
 * @param root window which's screen should be queried
 * @param output that should be set as given root window's screen primary output
 */
EAPI void
ecore_x_randr_primary_output_set(Ecore_X_Window       root,
                                 Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_3_RET();

   if (_ecore_x_randr_output_validate(root, output))
     {
        XRRSetOutputPrimary(_ecore_x_disp, root, output);
     }

#endif
}

