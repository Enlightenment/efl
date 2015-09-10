#ifndef _ECORE_WL2_PRIVATE_H
# define _ECORE_WL2_PRIVATE_H

# include "Ecore_Wl2.h"
# include "Ecore_Input.h"

# include "xdg-shell-client-protocol.h"
# define XDG_VERSION 5

extern int _ecore_wl2_log_dom;

# ifdef ECORE_WL2_DEFAULT_LOG_COLOR
#  undef ECORE_WL2_DEFAULT_LOG_COLOR
# endif
# define ECORE_WL2_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_wl2_log_dom, __VA_ARGS__)

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) EINA_LOG_DOM_CRIT(_ecore_wl2_log_dom, __VA_ARGS__)

struct _Ecore_Wl2_Display
{
   const char *name;

   struct
     {
        struct wl_display *display;
        struct wl_compositor *compositor;
        struct wl_subcompositor *subcompositor;
        struct wl_data_device_manager *data_device_manager;
        struct wl_shm *shm;
        struct wl_shell *wl_shell;
        struct xdg_shell *xdg_shell;
     } wl;

   struct xkb_context *xkb_context;

   Ecore_Fd_Handler *fd_hdl;

   Eina_Hash *globals;

   Eina_Inlist *outputs;
   Eina_Inlist *inputs;

   Eina_Bool sync_done : 1;
};

struct _Ecore_Wl2_Window
{
   Ecore_Wl2_Display *display;

   Ecore_Wl2_Window *parent;

   int id;
   const char *title;
   const char *class;

   struct wl_surface *surface;
   struct wl_shell_surface *wl_shell_surface;
   struct xdg_surface *xdg_surface;
   struct xdg_popup *xdg_popup;

   Eina_Rectangle geometry;

   Ecore_Wl2_Window_Type type;

   Eina_Bool minimized : 1;
   Eina_Bool maximized : 1;
   Eina_Bool fullscreen : 1;
   Eina_Bool focused : 1;
   Eina_Bool resizing : 1;
};

struct _Ecore_Wl2_Output
{
   EINA_INLIST;

   Ecore_Wl2_Display *display;
   struct wl_output *wl_output;

   int mw, mh, transform;
   const char *make, *model;
   Eina_Rectangle geometry;
};

struct _Ecore_Wl2_Input
{
   EINA_INLIST;

   Ecore_Wl2_Display *display;

   struct
     {
        struct wl_seat *seat;
        struct wl_pointer *pointer;
        struct wl_keyboard *keyboard;
        struct wl_touch *touch;
     } wl;

   struct
     {
        struct wl_data_device *device;
        struct wl_data_source *source;
        struct wl_array types;
     } data;

   struct
     {
        const char *name, *theme_name;
        unsigned int index, size;
        struct wl_cursor *wl_cursor;
        struct wl_cursor_theme *theme;
        struct wl_surface *surface;
        struct wl_callback *frame_cb;
        Ecore_Timer *timer;
     } cursor;

   struct
     {
        double sx, sy;
        unsigned int button;
        unsigned int enter_serial;
     } pointer;

   struct
     {
        Ecore_Wl2_Window *pointer;
        Ecore_Wl2_Window *keyboard;
        Ecore_Wl2_Window *touch;
     } focus;

   struct
     {
        unsigned int button, count, timestamp;
        Ecore_Wl2_Window *window;
     } grab;

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
        Ecore_Timer *timer;
        unsigned int sym, key, time;
        double rate, delay;
        Eina_Bool enabled : 1;
     } repeat;
};

void _ecore_wl2_output_add(Ecore_Wl2_Display *display, unsigned int id);
void _ecore_wl2_output_del(Ecore_Wl2_Output *output);

void _ecore_wl2_input_add(Ecore_Wl2_Display *display, unsigned int id);
void _ecore_wl2_input_del(Ecore_Wl2_Input *input);

#endif
