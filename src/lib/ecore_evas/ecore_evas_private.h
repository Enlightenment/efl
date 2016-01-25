#ifndef _ECORE_EVAS_PRIVATE_H
#define _ECORE_EVAS_PRIVATE_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_EVAS_BUILD */
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
#endif /* ! _WIN32 */

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
   Eina_Bool   draw_ok : 1;
   Eina_Bool   should_be_visible : 1;
   Eina_Bool   alpha  : 1;
   Eina_Bool   transparent  : 1;
   Eina_Bool   in  : 1;
   Eina_Bool   events_block  : 1; /* @since 1.14 */

   Eina_Hash  *data;

   struct {
      int      x, y, w, h;
   } req;

   struct {
      int      x, y;
   } mouse;

   struct {
      int      w, h;
   } expecting_resize;

   struct {
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
      struct {
         Evas_Object *object;
         int          layer;
         struct {
            int       x, y;
         } hot;
      } cursor;
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
      int             layer;
      Ecore_Window    window;
      unsigned char   avoid_damage;
      Ecore_Evas     *group_ee;
      Ecore_Window    group_ee_win;
      double          aspect;
      Eina_Bool       focused      : 1;
      Eina_Bool       iconified    : 1;
      Eina_Bool       borderless   : 1;
      Eina_Bool       override     : 1;
      Eina_Bool       maximized    : 1;
      Eina_Bool       fullscreen   : 1;
      Eina_Bool       withdrawn    : 1;
      Eina_Bool       sticky       : 1;
      Eina_Bool       request_pos  : 1;
      Eina_Bool       draw_frame   : 1;
      Eina_Bool       hwsurface    : 1;
      Eina_Bool       urgent           : 1;
      Eina_Bool       modal            : 1;
      Eina_Bool       demand_attention : 1;
      Eina_Bool       focus_skip       : 1;
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
   } func;

   Ecore_Evas_Engine engine;
   Eina_List *sub_ecore_evas;

   // Animator code
   Ecore_Animator *anim;

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
#define ECORE_EVAS_ASYNC_RENDER_DEBUG 1 /* TODO: remove me */
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
EAPI void _ecore_evas_free(Ecore_Evas *ee);
EAPI void _ecore_evas_idle_timeout_update(Ecore_Evas *ee);
EAPI void _ecore_evas_mouse_move_process(Ecore_Evas *ee, int x, int y, unsigned int timestamp);
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

EAPI void ecore_evas_animator_tick(Ecore_Evas *ee, Eina_Rectangle *viewport);

#undef EAPI
#define EAPI

#endif
