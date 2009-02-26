/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Eina.h>

#include "Ecore.h"
#include "Ecore_WinCE.h"
#include "ecore_wince_private.h"


/***** Private declarations *****/

static Ecore_WinCE_Window *_ecore_wince_mouse_down_last_window = NULL;
static Ecore_WinCE_Window *_ecore_wince_mouse_down_last_last_window = NULL;
static double              _ecore_wince_mouse_down_last_time = 0;
static double              _ecore_wince_mouse_down_last_last_time = 0;
static int                 _ecore_wince_mouse_down_did_triple = 0;
static int                 _ecore_wince_mouse_up_count = 0;


static void _ecore_wince_event_free_key_down(void *data,
                                             void *ev);

static void _ecore_wince_event_free_key_up(void *data,
                                           void *ev);

static int  _ecore_wince_event_keystroke_get(int    key,
                                             char **keyname,
                                             char **keysymbol,
                                             char **keycompose);

static int  _ecore_wince_event_char_get(int    key,
                                        char **keyname,
                                        char **keysymbol,
                                        char **keycompose);


/***** Global functions *****/

void
_ecore_wince_event_handle_key_press(Ecore_WinCE_Callback_Data *msg,
                                    int                        is_keystroke)
{
   Ecore_WinCE_Event_Key_Down *e;

   EINA_ERROR_PINFO("key pressed\n");

   e = (Ecore_WinCE_Event_Key_Down *)malloc(sizeof(Ecore_WinCE_Event_Key_Down));
   if (!e) return;

   if (is_keystroke)
     {
        if (!_ecore_wince_event_keystroke_get(LOWORD(msg->window_param),
                                              &e->keyname,
                                              &e->keysymbol,
                                              &e->keycompose))
          {
             free(e);
             return;
          }
     }
   else
     {
        if (!_ecore_wince_event_char_get(LOWORD(msg->window_param),
                                         &e->keyname,
                                         &e->keysymbol,
                                         &e->keycompose))
          {
             free(e);
             return;
          }
     }

   e->window = (void *)GetWindowLong(msg->window, GWL_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }
   e->time = (double)msg->time / 1000.0;

   _ecore_wince_event_last_time = e->time;

   ecore_event_add(ECORE_WINCE_EVENT_KEY_DOWN, e, _ecore_wince_event_free_key_down, NULL);
}

void
_ecore_wince_event_handle_key_release(Ecore_WinCE_Callback_Data *msg,
                                      int                        is_keystroke)
{
   Ecore_WinCE_Event_Key_Up *e;

   EINA_ERROR_PINFO("key released\n");

   e = (Ecore_WinCE_Event_Key_Up *)calloc(1, sizeof(Ecore_WinCE_Event_Key_Up));
   if (!e) return;

   if (is_keystroke)
     {
        if (!_ecore_wince_event_keystroke_get(LOWORD(msg->window_param),
                                              &e->keyname,
                                              &e->keysymbol,
                                              &e->keycompose))
          {
             free(e);
             return;
          }
     }
   else
     {
        if (!_ecore_wince_event_char_get(LOWORD(msg->window_param),
                                         &e->keyname,
                                         &e->keysymbol,
                                         &e->keycompose))
          {
             free(e);
             return;
          }
     }

   e->window = (void *)GetWindowLong(msg->window, GWL_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }
   e->time = (double)msg->time / 1000.0;

   _ecore_wince_event_last_time = e->time;

   ecore_event_add(ECORE_WINCE_EVENT_KEY_UP, e, _ecore_wince_event_free_key_up, NULL);
}

void
_ecore_wince_event_handle_button_press(Ecore_WinCE_Callback_Data *msg,
                                       int                        button)
{
   Ecore_WinCE_Window *window;

   EINA_ERROR_PINFO("mouse button pressed\n");

   window = (void *)GetWindowLong(msg->window, GWL_USERDATA);

   {
      Ecore_WinCE_Event_Mouse_Move *e;

      e = (Ecore_WinCE_Event_Mouse_Move *)calloc(1, sizeof(Ecore_WinCE_Event_Mouse_Move));
      if (!e) return;

      e->window = window;
      e->x = LOWORD(msg->data_param);
      e->y = HIWORD(msg->data_param);
      e->time = (double)msg->time / 1000.0;

      _ecore_wince_event_last_time = e->time;
      _ecore_wince_event_last_window = e->window;

      ecore_event_add(ECORE_WINCE_EVENT_MOUSE_MOVE, e, NULL, NULL);
   }

   {
      Ecore_WinCE_Event_Mouse_Button_Down *e;

      if (_ecore_wince_mouse_down_did_triple)
        {
           _ecore_wince_mouse_down_last_window = NULL;
           _ecore_wince_mouse_down_last_last_window = NULL;
           _ecore_wince_mouse_down_last_time = 0.0;
           _ecore_wince_mouse_down_last_last_time = 0.0;
        }

      e = (Ecore_WinCE_Event_Mouse_Button_Down *)calloc(1, sizeof(Ecore_WinCE_Event_Mouse_Button_Down));
      if (!e) return;

      e->window = window;
      e->button = button;
      e->x = LOWORD(msg->data_param);
      e->y = HIWORD(msg->data_param);
      e->time = (double)msg->time / 1000.0;

      if (((e->time - _ecore_wince_mouse_down_last_time) <= _ecore_wince_double_click_time) &&
          (e->window == _ecore_wince_mouse_down_last_window))
        e->double_click = 1;

      if (((e->time - _ecore_wince_mouse_down_last_last_time) <= (2.0 * _ecore_wince_double_click_time)) &&
          (e->window == _ecore_wince_mouse_down_last_window) &&
          (e->window == _ecore_wince_mouse_down_last_last_window))
        {
           e->triple_click = 1;
           _ecore_wince_mouse_down_did_triple = 1;
        }
      else
        _ecore_wince_mouse_down_did_triple = 0;

      if (!e->double_click && !e->triple_click)
        _ecore_wince_mouse_up_count = 0;

      _ecore_wince_event_last_time = e->time;
      _ecore_wince_event_last_window = e->window;

      if (!_ecore_wince_mouse_down_did_triple)
        {
           _ecore_wince_mouse_down_last_last_window = _ecore_wince_mouse_down_last_window;
           _ecore_wince_mouse_down_last_window = e->window;
           _ecore_wince_mouse_down_last_last_time = _ecore_wince_mouse_down_last_time;
           _ecore_wince_mouse_down_last_time = e->time;
        }

      ecore_event_add(ECORE_WINCE_EVENT_MOUSE_BUTTON_DOWN, e, NULL, NULL);
   }
}

void
_ecore_wince_event_handle_button_release(Ecore_WinCE_Callback_Data *msg,
                                         int                          button)
{
   Ecore_WinCE_Window *window;

   EINA_ERROR_PINFO("mouse button released\n");

   window = (void *)GetWindowLong(msg->window, GWL_USERDATA);

   {
      Ecore_WinCE_Event_Mouse_Move *e;

      e = (Ecore_WinCE_Event_Mouse_Move *)calloc(1, sizeof(Ecore_WinCE_Event_Mouse_Move));
      if (!e) return;

      e->window = window;
      e->x = LOWORD(msg->data_param);
      e->y = HIWORD(msg->data_param);
      e->time = (double)msg->time / 1000.0;

      _ecore_wince_event_last_time = e->time;
      _ecore_wince_event_last_window = e->window;

      ecore_event_add(ECORE_WINCE_EVENT_MOUSE_MOVE, e, NULL, NULL);
   }

   {
      Ecore_WinCE_Event_Mouse_Button_Up *e;

      e = (Ecore_WinCE_Event_Mouse_Button_Up *)calloc(1, sizeof(Ecore_WinCE_Event_Mouse_Button_Up));
      if (!e) return;

      e->window = window;
      e->button = button;
      e->x = LOWORD(msg->data_param);
      e->y = HIWORD(msg->data_param);
      e->time = (double)msg->time / 1000.0;

      _ecore_wince_mouse_up_count++;

      if ((_ecore_wince_mouse_up_count >= 2) &&
          ((e->time - _ecore_wince_mouse_down_last_time) <= _ecore_wince_double_click_time) &&
          (e->window == _ecore_wince_mouse_down_last_window))
        e->double_click = 1;

      if ((_ecore_wince_mouse_up_count >= 3) &&
          ((e->time - _ecore_wince_mouse_down_last_last_time) <= (2.0 * _ecore_wince_double_click_time)) &&
          (e->window == _ecore_wince_mouse_down_last_window) &&
          (e->window == _ecore_wince_mouse_down_last_last_window))
        e->triple_click = 1;

      _ecore_wince_event_last_time = e->time;
      _ecore_wince_event_last_window = e->window;

      ecore_event_add(ECORE_WINCE_EVENT_MOUSE_BUTTON_UP, e, NULL, NULL);
   }
}

void
_ecore_wince_event_handle_motion_notify(Ecore_WinCE_Callback_Data *msg)
{
   Ecore_WinCE_Event_Mouse_Move *e;

   EINA_ERROR_PINFO("mouse moved\n");

   e = (Ecore_WinCE_Event_Mouse_Move *)calloc(1, sizeof(Ecore_WinCE_Event_Mouse_Move));
   if (!e) return;

   e->window = (void *)GetWindowLong(msg->window, GWL_USERDATA);
   e->x = LOWORD(msg->data_param);
   e->y = HIWORD(msg->data_param);
   e->time = (double)msg->time / 1000.0;

   ecore_event_add(ECORE_WINCE_EVENT_MOUSE_MOVE, e, NULL, NULL);
}

void
_ecore_wince_event_handle_enter_notify(Ecore_WinCE_Callback_Data *msg)
{
   Ecore_WinCE_Window *window;

   EINA_ERROR_PINFO("mouse in\n");

   window = (void *)GetWindowLong(msg->window, GWL_USERDATA);

   {
      Ecore_WinCE_Event_Mouse_Move *e;

      e = (Ecore_WinCE_Event_Mouse_Move *)calloc(1, sizeof(Ecore_WinCE_Event_Mouse_Move));
      if (!e) return;

      e->window = window;
      e->x = msg->x;
      e->y = msg->y;
      e->time = (double)msg->time / 1000.0;

      _ecore_wince_event_last_time = e->time;
      _ecore_wince_event_last_window = e->window;

      ecore_event_add(ECORE_WINCE_EVENT_MOUSE_MOVE, e, NULL, NULL);
   }

   {
      Ecore_WinCE_Event_Mouse_In *e;

      e = (Ecore_WinCE_Event_Mouse_In *)calloc(1, sizeof(Ecore_WinCE_Event_Mouse_In));
      if (!e) return;

      e->window = window;
      e->x = msg->x;
      e->y = msg->y;
      e->time = (double)msg->time / 1000.0;

      _ecore_wince_event_last_time = e->time;

      ecore_event_add(ECORE_WINCE_EVENT_MOUSE_IN, e, NULL, NULL);
   }
}

void
_ecore_wince_event_handle_leave_notify(Ecore_WinCE_Callback_Data *msg)
{
   Ecore_WinCE_Window *window;

   EINA_ERROR_PINFO("mouse out\n");

   window = (void *)GetWindowLong(msg->window, GWL_USERDATA);

   {
      Ecore_WinCE_Event_Mouse_Move *e;

      e = (Ecore_WinCE_Event_Mouse_Move *)calloc(1, sizeof(Ecore_WinCE_Event_Mouse_Move));
      if (!e) return;

      e->window = window;
      e->x = msg->x;
      e->y = msg->y;
      e->time = (double)msg->time / 1000.0;

      _ecore_wince_event_last_time = e->time;
      _ecore_wince_event_last_window = e->window;

      ecore_event_add(ECORE_WINCE_EVENT_MOUSE_MOVE, e, NULL, NULL);
   }

   {
      Ecore_WinCE_Event_Mouse_Out *e;

      e = (Ecore_WinCE_Event_Mouse_Out *)calloc(1, sizeof(Ecore_WinCE_Event_Mouse_Out));
      if (!e) return;

      e->window = window;
      e->x = msg->x;
      e->y = msg->y;
      e->time = (double)msg->time / 1000.0;

      _ecore_wince_event_last_time = e->time;

      ecore_event_add(ECORE_WINCE_EVENT_MOUSE_OUT, e, NULL, NULL);
   }
}

void
_ecore_wince_event_handle_focus_in(Ecore_WinCE_Callback_Data *msg)
{
   Ecore_WinCE_Event_Window_Focus_In *e;
   struct _Ecore_WinCE_Window        *window;

   EINA_ERROR_PINFO("focus in\n");

   e = (Ecore_WinCE_Event_Window_Focus_In *)calloc(1, sizeof(Ecore_WinCE_Event_Window_Focus_In));
   if (!e) return;

   window = (void *)GetWindowLong(msg->window, GWL_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }

   if (window->resume)
     window->resume(window->backend);

   e->window = window;

   e->time = _ecore_wince_event_last_time;
   _ecore_wince_event_last_time = e->time;

   ecore_event_add(ECORE_WINCE_EVENT_WINDOW_FOCUS_IN, e, NULL, NULL);
}

void
_ecore_wince_event_handle_focus_out(Ecore_WinCE_Callback_Data *msg)
{
   Ecore_WinCE_Event_Window_Focus_Out *e;
   struct _Ecore_WinCE_Window         *window;

   EINA_ERROR_PINFO("focus out\n");

   e = (Ecore_WinCE_Event_Window_Focus_Out *)calloc(1, sizeof(Ecore_WinCE_Event_Window_Focus_Out));
   if (!e) return;

   window = (void *)GetWindowLong(msg->window, GWL_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }
   if (window->suspend)
     window->suspend(window->backend);

   e->window = window;

   e->time = _ecore_wince_event_last_time;
   _ecore_wince_event_last_time = e->time;

   ecore_event_add(ECORE_WINCE_EVENT_WINDOW_FOCUS_OUT, e, NULL, NULL);
}

void
_ecore_wince_event_handle_expose(Ecore_WinCE_Callback_Data *msg)
{
   Ecore_WinCE_Event_Window_Damage *e;

   EINA_ERROR_PINFO("window expose\n");

   e = (Ecore_WinCE_Event_Window_Damage *)calloc(1, sizeof(Ecore_WinCE_Event_Window_Damage));
   if (!e) return;

   e->window = (void *)GetWindowLong(msg->window, GWL_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }

   e->x = msg->update.left;
   e->y = msg->update.top;
   e->width = msg->update.right - msg->update.left;
   e->height = msg->update.bottom - msg->update.top;
   EINA_ERROR_PINFO("window expose size: %dx%d\n", e->width, e->height);

   e->time = _ecore_wince_event_last_time;

   ecore_event_add(ECORE_WINCE_EVENT_WINDOW_DAMAGE, e, NULL, NULL);
}

void
_ecore_wince_event_handle_create_notify(Ecore_WinCE_Callback_Data *msg)
{
   Ecore_WinCE_Event_Window_Create *e;

   EINA_ERROR_PINFO("window create notify\n");

   e = calloc(1, sizeof(Ecore_WinCE_Event_Window_Create));
   if (!e) return;

   e->window = (void *)GetWindowLong(msg->window, GWL_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }

   e->time = _ecore_wince_event_last_time;

   ecore_event_add(ECORE_WINCE_EVENT_WINDOW_CREATE, e, NULL, NULL);
}

void
_ecore_wince_event_handle_destroy_notify(Ecore_WinCE_Callback_Data *msg)
{
   Ecore_WinCE_Event_Window_Destroy *e;

   EINA_ERROR_PINFO("window destroy notify\n");

   e = calloc(1, sizeof(Ecore_WinCE_Event_Window_Destroy));
   if (!e) return;

   e->window = (void *)GetWindowLong(msg->window, GWL_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }

   e->time = _ecore_wince_event_last_time;
/*    if (e->window == _ecore_wince_event_last_window) _ecore_wince_event_last_window = NULL; */

   ecore_event_add(ECORE_WINCE_EVENT_WINDOW_DESTROY, e, NULL, NULL);
}

void
_ecore_wince_event_handle_map_notify(Ecore_WinCE_Callback_Data *msg)
{
   Ecore_WinCE_Event_Window_Show *e;

   EINA_ERROR_PINFO("window map notify\n");

   e = calloc(1, sizeof(Ecore_WinCE_Event_Window_Show));
   if (!e) return;

   e->window = (void *)GetWindowLong(msg->window, GWL_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }

   e->time = _ecore_wince_event_last_time;

   ecore_event_add(ECORE_WINCE_EVENT_WINDOW_SHOW, e, NULL, NULL);
}

void
_ecore_wince_event_handle_unmap_notify(Ecore_WinCE_Callback_Data *msg)
{
   Ecore_WinCE_Event_Window_Hide *e;

   EINA_ERROR_PINFO("window unmap notify\n");

   e = calloc(1, sizeof(Ecore_WinCE_Event_Window_Hide));
   if (!e) return;

   e->window = (void *)GetWindowLong(msg->window, GWL_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }

   e->time = _ecore_wince_event_last_time;

   ecore_event_add(ECORE_WINCE_EVENT_WINDOW_HIDE, e, NULL, NULL);
}

void
_ecore_wince_event_handle_delete_request(Ecore_WinCE_Callback_Data *msg)
{
   Ecore_WinCE_Event_Window_Delete_Request *e;

   EINA_ERROR_PINFO("window delete request\n");

   e = calloc(1, sizeof(Ecore_WinCE_Event_Window_Delete_Request));
   if (!e) return;

   e->window = (void *)GetWindowLong(msg->window, GWL_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }

   e->time = _ecore_wince_event_last_time;

   ecore_event_add(ECORE_WINCE_EVENT_WINDOW_DELETE_REQUEST, e, NULL, NULL);
}


/***** Private functions definitions *****/

static void
_ecore_wince_event_free_key_down(void *data __UNUSED__,
                                 void *ev)
{
   Ecore_WinCE_Event_Key_Down *e;

   e = ev;
   if (e->keyname) free(e->keyname);
   if (e->keysymbol) free(e->keysymbol);
   if (e->keycompose) free(e->keycompose);
   free(e);
}

static void
_ecore_wince_event_free_key_up(void *data __UNUSED__,
                               void *ev)
{
   Ecore_WinCE_Event_Key_Up *e;

   e = ev;
   if (e->keyname) free(e->keyname);
   if (e->keysymbol) free(e->keysymbol);
   if (e->keycompose) free(e->keycompose);
   free(e);
}

static int
_ecore_wince_event_keystroke_get(int    key,
                                 char **keyname,
                                 char **keysymbol,
                                 char **keycompose)
{
  char *kn;
  char *ks;
  char *kc;

  *keyname = NULL;
  *keysymbol = NULL;
  *keycompose = NULL;

   switch (key)
     {
       /* Keystroke */
     case VK_PRIOR:
       kn = "KP_Prior";
       ks = "KP_Prior";
       kc = "";
       break;
     case VK_NEXT:
       kn = "KP_Next";
       ks = "KP_Next";
       kc = "";
       break;
     case VK_END:
       kn = "KP_End";
       ks = "KP_End";
       kc = "";
       break;
     case VK_HOME:
       kn = "KP_Home";
       ks = "KP_Home";
       kc = "";
       break;
     case VK_LEFT:
       kn = "KP_Left";
       ks = "KP_Left";
       kc = "";
       break;
     case VK_UP:
       kn = "KP_Up";
       ks = "KP_Up";
       kc = "";
       break;
     case VK_RIGHT:
       kn = "KP_Right";
       ks = "KP_Right";
       kc = "";
       break;
     case VK_DOWN:
       kn = "KP_Down";
       ks = "KP_Down";
       kc = "";
       break;
     case VK_INSERT:
       kn = "KP_Insert";
       ks = "KP_Insert";
       kc = "";
       break;
     case VK_DELETE:
       kn = "KP_Delete";
       ks = "KP_Delete";
       kc = "";
       break;
     case VK_F1:
       kn = "F1";
       ks = "F1";
       kc = "";
       break;
     case VK_F2:
       kn = "F2";
       ks = "F2";
       kc = "";
       break;
     case VK_F3:
       kn = "F3";
       ks = "F3";
       kc = "";
       break;
     case VK_F4:
       kn = "F4";
       ks = "F4";
       kc = "";
       break;
     case VK_F5:
       kn = "F5";
       ks = "F5";
       kc = "";
       break;
     case VK_F6:
       kn = "F6";
       ks = "F6";
       kc = "";
       break;
     case VK_F7:
       kn = "F7";
       ks = "F7";
       kc = "";
       break;
     case VK_F8:
       kn = "F8";
       ks = "F8";
       kc = "";
       break;
     case VK_F9:
       kn = "F9";
       ks = "F9";
       kc = "";
       break;
     case VK_F10:
       kn = "F10";
       ks = "F10";
       kc = "";
       break;
     case VK_F11:
       kn = "F11";
       ks = "F11";
       kc = "";
       break;
     case VK_F12:
       kn = "F12";
       ks = "F12";
       kc = "";
       break;
     case VK_F13:
       kn = "F13";
       ks = "F13";
       kc = "";
       break;
     case VK_F14:
       kn = "F14";
       ks = "F14";
       kc = "";
       break;
     case VK_F15:
       kn = "F15";
       ks = "F15";
       kc = "";
       break;
     case VK_F16:
       kn = "F16";
       ks = "F16";
       kc = "";
       break;
     case VK_F17:
       kn = "F17";
       ks = "F17";
       kc = "";
       break;
     case VK_F18:
       kn = "F18";
       ks = "F18";
       kc = "";
       break;
     case VK_F19:
       kn = "F19";
       ks = "F19";
       kc = "";
       break;
     case VK_F20:
       kn = "F20";
       ks = "F20";
       kc = "";
       break;
     case VK_F21:
       kn = "F21";
       ks = "F21";
       kc = "";
       break;
     case VK_F22:
       kn = "F22";
       ks = "F22";
       kc = "";
       break;
     case VK_F23:
       kn = "F23";
       ks = "F23";
       kc = "";
       break;
     case VK_F24:
       kn = "F24";
       ks = "F24";
       kc = "";
       break;
     case VK_APPS:
       kn = "Application";
       ks = "Application";
       kc = "";
       break;
     case VK_MENU:
       kn = "Menu";
       ks = "Menu";
       kc = "";
       break;
     default:
       /* other non keystroke characters */
       return 0;
     }
   *keyname = strdup(kn);
   if (!*keyname) return 0;
   *keysymbol = strdup(ks);
   if (!*keysymbol)
     {
        free(*keyname);
        *keyname = NULL;
        return 0;
     }
   *keycompose = strdup(kc);
   if (!*keycompose)
     {
        free(*keyname);
        free(*keysymbol);
        *keyname = NULL;
        *keysymbol = NULL;
        return 0;
     }

   return 1;
}

static int
_ecore_wince_event_char_get(int    key,
                            char **keyname,
                            char **keysymbol,
                            char **keycompose)
{
  char kn[32];
  char ks[32];
  char *kc;

  *keyname = NULL;
  *keysymbol = NULL;
  *keycompose = NULL;

   switch (key)
     {
     case VK_APP3:
     case VK_BACK:
       strncpy(kn, "Backspace", 32);
       strncpy(ks, "Backspace", 32);
       kc = "";
       break;
     case VK_APP4:
     case VK_TAB:
       strncpy(kn, "Tab", 32);
       strncpy(ks, "Tab", 32);
       kc = "";
       break;
     case VK_APP5:
     case 0x0a:
       /* Line feed (Shift + Enter) */
       strncpy(kn, "LineFeed", 32);
       strncpy(ks, "LineFeed", 32);
       kc = "";
       break;
     case VK_APP2:
     case VK_RETURN:
       strncpy(kn, "Return", 32);
       strncpy(ks, "Return", 32);
       kc = "";
       break;
     case VK_APP1:
     case VK_ESCAPE:
       strncpy(kn, "Escape", 32);
       strncpy(ks, "Escape", 32);
       kc = "";
       break;
     default:
       /* displayable characters */
       printf (" * key : %d\n", key);
       kn[0] = (TCHAR)key;
       kn[1] = '\0';
       ks[0] = (TCHAR)key;
       ks[1] = '\0';
       kc = "";
       break;
     }
   *keyname = strdup(kn);
   if (!*keyname) return 0;
   *keysymbol = strdup(ks);
   if (!*keysymbol)
     {
        free(*keyname);
        *keyname = NULL;
        return 0;
     }
   *keycompose = strdup(kc);
   if (!*keycompose)
     {
        free(*keyname);
        free(*keysymbol);
        *keyname = NULL;
        *keysymbol = NULL;
        return 0;
     }

   return 1;
}
