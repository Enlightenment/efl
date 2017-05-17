#ifndef _ECORE_EVAS_WAYLAND_PRIVATE_H_
# define _ECORE_EVAS_WAYLAND_PRIVATE_H_

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# define ECORE_EVAS_INTERNAL

# ifndef ELEMENTARY_H
//#define LOGFNS 1
#  ifdef LOGFNS
#   include <stdio.h>
#   define LOGFN(fl, ln, fn) \
   printf("-ECORE_EVAS-WL: %25s: %5i - %s\n", fl, ln, fn);
#  else
#   define LOGFN(fl, ln, fn)
#  endif

#  include <Eina.h>
#  include <Ecore.h>
#  include <Ecore_Input.h>
#  include <Ecore_Input_Evas.h>
#  include <Ecore_Wl2.h>

#  include <Ecore_Evas.h>
# endif

# include "ecore_wl2_private.h"
# include "ecore_private.h"
# include "ecore_evas_private.h"
# include "ecore_evas_wayland.h"

typedef struct _Ecore_Evas_Engine_Wl_Data Ecore_Evas_Engine_Wl_Data;

struct _Ecore_Evas_Engine_Wl_Data 
{
   Ecore_Wl2_Display *display;
   Eina_List *regen_objs;
   Ecore_Wl2_Window *parent, *win;
   Ecore_Event_Handler *sync_handler;
   int fx, fy, fw, fh;
   Eina_Rectangle content;
   struct wl_callback *anim_callback;
   int x_rel;
   int y_rel;
   uint32_t timestamp;
   Eina_List *devices_list;
   Eina_Bool dragging : 1;
   Eina_Bool sync_done : 1;
   Eina_Bool defer_show : 1;
   Eina_Bool reset_pending : 1;
   Eina_Bool activated : 1;
};

Ecore_Evas *_ecore_evas_wl_common_new_internal(const char *disp_name, unsigned int parent, int x, int y, int w, int h, Eina_Bool frame, const char *engine_name);

#endif /* _ECORE_EVAS_WAYLAND_PRIVATE_H_ */
