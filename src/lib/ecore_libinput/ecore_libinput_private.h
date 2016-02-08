#ifndef _ECORE_LIBINPUT_PRIVATE_H
# define _ECORE_LIBINPUT_PRIVATE_H

# include "Ecore.h"
# include "ecore_private.h"
# include "Ecore_Input.h"
# include "Eeze.h"

# ifndef ECORE_LIBINPUT_COLOR_DEFAULT
#  define ECORE_LIBINPUT_COLOR_DEFAULT EINA_COLOR_CYAN
# endif

extern int _ecore_libinput_log_dom;

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_libinput_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_libinput_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_libinput_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_libinput_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_libinput_log_dom, __VA_ARGS__)

#endif
