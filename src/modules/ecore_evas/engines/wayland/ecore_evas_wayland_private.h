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
#   define LOGFN \
   printf("-ECORE_EVAS-WL: %25s: %5i - %s\n", __FILE__, __LINE__, __func__)
#  else
#   define LOGFN
#  endif

#  include <Eina.h>
#  include <Ecore.h>
#  include <Ecore_Input.h>
#  include <Ecore_Input_Evas.h>
#  include <Ecore_Wl2.h>

#  include <Ecore_Evas.h>
# endif

# include "ecore_wl2_internal.h"
# include "ecore_wl2_private.h"
# include "ecore_private.h"
# include "ecore_evas_private.h"
# include "ecore_evas_wayland.h"

#ifdef _WIN32
# ifndef EFL_MODULE_STATIC
#  define EMODAPI __declspec(dllexport)
# else
#  define EMODAPI
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EMODAPI __attribute__ ((visibility("default")))
#  endif
# endif
#endif /* ! _WIN32 */

#ifndef EMODAPI
# define EMODAPI
#endif

typedef struct _Ecore_Evas_Engine_Wl_Data Ecore_Evas_Engine_Wl_Data;

typedef struct _Ecore_Evas_Wl_Selection_Data Ecore_Evas_Wl_Selection_Data;

struct _Ecore_Evas_Wl_Selection_Data
{
   Ecore_Evas_Selection_Callbacks callbacks;
   Eina_Promise *delivery;
   Ecore_Wl2_Offer *offer;
   const char *later_convert;
   uint32_t sent_serial; //The serial of the last sent selection op
};

struct _Ecore_Evas_Engine_Wl_Data
{
   Ecore_Wl2_Display *display;
   Eina_List *regen_objs;
   Ecore_Wl2_Window *parent, *win;
   Ecore_Event_Handler *sync_handler, *changed_handler, *end_handler, *send_handler, *offer_handler, *dnd_leave_handler, *dnd_motion_handler, *dnd_enter_handler, *dnd_drop_handler, *dnd_end_handler;
   int fx, fy, fw, fh;
   Ecore_Wl2_Frame_Cb_Handle *frame;
   int x_rel;
   int y_rel;
   uint32_t timestamp;
   Eina_List *devices_list;
   int cw, ch;
   Ecore_Evas_Wl_Selection_Data selection_data[ECORE_EVAS_SELECTION_BUFFER_LAST];
   Ecore_Wl2_Offer *external_offer;
   struct
     {
        Eina_Bool supported : 1;
        Eina_Bool prepare : 1;
        Eina_Bool request : 1;
        Eina_Bool done : 1;
        Eina_Bool configure_coming : 1;
        Ecore_Job *manual_mode_job;
     } wm_rot;

   Eina_Bool resizing : 1;
   Eina_Bool dragging : 1;
   Eina_Bool sync_done : 1;
   Eina_Bool defer_show : 1;
   Eina_Bool reset_pending : 1;
   Eina_Bool activated : 1;
   Eina_Bool ticking : 1;
};

/**
 * @brief Create an Ecore_Evas window using a Wayland-based engine.
 *
 * @param disp_name Name of the Wayland display to connect to.
 * @param parent ID of the parent window this Ecore_Evas window belongs
 *        to, or 0 if this is a top-level window.
 * @param x Horizontal position of the Ecore_Evas window.
 * @param y Vertical position of the Ecore_Evas window.
 * @param w Width of the Ecore_Evas window to be created.
 * @param h Height of the Ecore_Evas window to be created.
 * @param frame Deprecated.  (Just pass EINA_FALSE.)
 * @param opt engine option list
 * @param engine_name Wayland engine to use for rendering.
 * @return Ecore_Evas instance or @c NULL if creation failed.
 *
 * Like Evas' other window creation routines, this constructs an
 * Ecore_Evas window object using an engine name and various common
 * parameters.  However, this connects to the display using a Wayland
 * protocol for client-server communication rather than the X11
 * protocol.
 *
 * Also, Wayland uses a direct rendering model that permits clients to
 * do their own rendering and share it using one of a range of buffer
 * sharing alternatives.  The caller of this routine must specify which
 * @p engine_name to use (e.g. @c "wayland_egl" or @c "wayland_shm").
 *
 * @see ecore_evas_new()
 * @see ecore_evas_wayland_shm_new()
 * @see ecore_evas_wayland_egl_new()
 */
Ecore_Evas *_ecore_evas_wl_common_new_internal(const char *disp_name, Ecore_Window parent, int x, int y, int w, int h, Eina_Bool frame, const int *opt, const char *engine_name);

#endif /* _ECORE_EVAS_WAYLAND_PRIVATE_H_ */
