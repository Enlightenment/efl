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

#endif
