#ifndef _ECORE_WAYLAND_PRIVATE_H
# define _ECORE_WAYLAND_PRIVATE_H

# include <limits.h>

//# define LOGFNS 1

# ifdef LOGFNS
#  include <stdio.h>
#  define LOGFN(fl, ln, fn) printf("-ECORE-WL: %25s: %5i - %s\n", fl, ln, fn);
# else
#  define LOGFN(fl, ln, fn)
# endif

extern int _ecore_wl_log_dom;

# ifdef ECORE_WL_DEFAULT_LOG_COLOR
#  undef ECORE_WL_DEFAULT_LOG_COLOR
# endif
# define ECORE_WL_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_wl_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_wl_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_wl_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_wl_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_wl_log_dom, __VA_ARGS__)

typedef struct _Ecore_Wl_Dnd_Source
{
   struct wl_data_offer *offer;
   int refs;

   Eina_Array *types;

   uint32_t timestamp;
   void *data;
} Ecore_Wl_Dnd_Source;

typedef struct _Ecore_Wl_Dnd_Target
{
   /* NB: These are not the real fields for this structure, 
    * and it is Bound to change....soon */
   struct wl_data_offer *offer;
   int refs;

   Eina_Array *types;

   uint32_t timestamp;
   void *data;
} Ecore_Wl_Dnd_Target;

struct _Ecore_Wl_Drag_Source
{
   struct wl_data_device *data_dev;
   struct wl_buffer *buffer;

   int32_t hotspot_x, hotspot_y;
   int32_t offset_x, offset_y;
   const char *mimetype;
   uint32_t timestamp;
   void *data;

   struct wl_data_source *data_source;
};

#endif
