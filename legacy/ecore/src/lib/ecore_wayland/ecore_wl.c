#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <fcntl.h>

/* FIXME: This gives BTN_LEFT/RIGHT/MIDDLE for linux systems ... 
 *        What about other OSs ?? */
#ifdef __linux__
# include <linux/input.h>
#else
# define BTN_LEFT 0x110
# define BTN_RIGHT 0x111
# define BTN_MIDDLE 0x112
# define BTN_SIDE 0x113
# define BTN_EXTRA 0x114
# define BTN_FORWARD 0x115
# define BTN_BACK 0x116
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Input.h"
#include "ecore_wl_private.h"
#include "Ecore_Wayland.h"

/* local function prototypes */
static Eina_Bool _ecore_wl_shutdown(Eina_Bool close);
static int _ecore_wl_cb_event_mask_update(unsigned int mask, void *data);
static Eina_Bool _ecore_wl_cb_handle_data(void *data, Ecore_Fd_Handler *hdl __UNUSED__);
static void _ecore_wl_cb_handle_global(struct wl_display *disp, unsigned int id, const char *interface, unsigned int version __UNUSED__, void *data);
static Eina_Bool _ecore_wl_egl_init(Ecore_Wl_Display *ewd);
static Eina_Bool _ecore_wl_egl_shutdown(Ecore_Wl_Display *ewd);
static Eina_Bool _ecore_wl_xkb_init(Ecore_Wl_Display *ewd);
static Eina_Bool _ecore_wl_xkb_shutdown(Ecore_Wl_Display *ewd);

/* local variables */
static int _ecore_wl_init_count = 0;

/* external variables */
int _ecore_wl_log_dom = -1;
Ecore_Wl_Display *_ecore_wl_disp = NULL;

EAPI int ECORE_WL_EVENT_MOUSE_IN = 0;
EAPI int ECORE_WL_EVENT_MOUSE_OUT = 0;
EAPI int ECORE_WL_EVENT_FOCUS_IN = 0;
EAPI int ECORE_WL_EVENT_FOCUS_OUT = 0;
EAPI int ECORE_WL_EVENT_WINDOW_CONFIGURE = 0;
EAPI int ECORE_WL_EVENT_DND_ENTER = 0;
EAPI int ECORE_WL_EVENT_DND_POSITION = 0;
EAPI int ECORE_WL_EVENT_DND_LEAVE = 0;
EAPI int ECORE_WL_EVENT_DND_DROP = 0;
EAPI int ECORE_WL_EVENT_INTERFACES_BOUND = 0;

/**
 * @defgroup Ecore_Wl_Init_Group Wayland Library Init and Shutdown Functions
 * 
 * Functions that start and shutdown the Ecore Wayland Library.
 */

/**
 * Initialize the Wayland display connection to the given display.
 * 
 * @param   name Display target name. if @c NULL, the default display is 
 *          assumed.
 * @return  The number of times the library has been initialized without being 
 *          shut down. 0 is returned if an error occurs.
 * 
 * @ingroup Ecore_Wl_Init_Group
 */
EAPI int 
ecore_wl_init(const char *name)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (++_ecore_wl_init_count != 1) return _ecore_wl_init_count;

   if (!eina_init()) return --_ecore_wl_init_count;

   _ecore_wl_log_dom = 
     eina_log_domain_register("ecore_wl", ECORE_WL_DEFAULT_LOG_COLOR);
   if (_ecore_wl_log_dom < 0)
     {
        EINA_LOG_ERR("Cannot create a log domain for Ecore Wayland");
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   if (!ecore_init())
     {
        ERR("Could not initialize ecore");
        eina_log_domain_unregister(_ecore_wl_log_dom);
        _ecore_wl_log_dom = -1;
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   if (!ecore_event_init())
     {
        ERR("Could not initialize ecore_event");
        eina_log_domain_unregister(_ecore_wl_log_dom);
        _ecore_wl_log_dom = -1;
        ecore_shutdown();
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   if (!ECORE_WL_EVENT_MOUSE_IN)
     {
        ECORE_WL_EVENT_MOUSE_IN = ecore_event_type_new();
        ECORE_WL_EVENT_MOUSE_OUT = ecore_event_type_new();
        ECORE_WL_EVENT_FOCUS_IN = ecore_event_type_new();
        ECORE_WL_EVENT_FOCUS_OUT = ecore_event_type_new();
        ECORE_WL_EVENT_WINDOW_CONFIGURE = ecore_event_type_new();
        ECORE_WL_EVENT_DND_ENTER = ecore_event_type_new();
        ECORE_WL_EVENT_DND_POSITION = ecore_event_type_new();
        ECORE_WL_EVENT_DND_LEAVE = ecore_event_type_new();
        ECORE_WL_EVENT_DND_DROP = ecore_event_type_new();
        ECORE_WL_EVENT_INTERFACES_BOUND = ecore_event_type_new();
     }

   if (!(_ecore_wl_disp = malloc(sizeof(Ecore_Wl_Display))))
     {
        ERR("Could not allocate memory for Ecore_Wl_Display structure");
        eina_log_domain_unregister(_ecore_wl_log_dom);
        _ecore_wl_log_dom = -1;
        ecore_event_shutdown();
        ecore_shutdown();
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   memset(_ecore_wl_disp, 0, sizeof(Ecore_Wl_Display));

   if (!(_ecore_wl_disp->wl.display = wl_display_connect(name)))
     {
        ERR("Could not connect to Wayland display");
        eina_log_domain_unregister(_ecore_wl_log_dom);
        _ecore_wl_log_dom = -1;
        ecore_event_shutdown();
        ecore_shutdown();
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   _ecore_wl_disp->fd = 
     wl_display_get_fd(_ecore_wl_disp->wl.display, 
                       _ecore_wl_cb_event_mask_update, _ecore_wl_disp);

   _ecore_wl_disp->fd_hdl = 
     ecore_main_fd_handler_add(_ecore_wl_disp->fd, ECORE_FD_READ, 
                               _ecore_wl_cb_handle_data, _ecore_wl_disp, 
                               NULL, NULL);

   wl_list_init(&_ecore_wl_disp->inputs);
   wl_list_init(&_ecore_wl_disp->outputs);

   wl_display_add_global_listener(_ecore_wl_disp->wl.display, 
                                  _ecore_wl_cb_handle_global, _ecore_wl_disp);

   /* FIXME: Process connection events ?? */
   wl_display_iterate(_ecore_wl_disp->wl.display, WL_DISPLAY_READABLE);

   /* if (!_ecore_wl_egl_init(_ecore_wl_disp)) */
   /*   { */
   /*      ERR("Could not initialize EGL"); */
   /*      free(_ecore_wl_disp); */
   /*      eina_log_domain_unregister(_ecore_wl_log_dom); */
   /*      _ecore_wl_log_dom = -1; */
   /*      ecore_event_shutdown(); */
   /*      ecore_shutdown(); */
   /*      eina_shutdown(); */
   /*      return --_ecore_wl_init_count; */
   /*   } */

   /* _ecore_wl_disp->create_image =  */
   /*   (void *)eglGetProcAddress("eglCreateImageKHR"); */
   /* _ecore_wl_disp->destroy_image =  */
   /*   (void *)eglGetProcAddress("eglDestroyImageKHR"); */

   /* TODO: create pointer surfaces */

   if (!_ecore_wl_xkb_init(_ecore_wl_disp))
     {
        ERR("Could not initialize XKB");
        _ecore_wl_egl_shutdown(_ecore_wl_disp);
        free(_ecore_wl_disp);
        eina_log_domain_unregister(_ecore_wl_log_dom);
        _ecore_wl_log_dom = -1;
        ecore_event_shutdown();
        ecore_shutdown();
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   _ecore_wl_window_init();

   return _ecore_wl_init_count;
}

/**
 * Shuts down the Ecore Wayland Library
 * 
 * In shutting down the library, the Wayland display connection is terminated 
 * and any event handlers for it are removed.
 * 
 * @return  The number of times the library has been initialized without 
 *          being shut down.
 * 
 * @ingroup Ecore_Wl_Init_Group
 */
EAPI int 
ecore_wl_shutdown(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_wl_shutdown(EINA_TRUE);
}

/**
 * @defgroup Ecore_Wl_Flush_Group Wayland Synchronization Functions
 * 
 * Functions that ensure that all commands which have been issued by the 
 * Ecore Wayland library have been sent to the server.
 */

/**
 * Sends all Wayland commands to the Wayland Display.
 * 
 * @ingroup Ecore_Wl_Flush_Group
 * @since 1.2
 */
EAPI void 
ecore_wl_flush(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   while (_ecore_wl_disp->mask & WL_DISPLAY_WRITABLE)
     wl_display_iterate(_ecore_wl_disp->wl.display, WL_DISPLAY_WRITABLE);
//   wl_display_flush(_ecore_wl_disp->wl.display); // old flush code
}

/**
 * Flushes the command buffer and waits until all requests have been 
 * processed by the server.
 * 
 * @ingroup Ecore_Wl_Flush_Group
 * @since 1.2
 */
EAPI void 
ecore_wl_sync(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   wl_display_roundtrip(_ecore_wl_disp->wl.display);
   // old sync code
//   wl_display_iterate(_ecore_wl_disp->wl.display, WL_DISPLAY_READABLE);
}

/**
 * @defgroup Ecore_Wl_Display_Group Wayland Display Functions
 * 
 * Functions that set and retrieve various information about the Wayland Display.
 */

/**
 * Retrieves the Wayland Shm Interface used for the current Wayland connection.
 * 
 * @return The current wayland shm interface
 * 
 * @ingroup Ecore_Wl_Display_Group
 * @since 1.2
 */
EAPI struct wl_shm *
ecore_wl_shm_get(void)
{
   return _ecore_wl_disp->wl.shm;
}

/**
 * Retrieves the Wayland Display Interface used for the current Wayland connection.
 * 
 * @return The current wayland display interface
 * 
 * @ingroup Ecore_Wl_Display_Group
 * @since 1.2
 */
EAPI struct wl_display *
ecore_wl_display_get(void)
{
   return _ecore_wl_disp->wl.display;
}

/**
 * Retrieves the size of the current screen.
 * 
 * @param w where to return the width. May be NULL. Returns 0 on error.
 * @param h where to return the height. May be NULL. Returns 0 on error.
 * 
 * @ingroup Ecore_Wl_Display_Group
 * @since 1.2
 */
EAPI void 
ecore_wl_screen_size_get(int *w, int *h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (w) *w = _ecore_wl_disp->output->allocation.w;
   if (h) *h = _ecore_wl_disp->output->allocation.h;
}

/* @since 1.2 */
EAPI void 
ecore_wl_pointer_xy_get(int *x, int *y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_input_pointer_xy_get(x, y);
}

/* local functions */
static Eina_Bool 
_ecore_wl_shutdown(Eina_Bool close)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (--_ecore_wl_init_count != 0) return _ecore_wl_init_count;
   if (!_ecore_wl_disp) return _ecore_wl_init_count;

   _ecore_wl_window_shutdown();

   if (_ecore_wl_disp->fd_hdl) 
     ecore_main_fd_handler_del(_ecore_wl_disp->fd_hdl);

   if (close) 
     {
        Ecore_Wl_Output *out, *tout;
        Ecore_Wl_Input *in, *tin;

        wl_list_for_each_safe(out, tout, &_ecore_wl_disp->outputs, link)
          _ecore_wl_output_del(out);

        wl_list_for_each_safe(in, tin, &_ecore_wl_disp->inputs, link)
          _ecore_wl_input_del(in);

        _ecore_wl_xkb_shutdown(_ecore_wl_disp);
        /* _ecore_wl_egl_shutdown(_ecore_wl_disp); */

        if (_ecore_wl_disp->wl.shell) 
          wl_shell_destroy(_ecore_wl_disp->wl.shell);
        if (_ecore_wl_disp->wl.shm) wl_shm_destroy(_ecore_wl_disp->wl.shm);
        if (_ecore_wl_disp->wl.data_device_manager)
          wl_data_device_manager_destroy(_ecore_wl_disp->wl.data_device_manager);
        if (_ecore_wl_disp->wl.compositor)
          wl_compositor_destroy(_ecore_wl_disp->wl.compositor);
        if (_ecore_wl_disp->wl.display)
          {
             wl_display_flush(_ecore_wl_disp->wl.display);
             wl_display_disconnect(_ecore_wl_disp->wl.display);
          }
        free(_ecore_wl_disp);
     }

   ecore_event_shutdown();
   ecore_shutdown();

   eina_log_domain_unregister(_ecore_wl_log_dom);
   _ecore_wl_log_dom = -1;
   eina_shutdown();

   return _ecore_wl_init_count;
}

static int 
_ecore_wl_cb_event_mask_update(unsigned int mask, void *data)
{
   Ecore_Wl_Display *ewd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ewd = data;
   ewd->mask = mask;
   return 0;
}

static Eina_Bool 
_ecore_wl_cb_handle_data(void *data, Ecore_Fd_Handler *hdl __UNUSED__)
{
   Ecore_Wl_Display *ewd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ewd = data)) return ECORE_CALLBACK_RENEW;
   wl_display_iterate(ewd->wl.display, ewd->mask);
   return ECORE_CALLBACK_RENEW;
}

static void 
_ecore_wl_cb_handle_global(struct wl_display *disp, unsigned int id, const char *interface, unsigned int version __UNUSED__, void *data)
{
   Ecore_Wl_Display *ewd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!strcmp(interface, "wl_display")) || 
       (!strcmp(interface, "wl_drm")) || 
       (!strcmp(interface, "desktop_shell")))
     return;

   ewd = data;

   if (!strcmp(interface, "wl_compositor"))
     ewd->wl.compositor = wl_display_bind(disp, id, &wl_compositor_interface);
   else if (!strcmp(interface, "wl_output"))
     _ecore_wl_output_add(ewd, id);
   else if (!strcmp(interface, "wl_input_device"))
     _ecore_wl_input_add(ewd, id);
   else if (!strcmp(interface, "wl_shell"))
     ewd->wl.shell = wl_display_bind(disp, id, &wl_shell_interface);
   else if (!strcmp(interface, "wl_shm"))
     ewd->wl.shm = wl_display_bind(disp, id, &wl_shm_interface);
   else if (!strcmp(interface, "wl_data_device_manager"))
     {
        ewd->wl.data_device_manager = 
          wl_display_bind(disp, id, &wl_data_device_manager_interface);
     }

   if ((ewd->wl.compositor) && (ewd->wl.shm) && (ewd->wl.shell))
     {
        Ecore_Wl_Event_Interfaces_Bound *ev;

        if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Interfaces_Bound))))
          return;

        ev->compositor = (ewd->wl.compositor != NULL);
        ev->shm = (ewd->wl.shm != NULL);
        ev->shell = (ewd->wl.shell != NULL);

        ecore_event_add(ECORE_WL_EVENT_INTERFACES_BOUND, ev, NULL, NULL);
     }
}

static Eina_Bool 
_ecore_wl_egl_init(Ecore_Wl_Display *ewd)
{
   EGLint major, minor, n;
   static const EGLint context_attribs[] = 
     {
        EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
     };
   static const EGLint argb_attribs[] = 
     {
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, 
        EGL_ALPHA_SIZE, 1, EGL_DEPTH_SIZE, 0, EGL_STENCIL_SIZE, 0, 
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE, 
        EGL_WINDOW_BIT, EGL_NONE
     };

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ewd->egl.display = eglGetDisplay(ewd->wl.display);
   if (!eglInitialize(ewd->egl.display, &major, &minor))
     {
        ERR("Failed to initialize EGL display");
        return EINA_FALSE;
     }

   if (!eglBindAPI(EGL_OPENGL_ES_API))
     {
        ERR("Failed to bind EGL Api");
        return EINA_FALSE;
     }

   if ((!eglChooseConfig(ewd->egl.display, argb_attribs, &ewd->egl.argb_config,
                        1, &n)) || (n == 0))
     {
        ERR("Failed to choose ARGB config");
        return EINA_FALSE;
     }

   ewd->egl.argb_context = 
     eglCreateContext(ewd->egl.display, ewd->egl.argb_config, 
                      EGL_NO_CONTEXT, context_attribs);
   if (!ewd->egl.argb_context)
     {
        ERR("Failed to create ARGB context");
        return EINA_FALSE;
     }

   if (!eglMakeCurrent(ewd->egl.display, EGL_NO_SURFACE, 
                       EGL_NO_SURFACE, ewd->egl.argb_context))
     {
        ERR("Failed to make ARGB context current");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool 
_ecore_wl_egl_shutdown(Ecore_Wl_Display *ewd)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   eglMakeCurrent(ewd->egl.display, 
                  EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

   eglDestroyContext(ewd->egl.display, ewd->egl.argb_context);

   /* NB: This is hanging when we run elm apps as wayland clients 
    * inside the weston compositor */

   /* printf("Egl Terminate\n"); */
   /* eglTerminate(ewd->egl.display); */
   /* printf("Egl Terminate Done\n"); */

   eglReleaseThread();

   return EINA_TRUE;
}

static Eina_Bool 
_ecore_wl_xkb_init(Ecore_Wl_Display *ewd)
{
   struct xkb_rule_names names;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   names.rules = "evdev";
   names.model = "evdev";
   names.layout = "us";
   names.variant = "";
   names.options = "";

   if (!(ewd->xkb = xkb_compile_keymap_from_rules(&names)))
     {
        ERR("Failed to compile keymap");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool 
_ecore_wl_xkb_shutdown(Ecore_Wl_Display *ewd)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ewd->xkb) xkb_free_keymap(ewd->xkb);
   return EINA_TRUE;
}
