#ifndef _ECORE_SDL_PRIVATE_H
# define _ECORE_SDL_PRIVATE_H

extern int _ecore_sdl_log_dom;

# ifdef ECORE_SDL_DEFAULT_LOG_COLOR
#  undef ECORE_SDL_DEFAULT_LOG_COLOR
# endif
# define ECORE_SDL_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_sdl_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_sdl_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_sdl_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_sdl_log_dom, __VA_ARGS__)

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) EINA_LOG_DOM_CRIT(_ecore_sdl_log_dom, __VA_ARGS__)

#endif
