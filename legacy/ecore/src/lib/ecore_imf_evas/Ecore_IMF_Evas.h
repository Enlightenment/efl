/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef _ECORE_IMF_EVAS_H
#define _ECORE_IMF_EVAS_H

#include <Ecore_IMF.h>
#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_IMF_BUILD
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

#ifdef __cplusplus
extern "C" {
#endif

   EAPI void ecore_imf_evas_event_mouse_in_wrap(Evas_Event_Mouse_In *evas_event, Ecore_IMF_Event_Mouse_In *imf_event);
   EAPI void ecore_imf_evas_event_mouse_out_wrap(Evas_Event_Mouse_Out *evas_event, Ecore_IMF_Event_Mouse_Out *imf_event);
   EAPI void ecore_imf_evas_event_mouse_move_wrap(Evas_Event_Mouse_Move *evas_event, Ecore_IMF_Event_Mouse_Move *imf_event);
   EAPI void ecore_imf_evas_event_mouse_down_wrap(Evas_Event_Mouse_Down *evas_event, Ecore_IMF_Event_Mouse_Down *imf_event);
   EAPI void ecore_imf_evas_event_mouse_up_wrap(Evas_Event_Mouse_Up *evas_event, Ecore_IMF_Event_Mouse_Up *imf_event);
   EAPI void ecore_imf_evas_event_mouse_wheel_wrap(Evas_Event_Mouse_Wheel *evas_event, Ecore_IMF_Event_Mouse_Wheel *imf_event);
   EAPI void ecore_imf_evas_event_key_down_wrap(Evas_Event_Key_Down *evas_event, Ecore_IMF_Event_Key_Down *imf_event);
   EAPI void ecore_imf_evas_event_key_up_wrap(Evas_Event_Key_Up *evas_event, Ecore_IMF_Event_Key_Up *imf_event);

#ifdef __cplusplus
}
#endif

#endif
