#ifndef __ECORE_COCOA_H__
#define __ECORE_COCOA_H__

/*
 * DO NOT USE THIS HEADER. IT IS WORK IN PROGRESS. IT IS NOT FINAL AND
 * THE API MAY CHANGE.
 */

#ifndef ECORE_COCOA_WIP_GNSIDNQI
# warning "You are using a work in progress API. This API is not stable"
# warning "and is subject to change. You use this at your own risk."
#endif

#ifdef EAPI
# undef EAPI
#endif

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#else
# define EAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Ecore_Cocoa_Window Ecore_Cocoa_Window;
typedef struct _Ecore_Cocoa_Screen Ecore_Cocoa_Screen;

EAPI extern int ECORE_COCOA_EVENT_GOT_FOCUS;
EAPI extern int ECORE_COCOA_EVENT_LOST_FOCUS;
EAPI extern int ECORE_COCOA_EVENT_RESIZE;
EAPI extern int ECORE_COCOA_EVENT_EXPOSE;

typedef struct _Ecore_Cocoa_Event_Video_Resize Ecore_Cocoa_Event_Video_Resize;
struct _Ecore_Cocoa_Event_Video_Resize
{
   int             w;
   int             h;
};

struct _Ecore_Cocoa_Screen
{
  int dummy;
};

/* Core */

EAPI int  ecore_cocoa_init(void);
EAPI int  ecore_cocoa_shutdown(void);
EAPI void ecore_cocoa_feed_events(void);

/* Screen */

EAPI void ecore_cocoa_screen_size_get(Ecore_Cocoa_Screen *screen, int *w, int *h);

/* Window */

EAPI Ecore_Cocoa_Window *ecore_cocoa_window_new(int x,
                                                int y,
                                                int width,
                                                int height);

EAPI void ecore_cocoa_window_free(Ecore_Cocoa_Window *window);

EAPI void *ecore_cocoa_window_hwnd_get(Ecore_Cocoa_Window *window);

EAPI void ecore_cocoa_window_move(Ecore_Cocoa_Window *window,
                                  int                 x,
                                  int                 y);

EAPI void ecore_cocoa_window_resize(Ecore_Cocoa_Window *window,
                                    int                 width,
                                    int                 height);

EAPI void ecore_cocoa_window_move_resize(Ecore_Cocoa_Window *window,
                                         int                 x,
                                         int                 y,
                                         int                 width,
                                         int                 height);

EAPI void ecore_cocoa_window_geometry_get(Ecore_Cocoa_Window *window,
                                          int                *x,
                                          int                *y,
                                          int                *width,
                                          int                *height);

EAPI void ecore_cocoa_window_size_get(Ecore_Cocoa_Window *window,
                                      int                *width,
                                      int                *height);

EAPI void ecore_cocoa_window_size_min_set(Ecore_Cocoa_Window *window,
                                          unsigned int        min_width,
                                          unsigned int        min_height);

EAPI void ecore_cocoa_window_size_min_get(Ecore_Cocoa_Window *window,
                                          unsigned int       *min_width,
                                          unsigned int       *min_height);

EAPI void ecore_cocoa_window_size_max_set(Ecore_Cocoa_Window *window,
                                          unsigned int        max_width,
                                          unsigned int        max_height);

EAPI void ecore_cocoa_window_size_max_get(Ecore_Cocoa_Window *window,
                                          unsigned int       *max_width,
                                          unsigned int       *max_height);

EAPI void ecore_cocoa_window_size_base_set(Ecore_Cocoa_Window *window,
                                           unsigned int        base_width,
                                           unsigned int        base_height);

EAPI void ecore_cocoa_window_size_base_get(Ecore_Cocoa_Window *window,
                                           unsigned int       *base_width,
                                           unsigned int       *base_height);

EAPI void ecore_cocoa_window_size_step_set(Ecore_Cocoa_Window *window,
                                           unsigned int        step_width,
                                           unsigned int        step_height);

EAPI void ecore_cocoa_window_size_step_get(Ecore_Cocoa_Window *window,
                                           unsigned int       *step_width,
                                           unsigned int       *step_height);

EAPI void ecore_cocoa_window_show(Ecore_Cocoa_Window *window);

EAPI void ecore_cocoa_window_hide(Ecore_Cocoa_Window *window);

EAPI void ecore_cocoa_window_raise(Ecore_Cocoa_Window *window);

EAPI void ecore_cocoa_window_lower(Ecore_Cocoa_Window *window);

EAPI void ecore_cocoa_window_title_set(Ecore_Cocoa_Window *window,
                                       const char         *title);

EAPI void ecore_cocoa_window_focus_set(Ecore_Cocoa_Window *window);

EAPI void ecore_cocoa_window_iconified_set(Ecore_Cocoa_Window *window,
                                           int                 on);

EAPI void ecore_cocoa_window_borderless_set(Ecore_Cocoa_Window *window,
                                            int                 on);

EAPI void ecore_cocoa_window_view_set(Ecore_Cocoa_Window *window,
				      void *view);

EAPI int ecore_cocoa_titlebar_height_get(void);


#ifdef __cplusplus
}
#endif

#endif
