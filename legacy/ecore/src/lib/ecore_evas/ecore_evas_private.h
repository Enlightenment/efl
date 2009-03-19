/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef _ECORE_EVAS_PRIVATE_H
#define _ECORE_EVAS_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_private.h"
#include "Ecore_Data.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif

#include <Evas.h>

#define ECORE_MAGIC_EVAS 0x76543211

#ifndef BUILD_ECORE_DIRECTFB
# undef BUILD_ECORE_EVAS_DIRECTFB
#endif

#ifdef BUILD_ECORE_EVAS_X11
# include "Ecore_X.h"
# ifdef HAVE_ECORE_X_XCB
#  include <xcb/xcb.h>
#  ifdef BUILD_ECORE_EVAS_SOFTWARE_XCB
#   include <Evas_Engine_Software_X11.h>
#  endif
#  ifdef BUILD_ECORE_EVAS_XRENDER_XCB
#   include <xcb/render.h>
#   include <Evas_Engine_XRender_X11.h>
#  endif
# endif
# ifdef HAVE_ECORE_X_XLIB
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
#   include <Evas_Engine_Software_X11.h>
#  endif
#  ifdef BUILD_ECORE_EVAS_XRENDER_X11
#   include <X11/extensions/Xrender.h>
#   include <Evas_Engine_XRender_X11.h>
#  endif
#  ifdef BUILD_ECORE_EVAS_OPENGL_X11
#    include <Evas_Engine_GL_X11.h>
#  endif
#  ifdef BUILD_ECORE_EVAS_SOFTWARE_16_X11
#   include <Evas_Engine_Software_16_X11.h>
#  endif
# endif
#endif

#ifdef BUILD_ECORE_EVAS_FB
# include <Evas_Engine_FB.h>
#endif

#ifdef BUILD_ECORE_EVAS_DIRECTFB
# include <Evas_Engine_DirectFB.h>
# include "Ecore_DirectFB.h"
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
# include <Evas_Engine_Buffer.h>
#endif

#ifdef BUILD_ECORE_EVAS_WIN32
# include "Ecore_Win32.h"
# ifdef BUILD_ECORE_EVAS_SOFTWARE_DDRAW
#  include <Evas_Engine_Software_DDraw.h>
# endif
# ifdef BUILD_ECORE_EVAS_DIRECT3D
#  include <Evas_Engine_Direct3D.h>
# endif
# ifdef BUILD_ECORE_EVAS_OPENGL_GLEW
#  include <Evas_Engine_GL_Glew.h>
# endif
# ifdef BUILD_ECORE_EVAS_SOFTWARE_16_DDRAW
#  include <Evas_Engine_Software_16_DDraw.h>
# endif
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE
# include "Ecore_WinCE.h"
# include <Evas_Engine_Software_16_WinCE.h>
#endif


#define IDLE_FLUSH_TIME 0.5
#ifndef _ECORE_EVAS_H
typedef struct _Ecore_Evas Ecore_Evas;
#endif

typedef struct _Ecore_Evas_Engine      Ecore_Evas_Engine;
typedef struct _Ecore_Evas_Engine_Func Ecore_Evas_Engine_Func;

struct _Ecore_Evas_Engine_Func
{
   void        (*fn_free) (Ecore_Evas *ee);
   void        (*fn_callback_resize_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_move_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_show_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_hide_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_delete_request_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_destroy_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_focus_in_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_focus_out_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_mouse_in_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_mouse_out_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_sticky_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_unsticky_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_pre_render_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_post_render_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_move) (Ecore_Evas *ee, int x, int y);
   void        (*fn_managed_move) (Ecore_Evas *ee, int x, int y);
   void        (*fn_resize) (Ecore_Evas *ee, int w, int h);
   void        (*fn_move_resize) (Ecore_Evas *ee, int x, int y, int w, int h);
   void        (*fn_rotation_set) (Ecore_Evas *ee, int rot);
   void        (*fn_shaped_set) (Ecore_Evas *ee, int shaped);
   void        (*fn_show) (Ecore_Evas *ee);
   void        (*fn_hide) (Ecore_Evas *ee);
   void        (*fn_raise) (Ecore_Evas *ee);
   void        (*fn_lower) (Ecore_Evas *ee);
   void        (*fn_activate) (Ecore_Evas *ee);
   void        (*fn_title_set) (Ecore_Evas *ee, const char *t);
   void        (*fn_name_class_set) (Ecore_Evas *ee, const char *n, const char *c);
   void        (*fn_size_min_set) (Ecore_Evas *ee, int w, int h);
   void        (*fn_size_max_set) (Ecore_Evas *ee, int w, int h);
   void        (*fn_size_base_set) (Ecore_Evas *ee, int w, int h);
   void        (*fn_size_step_set) (Ecore_Evas *ee, int w, int h);
   void        (*fn_object_cursor_set) (Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y);
   void        (*fn_layer_set) (Ecore_Evas *ee, int layer);
   void        (*fn_focus_set) (Ecore_Evas *ee, int on);
   void        (*fn_iconified_set) (Ecore_Evas *ee, int on);
   void        (*fn_borderless_set) (Ecore_Evas *ee, int on);
   void        (*fn_override_set) (Ecore_Evas *ee, int on);
   void        (*fn_maximized_set) (Ecore_Evas *ee, int on);
   void        (*fn_fullscreen_set) (Ecore_Evas *ee, int on);
   void        (*fn_avoid_damage_set) (Ecore_Evas *ee, int on);
   void        (*fn_withdrawn_set) (Ecore_Evas *ee, int withdrawn);
   void        (*fn_sticky_set) (Ecore_Evas *ee, int sticky);
   void        (*fn_ignore_events_set) (Ecore_Evas *ee, int ignore);
   void        (*fn_alpha_set) (Ecore_Evas *ee, int alpha);
};

struct _Ecore_Evas_Engine
{
   Ecore_Evas_Engine_Func *func;

#if defined (BUILD_ECORE_EVAS_SOFTWARE_X11) || defined (BUILD_ECORE_EVAS_SOFTWARE_XCB)
   struct {
      Ecore_X_Window win_root;
      Eina_List     *win_extra;
      Ecore_X_Pixmap pmap;
      Ecore_X_Pixmap mask;
      Ecore_X_GC     gc;
#ifdef BUILD_ECORE_EVAS_SOFTWARE_XCB
# warning [XCB] No Region code
#else
      Region         damages;
#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_XCB */
      int            px, py, pw, ph;
      unsigned char  direct_resize : 1;
      unsigned char  using_bg_pixmap : 1;
      unsigned char  managed : 1;
      struct {
	   unsigned char modal : 1;
	   unsigned char sticky : 1;
	   unsigned char maximized_v : 1;
	   unsigned char maximized_h : 1;
	   unsigned char shaded : 1;
	   unsigned char skip_taskbar : 1;
	   unsigned char skip_pager : 1;
	   unsigned char fullscreen : 1;
	   unsigned char above : 1;
	   unsigned char below : 1;
      } state;
   } x;
#endif
#ifdef BUILD_ECORE_EVAS_FB
   struct {
      int real_w;
      int real_h;
   } fb;
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
   struct {
      void *pixels;
      Evas_Object *image;
   } buffer;
#endif
#ifdef BUILD_ECORE_EVAS_DIRECTFB
   struct {
      Ecore_DirectFB_Window *window;
   } directfb;
#endif
#ifdef BUILD_ECORE_EVAS_WIN32
   struct {
      Ecore_Win32_Window *parent;
      Ecore_Win32_Window *window;
     struct {
       unsigned char fullscreen : 1;
     } state;
   } win32;
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE
   struct {
      Ecore_WinCE_Window *window;
     struct {
       unsigned char fullscreen : 1;
     } state;
   } wince;
#endif

   Ecore_Timer *idle_flush_timer;
};

struct _Ecore_Evas
{
   Ecore_List  __list_data;
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

   Eina_Hash  *data;

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
	 int          w, h;
      } min,
	max,
	base,
	step;
      struct {
	 Evas_Object *object;
	 int          layer;
	 struct {
	    int       x, y;
	 } hot;
      } cursor;
      int             layer;
      Ecore_Window    window;
      unsigned char   avoid_damage;
      char            focused      : 1;
      char            iconified    : 1;
      char            borderless   : 1;
      char            override     : 1;
      char            maximized    : 1;
      char            fullscreen   : 1;
      char            withdrawn    : 1;
      char            sticky       : 1;
      char            request_pos  : 1;
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
   } func;

   Ecore_Evas_Engine engine;
   Eina_List *sub_ecore_evas;

   unsigned char ignore_events : 1;
};

#ifdef BUILD_ECORE_EVAS_X11
int _ecore_evas_x_shutdown(void);
#endif
#ifdef BUILD_ECORE_EVAS_FB
int _ecore_evas_fb_shutdown(void);
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
int _ecore_evas_buffer_shutdown(void);
void _ecore_evas_buffer_render(Ecore_Evas *ee);
#endif
#ifdef BUILD_ECORE_EVAS_DIRECTFB
int _ecore_evas_directfb_shutdown(void);
#endif
#ifdef BUILD_ECORE_EVAS_WIN32
int _ecore_evas_win32_shutdown(void);
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE
int _ecore_evas_wince_shutdown(void);
#endif

void _ecore_evas_fps_debug_init(void);
void _ecore_evas_fps_debug_shutdown(void);
void _ecore_evas_fps_debug_rendertime_add(double t);
void _ecore_evas_free(Ecore_Evas *ee);
void _ecore_evas_idle_timeout_update(Ecore_Evas *ee);
void _ecore_evas_mouse_move_process(void *ecore_evas, int x, int y, unsigned int timestamp);

#endif
