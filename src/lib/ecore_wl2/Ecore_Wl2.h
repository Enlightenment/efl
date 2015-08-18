#ifndef _ECORE_WL2_H_
# define _ECORE_WL2_H_

# include <Eina.h>
# include <Ecore.h>
# include <wayland-client.h>
# include <wayland-cursor.h>
# include <xkbcommon/xkbcommon.h>

# ifdef EAPI
#  undef EAPI
# endif

# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif

/* # ifdef __cplusplus */
/* extern "C" { */
/* # endif */

# ifndef _ECORE_WAYLAND_WINDOW_PREDEF
typedef struct _Ecore_Wl_Window Ecore_Wl_Window;
# endif

EAPI extern int ECORE_WL2_EVENT_GLOBAL_ADDED;
EAPI extern int ECORE_WL2_EVENT_GLOBAL_REMOVED;

EAPI int ecore_wl2_init(void);
EAPI int ecore_wl2_shutdown(void);

/* # ifdef __cplusplus */
/* } */
/* # endif */

# undef EAPI
# define EAPI

#endif
