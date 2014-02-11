#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <Eina.h>
#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>
#include <Ecore_Evas.h>
#include "ecore_evas_private.h"
#include "ecore_evas_drm.h"

#ifdef BUILD_ECORE_EVAS_DRM
# include <Evas_Engine_Drm.h>
#endif

/* local structures */
typedef struct _Ecore_Evas_Engine_Data_Drm Ecore_Evas_Engine_Data_Drm;

struct _Ecore_Evas_Engine_Data_Drm
{
   int fd;
};

/* local function prototypes */
static int _ecore_evas_drm_init(void);
static int _ecore_evas_drm_shutdown(void);
static Ecore_Evas_Interface_Drm *_ecore_evas_drm_interface_new(void);

/* local variables */
static int _ecore_evas_init_count = 0;

static Ecore_Evas_Engine_Func _ecore_evas_drm_engine_func = 
{
   NULL, //void (*fn_free) (Ecore_Evas *ee);
   NULL, //void (*fn_callback_resize_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_move_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_show_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_hide_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_delete_request_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_destroy_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_focus_in_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_focus_out_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_mouse_in_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_mouse_out_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_sticky_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_unsticky_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_pre_render_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_post_render_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_move) (Ecore_Evas *ee, int x, int y);
   NULL, //void (*fn_managed_move) (Ecore_Evas *ee, int x, int y);
   NULL, //void (*fn_resize) (Ecore_Evas *ee, int w, int h);
   NULL, //void (*fn_move_resize) (Ecore_Evas *ee, int x, int y, int w, int h);
   NULL, //void (*fn_rotation_set) (Ecore_Evas *ee, int rot, int resize);
   NULL, //void (*fn_shaped_set) (Ecore_Evas *ee, int shaped);
   NULL, //void (*fn_show) (Ecore_Evas *ee);
   NULL, //void (*fn_hide) (Ecore_Evas *ee);
   NULL, //void (*fn_raise) (Ecore_Evas *ee);
   NULL, //void (*fn_lower) (Ecore_Evas *ee);
   NULL, //void (*fn_activate) (Ecore_Evas *ee);
   NULL, //void (*fn_title_set) (Ecore_Evas *ee, const char *t);
   NULL, //void (*fn_name_class_set) (Ecore_Evas *ee, const char *n, const char *c);
   NULL, //void (*fn_size_min_set) (Ecore_Evas *ee, int w, int h);
   NULL, //void (*fn_size_max_set) (Ecore_Evas *ee, int w, int h);
   NULL, //void (*fn_size_base_set) (Ecore_Evas *ee, int w, int h);
   NULL, //void (*fn_size_step_set) (Ecore_Evas *ee, int w, int h);
   NULL, //void (*fn_object_cursor_set) (Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y);
   NULL, //void (*fn_layer_set) (Ecore_Evas *ee, int layer);
   NULL, //void (*fn_focus_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_iconified_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_borderless_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_override_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_maximized_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_fullscreen_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_avoid_damage_set) (Ecore_Evas *ee, int on);
   NULL, //void (*fn_withdrawn_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_sticky_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_ignore_events_set) (Ecore_Evas *ee, int ignore);
   NULL, //void (*fn_alpha_set) (Ecore_Evas *ee, int alpha);
   NULL, //void (*fn_transparent_set) (Ecore_Evas *ee, int transparent);
   NULL, //void (*fn_profiles_set) (Ecore_Evas *ee, const char **profiles, int count);
   NULL, //void (*fn_profile_set) (Ecore_Evas *ee, const char *profile);
   
   NULL, //void (*fn_window_group_set) (Ecore_Evas *ee, const Ecore_Evas *ee_group);
   NULL, //void (*fn_aspect_set) (Ecore_Evas *ee, double aspect);
   NULL, //void (*fn_urgent_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_modal_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_demands_attention_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_focus_skip_set) (Ecore_Evas *ee, Eina_Bool on);
   
   NULL, //int (*fn_render) (Ecore_Evas *ee);
   NULL, //void (*fn_screen_geometry_get) (const Ecore_Evas *ee, int *x, int *y, int *w, int *h);
   NULL, //void (*fn_screen_dpi_get) (const Ecore_Evas *ee, int *xdpi, int *ydpi);
   NULL, //void (*fn_msg_parent_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);
   NULL, //void (*fn_msg_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);

   NULL, // pointer_xy_get
   NULL, // pointer_warp

   NULL, // wm_rot_preferred_rotation_set
   NULL, // wm_rot_available_rotations_set
   NULL, // wm_rot_manual_rotation_done_set
   NULL, // wm_rot_manual_rotation_done

   NULL  // aux_hints_set
};

EAPI Ecore_Evas *
ecore_evas_drm_new_internal(const char *device, unsigned int parent, int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   int method;

   /* try to find the evas drm engine */
   if (!(method = evas_render_method_lookup("drm")))
     {
        ERR("Render method lookup failed for Drm");
        return NULL;
     }

   /* try to allocate space for new ecore_evas */
   if (!(ee = calloc(1, sizeof(Ecore_Evas))))
     {
        ERR("Failed to allocate space for new Ecore_Evas");
        return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   /* try to init drm */

   return ee;
}

/* local functions */
static int 
_ecore_evas_drm_init(void)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;

   ecore_event_evas_init();

   return _ecore_evas_init_count;
}

static int 
_ecore_evas_drm_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
        ecore_event_evas_shutdown();
     }

   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}
