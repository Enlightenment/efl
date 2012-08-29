#ifndef _ECORE_WAYLAND_PRIVATE_H
# define _ECORE_WAYLAND_PRIVATE_H

# include <limits.h>
# include <unistd.h>

# include "Ecore.h"
# include "Ecore_Input.h"
# include "Ecore_Wayland.h"

//# define LOGFNS 1

# ifdef LOGFNS
#  include <stdio.h>
#  define LOGFN(fl, ln, fn) printf("-ECORE-WL: %25s: %5i - %s\n", fl, ln, fn);
# else
#  define LOGFN(fl, ln, fn)
# endif

extern int _ecore_wl_log_dom;
extern Ecore_Wl_Display *_ecore_wl_disp;

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

struct _Ecore_Wl_Dnd
{
   Ecore_Wl_Display *ewd;
   Ecore_Wl_Input *input;

   /* As provider */
   struct wl_data_source *data_source;
   struct wl_array types_offered;

   /* TODO: dnd specific fields */
};

struct _Ecore_Wl_Dnd_Source
{
   struct wl_data_offer *offer;
   Ecore_Wl_Input *input;
   struct wl_array types;
   int refcount;
   int fd;
   int x, y;

   /* TODO: task & data_func */
   void *data;
};

struct _Ecore_Wl_Dnd_Target
{
   Ecore_Wl_Dnd_Source *source;
};

void _ecore_wl_window_init(void);
void _ecore_wl_window_shutdown(void);

void _ecore_wl_output_add(Ecore_Wl_Display *ewd, unsigned int id);
void _ecore_wl_output_del(Ecore_Wl_Output *output);

void _ecore_wl_input_add(Ecore_Wl_Display *ewd, unsigned int id);
void _ecore_wl_input_del(Ecore_Wl_Input *input);
void _ecore_wl_input_pointer_xy_get(int *x, int *y);
void _ecore_wl_input_set_selection(Ecore_Wl_Input *input, struct wl_data_source *source);

void _ecore_wl_dnd_add(Ecore_Wl_Input *input, struct wl_data_device *data_device, struct wl_data_offer *offer);
void _ecore_wl_dnd_enter(void *data, struct wl_data_device *data_device __UNUSED__, unsigned int timestamp __UNUSED__, struct wl_surface *surface, int x, int y, struct wl_data_offer *offer);
void _ecore_wl_dnd_leave(void *data, struct wl_data_device *data_device __UNUSED__);
void _ecore_wl_dnd_motion(void *data, struct wl_data_device *data_device __UNUSED__, unsigned int timestamp __UNUSED__, int x, int y);
void _ecore_wl_dnd_drop(void *data, struct wl_data_device *data_device __UNUSED__);
void _ecore_wl_dnd_selection(void *data, struct wl_data_device *data_device __UNUSED__, struct wl_data_offer *offer);
void _ecore_wl_dnd_del(Ecore_Wl_Dnd_Source *source);

struct wl_data_source *_ecore_wl_create_data_source(Ecore_Wl_Display *ewd);
#endif
