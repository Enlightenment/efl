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

/* # ifdef __cplusplus */
/* } */
/* # endif */

# undef EAPI
# define EAPI

#endif
