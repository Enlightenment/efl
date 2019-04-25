#ifndef _ECORE_EVAS_PRIVATE_H
#define _ECORE_EVAS_PRIVATE_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#define ECORE_MAGIC_EVAS 0x76543211

/** Log domain macros and variables **/

EAPI extern int _ecore_evas_log_dom;

#ifdef ECORE_EVAS_DEFAULT_LOG_COLOR
# undef ECORE_EVAS_DEFAULT_LOG_COLOR
#endif
#define ECORE_EVAS_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_evas_log_dom, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_evas_log_dom, __VA_ARGS__)
#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ecore_evas_log_dom, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_evas_log_dom, __VA_ARGS__)
#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_ecore_evas_log_dom, __VA_ARGS__)

#define PORTRAIT_CHECK(r) \
  ((r == 0) || (r == 180))

#define ECORE_EVAS_PORTRAIT(ee) \
  (PORTRAIT_CHECK(ee->rotation))


#define IDLE_FLUSH_TIME 0.5
#ifndef _ECORE_EVAS_H
typedef struct _Ecore_Evas Ecore_Evas;
typedef void   (*Ecore_Evas_Event_Cb) (Ecore_Evas *ee);
#endif

typedef struct _Ecore_Evas_Engine Ecore_Evas_Engine;
typedef struct _Ecore_Evas_Engine_Func Ecore_Evas_Engine_Func;
typedef struct _Ecore_Evas_Interface Ecore_Evas_Interface;
typedef struct _Ecore_Evas_Aux_Hint Ecore_Evas_Aux_Hint;
typedef struct _Ecore_Evas_Cursor Ecore_Evas_Cursor;

/* Engines interfaces */
struct _Ecore_Evas_Engine_Func
{
   void (*fn_free) (Ecore_Evas *ee);
   void (*fn_callback_resize_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_move_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_show_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_hide_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_delete_request_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_destroy_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_focus_in_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_focus_out_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_mouse_in_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_mouse_out_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_sticky_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_unsticky_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_pre_render_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_callback_post_render_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   void (*fn_move) (Ecore_Evas *ee, int x, int y);
   void (*fn_managed_move) (Ecore_Evas *ee, int x, int y);
   void (*fn_resize) (Ecore_Evas *ee, int w, int h);
   void (*fn_move_resize) (Ecore_Evas *ee, int x, int y, int w, int h);
   void (*fn_rotation_set) (Ecore_Evas *ee, int rot, int resize);
   void (*fn_shaped_set) (Ecore_Evas *ee, int shaped);
   void (*fn_show) (Ecore_Evas *ee);
   void (*fn_hide) (Ecore_Evas *ee);
   void (*fn_raise) (Ecore_Evas *ee);
   void (*fn_lower) (Ecore_Evas *ee);
   void (*fn_activate) (Ecore_Evas *ee);
   void (*fn_title_set) (Ecore_Evas *ee, const char *t);
   void (*fn_name_class_set) (Ecore_Evas *ee, const char *n, const char *c);
   void (*fn_size_min_set) (Ecore_Evas *ee, int w, int h);
   void (*fn_size_max_set) (Ecore_Evas *ee, int w, int h);
   void (*fn_size_base_set) (Ecore_Evas *ee, int w, int h);
   void (*fn_size_step_set) (Ecore_Evas *ee, int w, int h);
   void (*fn_object_cursor_set) (Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y);
   void (*fn_object_cursor_unset) (Ecore_Evas *ee);
   void (*fn_layer_set) (Ecore_Evas *ee, int layer);
   void (*fn_focus_set) (Ecore_Evas *ee, Eina_Bool on);
   void (*fn_iconified_set) (Ecore_Evas *ee, Eina_Bool on);
   void (*fn_borderless_set) (Ecore_Evas *ee, Eina_Bool on);
   void (*fn_override_set) (Ecore_Evas *ee, Eina_Bool on);
   void (*fn_maximized_set) (Ecore_Evas *ee, Eina_Bool on);
   void (*fn_fullscreen_set) (Ecore_Evas *ee, Eina_Bool on);
   void (*fn_avoid_damage_set) (Ecore_Evas *ee, int on);
   void (*fn_withdrawn_set) (Ecore_Evas *ee, Eina_Bool on);
   void (*fn_sticky_set) (Ecore_Evas *ee, Eina_Bool on);
   void (*fn_ignore_events_set) (Ecore_Evas *ee, int ignore);
   void (*fn_alpha_set) (Ecore_Evas *ee, int alpha);
   void (*fn_transparent_set) (Ecore_Evas *ee, int transparent);
   void (*fn_profiles_set) (Ecore_Evas *ee, const char **profiles, int count);
   void (*fn_profile_set) (Ecore_Evas *ee, const char *profile);

   void (*fn_window_group_set) (Ecore_Evas *ee, const Ecore_Evas *ee_group);
   void (*fn_aspect_set) (Ecore_Evas *ee, double aspect);
   void (*fn_urgent_set) (Ecore_Evas *ee, Eina_Bool on);
   void (*fn_modal_set) (Ecore_Evas *ee, Eina_Bool on);
   void (*fn_demands_attention_set) (Ecore_Evas *ee, Eina_Bool on);
   void (*fn_focus_skip_set) (Ecore_Evas *ee, Eina_Bool on);

   int (*fn_render) (Ecore_Evas *ee);
   void (*fn_screen_geometry_get) (const Ecore_Evas *ee, int *x, int *y, int *w, int *h);
   void (*fn_screen_dpi_get) (const Ecore_Evas *ee, int *xdpi, int *ydpi);
   void (*fn_msg_parent_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);
   void (*fn_msg_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);

   /* 1.8 abstractions */
   void (*fn_pointer_xy_get) (const Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y);
   Eina_Bool (*fn_pointer_warp) (const Ecore_Evas *ee, Evas_Coord x, Evas_Coord y);

   void (*fn_wm_rot_preferred_rotation_set) (Ecore_Evas *ee, int rot);
   void (*fn_wm_rot_available_rotations_set) (Ecore_Evas *ee, const int *rots, unsigned int count);
   void (*fn_wm_rot_manual_rotation_done_set) (Ecore_Evas *ee, Eina_Bool set);
   void (*fn_wm_rot_manual_rotation_done) (Ecore_Evas *ee);

   void (*fn_aux_hints_set) (Ecore_Evas *ee, const char *hints);

   void (*fn_animator_register)  (Ecore_Evas *ee);
   void (*fn_animator_unregister)(Ecore_Evas *ee);

   void (*fn_evas_changed)(Ecore_Evas *ee, Eina_Bool changed);

   void (*fn_focus_device_set) (Ecore_Evas *ee, Efl_Input_Device *seat, Eina_Bool on);
   void (*fn_callback_focus_device_in_set) (Ecore_Evas *ee, Ecore_Evas_Focus_Device_Event_Cb func);
   void (*fn_callback_focus_device_out_set) (Ecore_Evas *ee, Ecore_Evas_Focus_Device_Event_Cb func);

   void (*fn_callback_device_mouse_in_set) (Ecore_Evas *ee, Ecore_Evas_Mouse_IO_Cb func);
   void (*fn_callback_device_mouse_out_set) (Ecore_Evas *ee, Ecore_Evas_Mouse_IO_Cb func);
   void (*fn_pointer_device_xy_get)(const Ecore_Evas *ee, const Efl_Input_Device *pointer, Evas_Coord *x, Evas_Coord *y);

   Eina_Bool (*fn_prepare)(Ecore_Evas *ee);

   double (*fn_last_tick_get)(Ecore_Evas *ee);
};

struct _Ecore_Evas_Interface
{
    const char *name;
    unsigned int version;
};

struct _Ecore_Evas_Engine
{
   Ecore_Evas_Engine_Func *func;
   void *data;
   Eina_List *ifaces;
   Ecore_Timer *idle_flush_timer;
#ifdef BUILD_ECORE_EVAS_EWS
   struct {
      Evas_Object *image;
   } ews;
#endif
};

struct _Ecore_Evas_Cursor {
   Evas_Object *object;
   int          layer;
   struct {
      int       x, y;
   } hot;
   int pos_x;
   int pos_y;
};

struct _Ecore_Evas
{
   EINA_INLIST;
   ECORE_MAGIC;
   Evas       *evas;
   const char *driver;
   char       *name;
   int         x, y, w, h;
   short       rotation;
   Eina_Bool   shaped  : 1;
   Eina_Bool   visible : 1;
   Eina_Bool   draw_block : 1;
   Eina_Bool   should_be_visible : 1;
   Eina_Bool   alpha  : 1;
   Eina_Bool   transparent  : 1;
   Eina_Bool   events_block  : 1; /* @since 1.14 */

   Eina_Hash  *data;
   Eina_List  *mice_in;

   Eina_List  *vnc_server; /* @since 1.19 */

   struct {
      int      x, y, w, h;
   } req;

   struct {
      int      l, r, t, b;
      int      changed : 1;
   } shadow;

   struct {
      int      w, h;
   } expecting_resize;

   struct {
      int      w, h;
   } framespace;

   struct {
      Eina_Hash      *cursors;
      char           *title;
      char           *name;
      char           *clas;
      struct {
         char        *name;
         char       **available_list;
         int          count;
      } profile;
      struct {
         int          w, h;
      } min, max, base, step;
      Ecore_Evas_Cursor cursor_cache;
      struct {
         Eina_Bool       supported;      // indicate that the underlying window system supports window manager rotation protocol
         Eina_Bool       app_set;        // indicate that the ee supports window manager rotation protocol
         Eina_Bool       win_resize;     // indicate that the ee will be resized by the WM
         int             angle;          // rotation value which is decided by the WM 
         int             w, h;           // window size to rotate
         int             preferred_rot;  // preferred rotation hint
         int            *available_rots; // array of avaialable rotation values
         unsigned int    count;          // number of elements of available_rots
         struct {
            Eina_Bool    set;
            Eina_Bool    wait_for_done;
            Ecore_Timer *timer;
         } manual_mode;
      } wm_rot;
      struct {
         Eina_List      *supported_list;
         Eina_List      *hints;
         int             id;
      } aux_hint;
      Eina_List       *focused_by;
      int             layer;
      Ecore_Window    window;
      unsigned char   avoid_damage;
      Ecore_Evas     *group_ee;
      Ecore_Window    group_ee_win;
      double          aspect;
      Eina_Bool       iconified    : 1;
      Eina_Bool       borderless   : 1;
      Eina_Bool       override     : 1;
      Eina_Bool       maximized    : 1;
      Eina_Bool       fullscreen   : 1;
      Eina_Bool       withdrawn    : 1;
      Eina_Bool       sticky       : 1;
      Eina_Bool       request_pos  : 1;
      Eina_Bool       hwsurface    : 1;
      Eina_Bool       urgent           : 1;
      Eina_Bool       modal            : 1;
      Eina_Bool       demand_attention : 1;
      Eina_Bool       focus_skip       : 1;
      Eina_Bool       focused       : 1;
  } prop;

   struct {
      void          (*fn_resize) (Ecore_Evas *ee);
      void          (*fn_move) (Ecore_Evas *ee);
      void          (*fn_show) (Ecore_Evas *ee);
      void          (*fn_hide) (Ecore_Evas *ee);
      void          (*fn_delete_request) (Ecore_Evas *ee);
      void          (*fn_destroy) (Ecore_Evas *ee);
      void          (*fn_focus_in) (Ecore_Evas *ee);
      void          (*fn_focus_out) (Ecore_Evas *ee);
      void          (*fn_sticky) (Ecore_Evas *ee);
      void          (*fn_unsticky) (Ecore_Evas *ee);
      void          (*fn_mouse_in) (Ecore_Evas *ee);
      void          (*fn_mouse_out) (Ecore_Evas *ee);
      void          (*fn_pre_render) (Ecore_Evas *ee);
      void          (*fn_post_render) (Ecore_Evas *ee);
      void          (*fn_pre_free) (Ecore_Evas *ee);
      void          (*fn_state_change) (Ecore_Evas *ee);
      void          (*fn_msg_parent_handle) (Ecore_Evas *ee, int maj, int min, void *data, int size);
      void          (*fn_msg_handle) (Ecore_Evas *ee, int maj, int min, void *data, int size);
      void          (*fn_pointer_xy_get) (const Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y);
      Eina_Bool     (*fn_pointer_warp) (const Ecore_Evas *ee, Evas_Coord x, Evas_Coord y);
      void          (*fn_focus_device_in) (Ecore_Evas *ee, Efl_Input_Device *seat);
      void          (*fn_focus_device_out) (Ecore_Evas *ee, Efl_Input_Device *seat);
      void          (*fn_device_mouse_in) (Ecore_Evas *ee, Efl_Input_Device *mouse);
      void          (*fn_device_mouse_out) (Ecore_Evas *ee, Efl_Input_Device *mouse);
   } func;

   Ecore_Evas_Engine engine;
   Eina_List *sub_ecore_evas;

   // Animator code
   Ecore_Animator *anim;
   unsigned int animator_count;

   struct {
      Eina_Inlist *active;
      Eina_Inlist *deleted;
      Eina_Inlist *suspended;
      Eina_Inlist *run_list;
   } ee_anim;

   struct {
      unsigned char avoid_damage;
      unsigned char resize_shape : 1;
      unsigned char shaped : 1;
      unsigned char shaped_changed : 1;
      unsigned char alpha : 1;
      unsigned char alpha_changed : 1;
      unsigned char transparent : 1;
      unsigned char transparent_changed : 1;
      int           rotation;
      int           rotation_resize;
      unsigned char rotation_changed : 1;
   } delayed;

   int refcount;
//#define ECORE_EVAS_ASYNC_RENDER_DEBUG 1 /* TODO: remove me */
#ifdef ECORE_EVAS_ASYNC_RENDER_DEBUG
   double async_render_start;
#endif

   /* A flag to show if sync_draw_done is sent by gl or by ecore_evas.
      Some GL drivers do buffer copy in a separate thread.  We need to check
      whether GL driver sends SYNC_DRAW_DONE message after buffer copy.
      This is required to synchronize rendering and sync_draw_done message
      sending.

      -1 : uninitialized
      0 : sync_draw_done is sent by ecore_evas
      1 : sync_draw_done is sent by gl
    */
   signed char   gl_sync_draw_done;

   unsigned char ignore_events : 1;
   unsigned char manual_render : 1;
   unsigned char registered : 1;
   unsigned char no_comp_sync  : 1;
   unsigned char semi_sync  : 1;
   unsigned char deleted : 1;
   unsigned char profile_supported : 1;

   unsigned char in_async_render : 1;
   unsigned char can_async_render : 1;
   unsigned char animator_registered : 1;
   unsigned char animator_ticked : 1;
   unsigned char animator_ran : 1;
   unsigned char first_frame : 1;
};

struct _Ecore_Evas_Aux_Hint
{
   int           id;           // ID of aux hint
   const char   *hint;         // hint string
   const char   *val;          // value string
   unsigned char allowed : 1;  // received allowed event from the window manager
   unsigned char notified : 1; // let caller know ee has got response for this aux hint
};

EAPI void _ecore_evas_ref(Ecore_Evas *ee);
EAPI void _ecore_evas_unref(Ecore_Evas *ee);
EAPI int ecore_evas_buffer_render(Ecore_Evas *ee);

EAPI void _ecore_evas_fps_debug_init(void);
EAPI void _ecore_evas_fps_debug_shutdown(void);
EAPI void _ecore_evas_fps_debug_rendertime_add(double t);
EAPI void _ecore_evas_register(Ecore_Evas *ee);
EAPI void _ecore_evas_subregister(Ecore_Evas *ee_target, Ecore_Evas *ee);
EAPI void _ecore_evas_register_animators(Ecore_Evas *ee);
EAPI void _ecore_evas_free(Ecore_Evas *ee);
EAPI void _ecore_evas_idle_timeout_update(Ecore_Evas *ee);
EAPI void _ecore_evas_mouse_move_process(Ecore_Evas *ee, int x, int y, unsigned int timestamp);
EAPI void _ecore_evas_mouse_device_move_process(Ecore_Evas *ee, Efl_Input_Device *pointer,
                                                int x, int y, unsigned int timestamp);
EAPI void _ecore_evas_mouse_multi_move_process(Ecore_Evas *ee, int device,
                                          int x, int y,
                                          double radius,
                                          double radius_x, double radius_y,
                                          double pressure,
                                          double angle,
                                          double mx, double my,
                                          unsigned int timestamp);
EAPI void _ecore_evas_mouse_multi_down_process(Ecore_Evas *ee, int device,
                                          int x, int y,
                                          double radius,
                                          double radius_x, double radius_y,
                                          double pressure,
                                          double angle,
                                          double mx, double my,
                                          Evas_Button_Flags flags,
                                          unsigned int timestamp);
EAPI void _ecore_evas_mouse_multi_up_process(Ecore_Evas *ee, int device,
                                        int x, int y,
                                        double radius,
                                        double radius_x, double radius_y,
                                        double pressure,
                                        double angle,
                                        double mx, double my,
                                        Evas_Button_Flags flags,
                                        unsigned int timestamp);
EAPI Eina_Bool _ecore_evas_input_direct_cb(void *window, int type, const void *info);

EAPI extern Eina_Bool _ecore_evas_app_comp_sync;


EAPI Ecore_Evas_Interface *_ecore_evas_interface_get(const Ecore_Evas *ee, const char *iname);

/**
 * @brief Free the string of the window profile.
 *
 * This is a helper function to free window profile.
 */
EAPI void _ecore_evas_window_profile_free(Ecore_Evas *ee);

/**
 * @brief Free the string array of available window profiles.
 *
 * This is a helper function to free available window profiles.
 */
EAPI void _ecore_evas_window_available_profiles_free(Ecore_Evas *ee);

#ifdef BUILD_ECORE_EVAS_EWS
void _ecore_evas_ews_events_init(void);
void _ecore_evas_ews_events_flush(void);
int _ecore_evas_ews_shutdown(void);
#endif

void _ecore_evas_extn_init(void);
void _ecore_evas_extn_shutdown(void);

EAPI Eina_Strbuf *_ecore_evas_aux_hints_string_get(Ecore_Evas *ee);
void              _ecore_evas_aux_hint_free(Ecore_Evas *ee);

Eina_Module *_ecore_evas_engine_load(const char *engine);
const Eina_List *_ecore_evas_available_engines_get(void);
void _ecore_evas_engine_init(void);
void _ecore_evas_engine_shutdown(void);

EAPI void ecore_evas_animator_tick(Ecore_Evas *ee, Eina_Rectangle *viewport, double loop_time);

Eina_Module *_ecore_evas_vnc_server_module_load(void);

EAPI void _ecore_evas_focus_device_set(Ecore_Evas *ee, Efl_Input_Device *seat,
                                       Eina_Bool on);

EAPI Eina_Bool _ecore_evas_mouse_in_check(Ecore_Evas *ee, Efl_Input_Device *mouse);
EAPI void _ecore_evas_mouse_inout_set(Ecore_Evas *ee, Efl_Input_Device *mouse,
                                      Eina_Bool in, Eina_Bool force_out);

EAPI Evas_Object *_ecore_evas_default_cursor_image_get(Ecore_Evas *ee);
EAPI void _ecore_evas_default_cursor_hide(Ecore_Evas *ee);

Eina_Bool _ecore_evas_cursors_init(Ecore_Evas *ee);

EAPI void ecore_evas_render_wait(Ecore_Evas *ee);
EAPI Eina_Bool ecore_evas_render(Ecore_Evas *ee);

EAPI Evas *ecore_evas_evas_new(Ecore_Evas *ee, int w, int h);
EAPI void ecore_evas_done(Ecore_Evas *ee, Eina_Bool single_window);

#ifdef IPA_YLNO_ESU_LANRETNI_MLE
EAPI Ecore_Evas *_wayland_shm_new(const char *disp_name, Ecore_Window parent, int x, int y, int w, int h, Eina_Bool frame);
EAPI Ecore_Evas *_wayland_egl_new(const char *disp_name, Ecore_Window parent, int x, int y, int w, int h, Eina_Bool frame, const int *opt);
#else
#define _wayland_shm_new DONT_USE_INTERNAL_API
#define _wayland_egl_new DONT_USE_INTERNAL_API
#endif

static inline Eina_Bool
ecore_evas_render_prepare(Ecore_Evas *ee)
{
   Ecore_Evas *ee2;
   Eina_List *ll;
   Eina_Bool r = EINA_FALSE;

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
          r |= ee2->engine.func->fn_render(ee2);
        else
          r |= ecore_evas_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }

   // We do not force the child to be sync, so we should wait for them to be done
   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     if (!ee2->engine.func->fn_render)
       ecore_evas_render_wait(ee2);

   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);
   return r;
}

#undef EAPI
#define EAPI

#endif
