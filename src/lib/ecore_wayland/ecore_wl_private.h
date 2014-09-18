#ifndef _ECORE_WAYLAND_PRIVATE_H
# define _ECORE_WAYLAND_PRIVATE_H

# include <limits.h>
# include <unistd.h>

# include "Ecore.h"
# include "Ecore_Input.h"
# include "Ecore_Wayland.h"
# ifdef USE_IVI_SHELL
# include "ivi-application-client-protocol.h"
# define IVI_SURFACE_ID 6000
# endif

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

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) EINA_LOG_DOM_CRIT(_ecore_wl_log_dom, __VA_ARGS__)

# ifdef ECORE_WL_DEFAULT_CURSOR_SIZE
#  undef ECORE_WL_DEFAULT_CURSOR_SIZE
# endif
# define ECORE_WL_DEFAULT_CURSOR_SIZE 32

typedef struct _Ecore_Wl_Display Ecore_Wl_Display;

struct _Ecore_Wl_Display
{
   struct
     {
        struct wl_display *display;
        struct wl_registry *registry;
        struct wl_compositor *compositor;
        struct wl_subcompositor *subcompositor;
        struct wl_shell *shell;
        struct wl_shell *desktop_shell;
# ifdef USE_IVI_SHELL
        struct ivi_application *ivi_application;
# endif
        struct wl_shm *shm;
        struct wl_data_device_manager *data_device_manager;
     } wl;

   int fd;
   unsigned int mask;
   unsigned int serial;
   int sync_ref_count;
   Ecore_Fd_Handler *fd_hdl;
   Ecore_Idle_Enterer *idle_enterer;

   Eina_Inlist *inputs;
   Eina_Inlist *outputs;
   Eina_Inlist *globals; /** @since 1.7.6 */

   Eina_Bool init_done;

   struct
     {
        struct xkb_context *context;
     } xkb;

   struct wl_cursor_theme *cursor_theme;

   Ecore_Wl_Output *output;
   Ecore_Wl_Input *input;

   void (*output_configure)(Ecore_Wl_Output *output, void *data);
   void *data;
};

struct _Ecore_Wl_Window
{
   Ecore_Wl_Display *display;
   Ecore_Wl_Window *parent;

   struct wl_surface *surface;
   struct wl_shell_surface *shell_surface;
# ifdef USE_IVI_SHELL
   struct ivi_surface *ivi_surface;
   int ivi_surface_id;
# endif

   struct
     {
        struct wl_surface *surface;
        int hot_x, hot_y;
        Eina_Bool set : 1;
     } pointer;

   int id, surface_id;
   int rotation;

   const char *title;
   const char *class_name;

   Eina_Rectangle allocation;

   struct
     {
        int w, h;
     } saved;

   struct
     {
        int x, y, w, h;
     } opaque, input;

   /* Eina_Bool redraw_scheduled : 1; */
   /* Eina_Bool resize_scheduled : 1; */
   Eina_Bool alpha : 1;
   Eina_Bool transparent : 1;
   Eina_Bool has_buffer : 1;

   Ecore_Wl_Window_Type type;
   Ecore_Wl_Window_Buffer_Type buffer_type;

   Ecore_Wl_Input *pointer_device;
   Ecore_Wl_Input *keyboard_device;

   Eina_Bool anim_pending : 1;
   struct wl_callback *anim_callback;

   /* FIXME: Ideally we should record the cursor name for this window
    * so we can compare and avoid unnecessary cursor set calls to wayland */

   Ecore_Wl_Subsurf *subsurfs;

   void *data;
};

struct _Ecore_Wl_Input
{
   EINA_INLIST;
   Ecore_Wl_Display *display;
   struct wl_seat *seat;
   struct wl_pointer *pointer;
   struct wl_keyboard *keyboard;

   struct wl_touch *touch;

   const char *cursor_name;
   struct wl_cursor *cursor;
   struct wl_surface *cursor_surface;
   struct wl_callback *cursor_frame_cb;
   Ecore_Timer *cursor_timer;
   unsigned int cursor_current_index;
   unsigned int cursor_size;
   const char *cursor_theme_name;

   struct wl_data_device *data_device;
   struct wl_data_source *data_source;
   struct wl_array data_types;

   Ecore_Wl_Window *pointer_focus;
   Ecore_Wl_Window *keyboard_focus;
   Ecore_Wl_Window *touch_focus;

   unsigned int button;
   unsigned int timestamp;
   unsigned int modifiers;
   unsigned int pointer_enter_serial;
   int sx, sy;

   Ecore_Wl_Window *grab;
   unsigned int grab_button;
   unsigned int grab_timestamp;

   Ecore_Wl_Dnd_Source *drag_source;
   Ecore_Wl_Dnd_Source *selection_source;

   struct
     {
        struct xkb_keymap *keymap;
        struct xkb_state *state;
        xkb_mod_mask_t control_mask;
        xkb_mod_mask_t alt_mask;
        xkb_mod_mask_t shift_mask;
        xkb_mod_mask_t win_mask;
        xkb_mod_mask_t scroll_mask;
        xkb_mod_mask_t num_mask;
        xkb_mod_mask_t caps_mask;
        xkb_mod_mask_t altgr_mask;
        unsigned int mods_depressed;
        unsigned int mods_latched;
        unsigned int mods_locked;
        unsigned int mods_group;
     } xkb;

   struct
     {
        Ecore_Timer *tmr;
        unsigned int sym, key, time;
     } repeat;
};

struct _Ecore_Wl_Output
{
   EINA_INLIST;
   Ecore_Wl_Display *display;
   struct wl_output *output;
   Eina_Rectangle allocation;
   int mw, mh;
   int transform;

   void (*destroy) (Ecore_Wl_Output *output, void *data);
   void *data;
};

struct _Ecore_Wl_Dnd
{
   Ecore_Wl_Display *ewd;
   Ecore_Wl_Input *input;
};

struct _Ecore_Wl_Dnd_Source
{
   Ecore_Wl_Input *input;

   struct wl_data_offer *data_offer;
   struct wl_array types;

   int refcount;
   int fd;
   int x, y;
};

struct _Ecore_Wl_Dnd_Target
{
   Ecore_Wl_Dnd_Source *source;
};

extern Ecore_Wl_Display *_ecore_wl_disp;

void _ecore_wl_window_init(void);
void _ecore_wl_window_shutdown(void);
Eina_Hash *_ecore_wl_window_hash_get(void);

void _ecore_wl_output_add(Ecore_Wl_Display *ewd, unsigned int id);
void _ecore_wl_output_del(Ecore_Wl_Output *output);

void _ecore_wl_input_add(Ecore_Wl_Display *ewd, unsigned int id);
void _ecore_wl_input_del(Ecore_Wl_Input *input);
void _ecore_wl_input_pointer_xy_get(int *x, int *y);
void _ecore_wl_input_grab_release(Ecore_Wl_Input *input, Ecore_Wl_Window *win);

void _ecore_wl_dnd_add(Ecore_Wl_Input *input, struct wl_data_device *data_device, struct wl_data_offer *offer);
void _ecore_wl_dnd_enter(void *data, struct wl_data_device *data_device, unsigned int timestamp, struct wl_surface *surface, int x, int y, struct wl_data_offer *offer);
void _ecore_wl_dnd_leave(void *data, struct wl_data_device *data_device);
void _ecore_wl_dnd_motion(void *data, struct wl_data_device *data_device, unsigned int timestamp, int x, int y);
void _ecore_wl_dnd_drop(void *data, struct wl_data_device *data_device);
void _ecore_wl_dnd_selection(void *data, struct wl_data_device *data_device, struct wl_data_offer *offer);
void _ecore_wl_dnd_del(Ecore_Wl_Dnd_Source *source);

void _ecore_wl_events_init(void);
void _ecore_wl_events_shutdown(void);

void _ecore_wl_subsurfs_del_all(Ecore_Wl_Window *win);

struct wl_compositor *_ecore_wl_compositor_get(void);
struct wl_subcompositor *_ecore_wl_subcompositor_get(void);

#endif
