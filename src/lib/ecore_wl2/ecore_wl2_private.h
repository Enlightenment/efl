#ifndef _ECORE_WL2_PRIVATE_H
# define _ECORE_WL2_PRIVATE_H

# include "Ecore_Wl2.h"
# include "Ecore_Input.h"

extern int _ecore_wl2_log_dom;

# ifdef ECORE_WL2_DEFAULT_LOG_COLOR
#  undef ECORE_WL2_DEFAULT_LOG_COLOR
# endif
# define ECORE_WL2_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) EINA_LOG_DOM_CRIT(_ecore_wl2_log_dom, __VA_ARGS__)

struct _Ecore_Wl2_Display
{
   const char *name;

   struct
     {
        struct wl_display *display;
        struct wl_registry *registry;
     } wl;

   Ecore_Fd_Handler *fd_hdl;
};

#endif
