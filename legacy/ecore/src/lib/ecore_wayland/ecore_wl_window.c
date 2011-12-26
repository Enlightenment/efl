#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Wayland.h"
#include "ecore_wl_private.h"

/* local function prototypes */
static void _ecore_wl_window_cb_frame(void *data, struct wl_callback *cb __UNUSED__, uint32_t tm __UNUSED__);

/* local variables */
static const struct wl_callback_listener _ecore_wl_window_frame_listener = 
{
   _ecore_wl_window_cb_frame
};

EAPI Ecore_Wl_Window *
ecore_wl_window_new(Ecore_Wl_Window_Type type, int x, int y, int w, int h) 
{
   static int _win_id = 1;
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(win = calloc(1, sizeof(Ecore_Wl_Window)))) return NULL;

   win->id = _win_id++;
   printf("Ecore_Wl Window Id: %d\n", win->id);
   win->x = x;
   win->y = y;
   win->w = w;
   win->h = h;
   win->synced = EINA_TRUE;
   win->type = type;

   return win;
}

EAPI void 
ecore_wl_window_free(Ecore_Wl_Window *win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
//   ecore_wl_window_hide(win);
   free(win);
}

EAPI void 
ecore_wl_window_move(Ecore_Wl_Window *win, int x, int y) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if ((win->x == x) && (win->y == y)) return;
   win->x = x;
   win->y = y;
}

EAPI void 
ecore_wl_window_resize(Ecore_Wl_Window *win, int w, int h) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if ((win->w == w) && (win->h == h)) return;
   win->w = w;
   win->h = h;
}

EAPI void 
ecore_wl_window_show(Ecore_Wl_Window *win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   win->surface = wl_compositor_create_surface(ecore_wl_compositor_get());

   win->shell_surface = 
     wl_shell_get_shell_surface(ecore_wl_shell_get(), win->surface);
   wl_shell_surface_set_toplevel(win->shell_surface);
}

EAPI void 
ecore_wl_window_hide(Ecore_Wl_Window *win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (win->callback) wl_callback_destroy(win->callback);
   if (win->shell_surface) wl_shell_surface_destroy(win->shell_surface);
   if (win->surface) wl_surface_destroy(win->surface);
}

EAPI void 
ecore_wl_window_raise(Ecore_Wl_Window *win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (!win->shell_surface) return;
   wl_shell_surface_set_toplevel(win->shell_surface);
}

EAPI void 
ecore_wl_window_lower(Ecore_Wl_Window *win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
}

EAPI void 
ecore_wl_window_activate(Ecore_Wl_Window *win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (!win->shell_surface) return;
   wl_shell_surface_set_toplevel(win->shell_surface);
}

EAPI void 
ecore_wl_window_focus(Ecore_Wl_Window *win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (!win->shell_surface) return;
   wl_shell_surface_set_toplevel(win->shell_surface);
}

EAPI void 
ecore_wl_window_fullscreen(Ecore_Wl_Window *win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (!win->shell_surface) return;
   wl_shell_surface_set_fullscreen(win->shell_surface);
}

EAPI void 
ecore_wl_window_attach(Ecore_Wl_Window *win, struct wl_buffer *buffer) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if ((win->surface) && (buffer))
     wl_surface_attach(win->surface, buffer, 0, 0);
}

EAPI void 
ecore_wl_window_damage(Ecore_Wl_Window *win, int x, int y, int w, int h) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (!win->surface) return;
   if (win->synced) 
     {
        if (win->callback) wl_callback_destroy(win->callback);
        win->callback = wl_surface_frame(win->surface);
        wl_callback_add_listener(win->callback, 
                                 &_ecore_wl_window_frame_listener, win);
        win->synced = EINA_FALSE;
     }
   wl_surface_damage(win->surface, x, y, w, h);
}

EAPI void 
ecore_wl_window_sync(Ecore_Wl_Window *win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   ecore_wl_flush();
   while (!win->synced)
     ecore_wl_sync();
}

/* local functions */
static void 
_ecore_wl_window_cb_frame(void *data, struct wl_callback *cb __UNUSED__, uint32_t tm __UNUSED__) 
{
   Ecore_Wl_Window *win;

   if (!(win = data)) return;
   win->synced = EINA_TRUE;
   if (win->callback) wl_callback_destroy(win->callback);
   win->callback = NULL;
   wl_surface_damage(win->surface, 0, 0, win->w, win->h);
}
