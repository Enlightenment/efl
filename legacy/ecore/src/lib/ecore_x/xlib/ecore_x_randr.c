#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "ecore_x_private.h"
#include "ecore_x_randr.h"

static Eina_Bool _randr_available = EINA_FALSE;
#ifdef ECORE_XRANDR
static int _randr_major, _randr_minor;
int _randr_version;
#define RANDR_1_1           ((1 << 16) | 1)
#define RANDR_1_2           ((1 << 16) | 2)
#define RANDR_1_3           ((1 << 16) | 3)

#define RANDR_VALIDATE_ROOT(screen,                                  \
                            root) ((screen =                         \
                                      XRRRootToScreen(_ecore_x_disp, \
                                                      root)) != -1)

#define Ecore_X_Randr_Unset -1

XRRScreenResources *(*_ecore_x_randr_get_screen_resources)(Display * dpy,
                                                           Window window);

#endif /* ifdef ECORE_XRANDR */

void
_ecore_x_randr_init(void)
{
#ifdef ECORE_XRANDR
   _randr_major = 1;
   _randr_minor = 3;
   _randr_version = 0;

   _ecore_x_randr_get_screen_resources = NULL;
   if (XRRQueryVersion(_ecore_x_disp, &_randr_major, &_randr_minor))
     {
        _randr_version = (_randr_major << 16) | _randr_minor;
        if (_randr_version >= RANDR_1_3)
          _ecore_x_randr_get_screen_resources = XRRGetScreenResourcesCurrent;
        else if (_randr_version == RANDR_1_2)
          _ecore_x_randr_get_screen_resources = XRRGetScreenResources;

        _randr_available = EINA_TRUE;
     }
   else
     _randr_available = EINA_FALSE;

#else
   _randr_available = EINA_FALSE;
#endif
}

/*
 * @brief query whether randr is available or not
 * @return EINA_TRUE, if extension is available, else EINA_FALSE
 */
EAPI Eina_Bool
ecore_x_randr_query(void)
{
   return _randr_available;
}

/*
 * @return version of the RandRR extension supported by the server or,
 * in case RandRR extension is not available, Ecore_X_Randr_Unset (=-1).
 * bit version information: 31   MAJOR   16 | 15   MINOR   0
 */
EAPI int
ecore_x_randr_version_get(void)
{
#ifdef ECORE_XRANDR
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (_randr_available)
     {
        return _randr_version;
     }
   else
     {
        return Ecore_X_Randr_Unset;
     }
#else
   return -1;
#endif
}

Eina_Bool
_ecore_x_randr_root_validate(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   Ecore_X_Randr_Screen scr = -1;
   if (root && RANDR_VALIDATE_ROOT(scr, root))
     return EINA_TRUE;
   else
     return EINA_FALSE;

#else
   return EINA_FALSE;
#endif
}

