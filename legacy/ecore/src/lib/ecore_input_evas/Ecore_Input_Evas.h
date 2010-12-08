#ifndef _ECORE_INPUT_EVAS_H
#define _ECORE_INPUT_EVAS_H

#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_INPUT_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_INPUT_EVAS_BUILD */
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

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*Ecore_Event_Mouse_Move_Cb)(void *window, int x, int y, unsigned int timestamp);
typedef void (*Ecore_Event_Multi_Move_Cb)(void *window, int device, int x, int y, double radius, double radius_x, double radius_y, double pressure, double angle, double mx, double my, unsigned int timestamp);
typedef void (*Ecore_Event_Multi_Down_Cb)(void *window, int device, int x, int y, double radius, double radius_x, double radius_y, double pressure, double angle, double mx, double my, Evas_Button_Flags flags, unsigned int timestamp);
typedef void (*Ecore_Event_Multi_Up_Cb)(void *window, int device, int x, int y, double radius, double radius_x, double radius_y, double pressure, double angle, double mx, double my, Evas_Button_Flags flags, unsigned int timestamp);
      
EAPI int       ecore_event_evas_init(void);
EAPI int       ecore_event_evas_shutdown(void);

EAPI Eina_Bool ecore_event_evas_key_down(void *data, int type, void *event);
EAPI Eina_Bool ecore_event_evas_key_up(void *data, int type, void *event);
EAPI Eina_Bool ecore_event_evas_mouse_button_up(void *data, int type, void *event);
EAPI Eina_Bool ecore_event_evas_mouse_button_down(void *data, int type, void *event);
EAPI Eina_Bool ecore_event_evas_mouse_wheel(void *data, int type, void *event);
EAPI Eina_Bool ecore_event_evas_mouse_move(void *data, int type, void *event);
EAPI Eina_Bool ecore_event_evas_mouse_in(void *data, int type, void *event);
EAPI Eina_Bool ecore_event_evas_mouse_out(void *data, int type, void *event);

EAPI void      ecore_event_window_register(Ecore_Window id, void *window, Evas *evas, Ecore_Event_Mouse_Move_Cb move_mouse, Ecore_Event_Multi_Move_Cb move_multi, Ecore_Event_Multi_Down_Cb down_multi, Ecore_Event_Multi_Up_Cb up_multi);
EAPI void      ecore_event_window_unregister(Ecore_Window id);
EAPI void     *ecore_event_window_match(Ecore_Window id);
EAPI void      ecore_event_window_ignore_events(Ecore_Window id, int ignore_event);

EAPI void      ecore_event_evas_modifier_lock_update(Evas *e, unsigned int modifiers);

#ifdef __cplusplus
}
#endif

#endif
