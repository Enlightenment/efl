#ifndef _ECORE_INPUT_EVAS_H
#define _ECORE_INPUT_EVAS_H

#include <Evas.h>

#include <ecore_input_evas_api.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*Ecore_Event_Mouse_Move_Cb)(void *window, int x, int y, unsigned int timestamp);
typedef void (*Ecore_Event_Multi_Move_Cb)(void *window, int device, int x, int y, double radius, double radius_x, double radius_y, double pressure, double angle, double mx, double my, unsigned int timestamp);
typedef void (*Ecore_Event_Multi_Down_Cb)(void *window, int device, int x, int y, double radius, double radius_x, double radius_y, double pressure, double angle, double mx, double my, Evas_Button_Flags flags, unsigned int timestamp);
typedef void (*Ecore_Event_Multi_Up_Cb)(void *window, int device, int x, int y, double radius, double radius_x, double radius_y, double pressure, double angle, double mx, double my, Evas_Button_Flags flags, unsigned int timestamp);

ECORE_INPUT_EVAS_API int       ecore_event_evas_init(void);
ECORE_INPUT_EVAS_API int       ecore_event_evas_shutdown(void);

ECORE_INPUT_EVAS_API Eina_Bool ecore_event_evas_key_down(void *data, int type, void *event);
ECORE_INPUT_EVAS_API Eina_Bool ecore_event_evas_key_up(void *data, int type, void *event);
ECORE_INPUT_EVAS_API Eina_Bool ecore_event_evas_mouse_button_up(void *data, int type, void *event);
ECORE_INPUT_EVAS_API Eina_Bool ecore_event_evas_mouse_button_down(void *data, int type, void *event);
ECORE_INPUT_EVAS_API Eina_Bool ecore_event_evas_mouse_wheel(void *data, int type, void *event);
ECORE_INPUT_EVAS_API Eina_Bool ecore_event_evas_mouse_move(void *data, int type, void *event);
ECORE_INPUT_EVAS_API Eina_Bool ecore_event_evas_mouse_in(void *data, int type, void *event);
ECORE_INPUT_EVAS_API Eina_Bool ecore_event_evas_mouse_out(void *data, int type, void *event);
ECORE_INPUT_EVAS_API Eina_Bool ecore_event_evas_axis_update(void *data, int type, void *event); /**< @since 1.13 */
ECORE_INPUT_EVAS_API Eina_Bool ecore_event_evas_mouse_button_cancel(void *data, int type, void *event); /**< @since 1.15 */

ECORE_INPUT_EVAS_API void      ecore_event_window_register(Ecore_Window id, void *window, Evas *evas, Ecore_Event_Mouse_Move_Cb move_mouse, Ecore_Event_Multi_Move_Cb move_multi, Ecore_Event_Multi_Down_Cb down_multi, Ecore_Event_Multi_Up_Cb up_multi);
ECORE_INPUT_EVAS_API void      ecore_event_window_unregister(Ecore_Window id);
ECORE_INPUT_EVAS_API void     *ecore_event_window_match(Ecore_Window id);
ECORE_INPUT_EVAS_API void      ecore_event_window_ignore_events(Ecore_Window id, int ignore_event);

ECORE_INPUT_EVAS_API void      ecore_event_evas_modifier_lock_update(Evas *e, unsigned int modifiers);

ECORE_INPUT_EVAS_API void ecore_event_evas_seat_modifier_lock_update(Evas *e, unsigned int modifiers,
                                                     Evas_Device *seat); /**< @since 1.19 */

#ifdef ECORE_EVAS_INTERNAL
typedef Eina_Bool (*Ecore_Event_Direct_Input_Cb)(void *window, int type, const void *info);
ECORE_INPUT_EVAS_API void      _ecore_event_window_direct_cb_set(Ecore_Window id, Ecore_Event_Direct_Input_Cb fptr);
#endif

#ifdef __cplusplus
}
#endif

#endif
