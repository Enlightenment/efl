#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>   /* for printf */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <windowsx.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Input.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"


typedef enum
{
   ECORE_WIN32_KEY_MASK_LSHIFT = 1 << 0,
   ECORE_WIN32_KEY_MASK_RSHIFT = 1 << 1,
   ECORE_WIN32_KEY_MASK_LCONTROL = 1 << 2,
   ECORE_WIN32_KEY_MASK_RCONTROL = 1 << 3,
   ECORE_WIN32_KEY_MASK_LMENU = 1 << 4,
   ECORE_WIN32_KEY_MASK_RMENU = 1 << 5
} Ecore_Win32_Key_Mask;

/***** Private declarations *****/


static Ecore_Win32_Window  *_ecore_win32_mouse_down_last_window = NULL;
static Ecore_Win32_Window  *_ecore_win32_mouse_down_last_last_window = NULL;
static long                 _ecore_win32_mouse_down_last_time = 0  ;
static long                 _ecore_win32_mouse_down_last_last_time = 0  ;
static int                  _ecore_win32_mouse_down_did_triple = 0;
static int                  _ecore_win32_mouse_up_count = 0;
static Ecore_Win32_Key_Mask _ecore_win32_key_mask = 0;

static void _ecore_win32_event_free_key_down(void *data,
                                             void *ev);

static void _ecore_win32_event_free_key_up(void *data,
                                           void *ev);

static int  _ecore_win32_event_keystroke_get(Ecore_Win32_Callback_Data *msg,
                                             Eina_Bool                  is_down,
                                             char                     **keyname,
                                             char                     **keysymbol,
                                             char                     **keycompose,
                                             unsigned int              *modifiers);

static int  _ecore_win32_event_char_get(int           key,
                                        char        **keyname,
                                        char        **keysymbol,
                                        char        **keycompose,
                                        unsigned int *modifiers);


/***** Global functions definitions *****/

void
_ecore_win32_event_handle_key_press(Ecore_Win32_Callback_Data *msg,
                                    int                        is_keystroke)
{
   Ecore_Event_Key *e;

   INF("key pressed");

   e = (Ecore_Event_Key *)calloc(1, sizeof(Ecore_Event_Key));
   if (!e) return;

   if (is_keystroke)
     {
        if (!_ecore_win32_event_keystroke_get(msg,
                                              EINA_TRUE,
                                              (char **)&e->keyname,
                                              (char **)&e->key,
                                              (char **)&e->string,
                                              &e->modifiers))
          {
             free(e);
             return;
          }
     }
   else
     {
        if (!_ecore_win32_event_char_get(LOWORD(msg->window_param),
                                         (char **)&e->keyname,
                                         (char **)&e->key,
                                         (char **)&e->string,
                                         &e->modifiers))
          {
             free(e);
             return;
          }
     }

   e->window = (Ecore_Window)GetWindowLongPtr(msg->window, GWLP_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }
   e->event_window = e->window;
   e->timestamp = msg->timestamp;

   _ecore_win32_event_last_time = e->timestamp;

   ecore_event_add(ECORE_EVENT_KEY_DOWN, e, _ecore_win32_event_free_key_down, NULL);
}

void
_ecore_win32_event_handle_key_release(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Event_Key *e;

   INF("key released");

   e = (Ecore_Event_Key *)calloc(1, sizeof(Ecore_Event_Key));
   if (!e) return;

   if (!_ecore_win32_event_keystroke_get(msg,
                                         EINA_FALSE,
                                         (char **)&e->keyname,
                                         (char **)&e->key,
                                         (char **)&e->string,
                                         &e->modifiers))
     {
        if (msg->discard_ctrl ||
            !_ecore_win32_event_char_get(LOWORD(msg->window_param),
                                         (char **)&e->keyname,
                                         (char **)&e->key,
                                         (char **)&e->string,
                                         &e->modifiers))
          {
             free(e);
             return;
          }
     }

   e->window = (Ecore_Window)GetWindowLongPtr(msg->window, GWLP_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }
   e->event_window = e->window;
   e->timestamp = msg->timestamp;

   _ecore_win32_event_last_time = e->timestamp;

   ecore_event_add(ECORE_EVENT_KEY_UP, e, _ecore_win32_event_free_key_up, NULL);
}

void
_ecore_win32_event_handle_button_press(Ecore_Win32_Callback_Data *msg,
                                       int                        button)
{
   Ecore_Win32_Window *window;

   INF("mouse button pressed");

   window = (Ecore_Win32_Window *)GetWindowLongPtr(msg->window, GWLP_USERDATA);

   if (button > 3)
     {
        Ecore_Event_Mouse_Wheel *e;

        e = (Ecore_Event_Mouse_Wheel *)calloc(1, sizeof(Ecore_Event_Mouse_Wheel));
        if (!e) return;

        e->window = (Ecore_Window)window;
        e->event_window = e->window;
        e->direction = 0;
        /* wheel delta is positive or negative, never 0 */
        e->z = GET_WHEEL_DELTA_WPARAM(msg->window_param) > 0 ? -1 : 1;
        e->x = GET_X_LPARAM(msg->data_param);
        e->y = GET_Y_LPARAM(msg->data_param);
        e->timestamp = msg->timestamp;

        _ecore_win32_event_last_time = e->timestamp;
        _ecore_win32_event_last_window = (Ecore_Win32_Window *)e->window;

        ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, e, NULL, NULL);
     }
   else
     {
       {
          Ecore_Event_Mouse_Move *e;

          e = (Ecore_Event_Mouse_Move *)calloc(1, sizeof(Ecore_Event_Mouse_Move));
          if (!e) return;

          e->window = (Ecore_Window)window;
          e->event_window = e->window;
          e->x = GET_X_LPARAM(msg->data_param);
          e->y = GET_Y_LPARAM(msg->data_param);
          e->timestamp = msg->timestamp;

          _ecore_win32_event_last_time = e->timestamp;
          _ecore_win32_event_last_window = (Ecore_Win32_Window *)e->window;

          ecore_event_add(ECORE_EVENT_MOUSE_MOVE, e, NULL, NULL);
       }

       {
          Ecore_Event_Mouse_Button *e;

          if (_ecore_win32_mouse_down_did_triple)
            {
               _ecore_win32_mouse_down_last_window = NULL;
               _ecore_win32_mouse_down_last_last_window = NULL;
               _ecore_win32_mouse_down_last_time = 0;
               _ecore_win32_mouse_down_last_last_time = 0;
            }

          e = (Ecore_Event_Mouse_Button *)calloc(1, sizeof(Ecore_Event_Mouse_Button));
          if (!e) return;

          e->window = (Ecore_Window)window;
          e->event_window = e->window;
          e->buttons = button;
          e->x = GET_X_LPARAM(msg->data_param);
          e->y = GET_Y_LPARAM(msg->data_param);
          e->timestamp = msg->timestamp;

          if (((e->timestamp - _ecore_win32_mouse_down_last_time) <= (unsigned long)(1000 * _ecore_win32_double_click_time)) &&
              (e->window == (Ecore_Window)_ecore_win32_mouse_down_last_window))
            e->double_click = 1;

          if (((e->timestamp - _ecore_win32_mouse_down_last_last_time) <= (unsigned long)(2 * 1000 * _ecore_win32_double_click_time)) &&
              (e->window == (Ecore_Window)_ecore_win32_mouse_down_last_window) &&
              (e->window == (Ecore_Window)_ecore_win32_mouse_down_last_last_window))
            {
               e->triple_click = 1;
               _ecore_win32_mouse_down_did_triple = 1;
            }
          else
            _ecore_win32_mouse_down_did_triple = 0;

          if (!e->double_click && !e->triple_click)
            _ecore_win32_mouse_up_count = 0;

          _ecore_win32_event_last_time = e->timestamp;
          _ecore_win32_event_last_window = (Ecore_Win32_Window *)e->window;

          if (!_ecore_win32_mouse_down_did_triple)
            {
               _ecore_win32_mouse_down_last_last_window = _ecore_win32_mouse_down_last_window;
               _ecore_win32_mouse_down_last_window = (Ecore_Win32_Window *)e->window;
               _ecore_win32_mouse_down_last_last_time = _ecore_win32_mouse_down_last_time;
               _ecore_win32_mouse_down_last_time = e->timestamp;
            }

          ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, e, NULL, NULL);
       }
     }
}

void
_ecore_win32_event_handle_button_release(Ecore_Win32_Callback_Data *msg,
                                         int                        button)
{
   Ecore_Win32_Window *window;

   INF("mouse button released");

   window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);

   {
      Ecore_Event_Mouse_Move *e;

      e = (Ecore_Event_Mouse_Move *)calloc(1, sizeof(Ecore_Event_Mouse_Move));
      if (!e) return;

      e->window = (Ecore_Window)window;
      e->event_window = e->window;
      e->x = GET_X_LPARAM(msg->data_param);
      e->y = GET_Y_LPARAM(msg->data_param);
      e->timestamp = msg->timestamp;

      _ecore_win32_event_last_time = e->timestamp;
      _ecore_win32_event_last_window = (Ecore_Win32_Window *)e->window;

      ecore_event_add(ECORE_EVENT_MOUSE_MOVE, e, NULL, NULL);
   }

   {
      Ecore_Event_Mouse_Button *e;

      e = (Ecore_Event_Mouse_Button *)calloc(1, sizeof(Ecore_Event_Mouse_Button));
      if (!e) return;

      e->window = (Ecore_Window)window;
      e->event_window = e->window;
      e->buttons = button;
      e->x = GET_X_LPARAM(msg->data_param);
      e->y = GET_Y_LPARAM(msg->data_param);
      e->timestamp = msg->timestamp;

      _ecore_win32_mouse_up_count++;

      if ((_ecore_win32_mouse_up_count >= 2) &&
          ((e->timestamp - _ecore_win32_mouse_down_last_time) <= (unsigned long)(1000 * _ecore_win32_double_click_time)) &&
          (e->window == (Ecore_Window)_ecore_win32_mouse_down_last_window))
        e->double_click = 1;

      if ((_ecore_win32_mouse_up_count >= 3) &&
          ((e->timestamp - _ecore_win32_mouse_down_last_last_time) <= (unsigned long)(2 * 1000 * _ecore_win32_double_click_time)) &&
          (e->window == (Ecore_Window)_ecore_win32_mouse_down_last_window) &&
          (e->window == (Ecore_Window)_ecore_win32_mouse_down_last_last_window))
        e->triple_click = 1;

      _ecore_win32_event_last_time = e->timestamp;
      _ecore_win32_event_last_window = (Ecore_Win32_Window *)e->window;

      ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, e, NULL, NULL);
   }
}

void
_ecore_win32_event_handle_motion_notify(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Event_Mouse_Move *e;

   INF("mouse moved");

   e = (Ecore_Event_Mouse_Move *)calloc(1, sizeof(Ecore_Event_Mouse_Move));
   if (!e) return;

   e->window = (Ecore_Window)GetWindowLongPtr(msg->window, GWLP_USERDATA);
   e->event_window = e->window;
   e->x = GET_X_LPARAM(msg->data_param);
   e->y = GET_Y_LPARAM(msg->data_param);
   e->timestamp = msg->timestamp;

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, e, NULL, NULL);
}

void
_ecore_win32_event_handle_enter_notify(Ecore_Win32_Callback_Data *msg)
{
  {
     Ecore_Event_Mouse_Move *e;

     INF("mouse in");

     e = (Ecore_Event_Mouse_Move *)calloc(1, sizeof(Ecore_Event_Mouse_Move));
     if (!e) return;

     e->window = (Ecore_Window)GetWindowLongPtr(msg->window, GWLP_USERDATA);
     e->event_window = e->window;
     e->x = msg->x;
     e->y = msg->y;
     e->timestamp = msg->timestamp;

     _ecore_win32_event_last_time = e->timestamp;
     _ecore_win32_event_last_window = (Ecore_Win32_Window *)e->window;

     ecore_event_add(ECORE_EVENT_MOUSE_MOVE, e, NULL, NULL);
  }

  {
     Ecore_Win32_Event_Mouse_In *e;

     e = (Ecore_Win32_Event_Mouse_In *)calloc(1, sizeof(Ecore_Win32_Event_Mouse_In));
     if (!e) return;

     e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);
     e->x = msg->x;
     e->y = msg->y;
     e->timestamp = msg->timestamp ;

     _ecore_win32_event_last_time = e->timestamp;

     ecore_event_add(ECORE_WIN32_EVENT_MOUSE_IN, e, NULL, NULL);
  }
}

void
_ecore_win32_event_handle_leave_notify(Ecore_Win32_Callback_Data *msg)
{
  {
     Ecore_Event_Mouse_Move *e;

     INF("mouse out");

     e = (Ecore_Event_Mouse_Move *)calloc(1, sizeof(Ecore_Event_Mouse_Move));
     if (!e) return;

     e->window = (Ecore_Window)GetWindowLongPtr(msg->window, GWLP_USERDATA);
     e->event_window = e->window;
     e->x = msg->x;
     e->y = msg->y;
     e->timestamp = msg->timestamp;

     _ecore_win32_event_last_time = e->timestamp;
     _ecore_win32_event_last_window = (Ecore_Win32_Window *)e->window;

     ecore_event_add(ECORE_EVENT_MOUSE_MOVE, e, NULL, NULL);
  }

  {
     Ecore_Win32_Event_Mouse_Out *e;

     e = (Ecore_Win32_Event_Mouse_Out *)calloc(1, sizeof(Ecore_Win32_Event_Mouse_Out));
     if (!e) return;

     e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);
     e->x = msg->x;
     e->y = msg->y;
     e->timestamp = msg->timestamp;

     _ecore_win32_event_last_time = e->timestamp;

     ecore_event_add(ECORE_WIN32_EVENT_MOUSE_OUT, e, NULL, NULL);
  }
}

void
_ecore_win32_event_handle_focus_in(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Win32_Event_Window_Focus_In *e;

   INF("focus in");

   e = (Ecore_Win32_Event_Window_Focus_In *)calloc(1, sizeof(Ecore_Win32_Event_Window_Focus_In));
   if (!e) return;

   e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);

   e->timestamp = _ecore_win32_event_last_time;
   _ecore_win32_event_last_time = e->timestamp;

   ecore_event_add(ECORE_WIN32_EVENT_WINDOW_FOCUS_IN, e, NULL, NULL);
}

void
_ecore_win32_event_handle_focus_out(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Win32_Event_Window_Focus_Out *e;

   INF("focus out");

   e = (Ecore_Win32_Event_Window_Focus_Out *)calloc(1, sizeof(Ecore_Win32_Event_Window_Focus_Out));
   if (!e) return;

   e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);

   e->timestamp = _ecore_win32_event_last_time;
   _ecore_win32_event_last_time = e->timestamp;

   ecore_event_add(ECORE_WIN32_EVENT_WINDOW_FOCUS_OUT, e, NULL, NULL);
}

void
_ecore_win32_event_handle_expose(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Win32_Event_Window_Damage *e;

   INF("window expose");

   e = (Ecore_Win32_Event_Window_Damage *)calloc(1, sizeof(Ecore_Win32_Event_Window_Damage));
   if (!e) return;

   e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);

   e->x = msg->update.left;
   e->y = msg->update.top;
   e->width = msg->update.right - msg->update.left;
   e->height = msg->update.bottom - msg->update.top;

   e->timestamp = _ecore_win32_event_last_time;

   ecore_event_add(ECORE_WIN32_EVENT_WINDOW_DAMAGE, e, NULL, NULL);
}

void
_ecore_win32_event_handle_create_notify(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Win32_Event_Window_Create *e;

   INF("window create notify");

   e = calloc(1, sizeof(Ecore_Win32_Event_Window_Create));
   if (!e) return;

   e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);

   e->timestamp = _ecore_win32_event_last_time;

   ecore_event_add(ECORE_WIN32_EVENT_WINDOW_CREATE, e, NULL, NULL);
}

void
_ecore_win32_event_handle_destroy_notify(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Win32_Event_Window_Destroy *e;

   INF("window destroy notify");

   e = calloc(1, sizeof(Ecore_Win32_Event_Window_Destroy));
   if (!e) return;

   e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);

   e->timestamp = _ecore_win32_event_last_time;
   if (e->window == _ecore_win32_event_last_window) _ecore_win32_event_last_window = NULL;

   ecore_event_add(ECORE_WIN32_EVENT_WINDOW_DESTROY, e, NULL, NULL);
}

void
_ecore_win32_event_handle_map_notify(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Win32_Event_Window_Show *e;

   INF("window map notify");

   e = calloc(1, sizeof(Ecore_Win32_Event_Window_Show));
   if (!e) return;

   e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);

   e->timestamp = _ecore_win32_event_last_time;

   ecore_event_add(ECORE_WIN32_EVENT_WINDOW_SHOW, e, NULL, NULL);
}

void
_ecore_win32_event_handle_unmap_notify(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Win32_Event_Window_Hide *e;

   INF("window unmap notify");

   e = calloc(1, sizeof(Ecore_Win32_Event_Window_Hide));
   if (!e) return;

   e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);

   e->timestamp = _ecore_win32_event_last_time;

   ecore_event_add(ECORE_WIN32_EVENT_WINDOW_HIDE, e, NULL, NULL);
}

void
_ecore_win32_event_handle_configure_notify(Ecore_Win32_Callback_Data *msg)
{
   WINDOWINFO                          wi;
   Ecore_Win32_Event_Window_Configure *e;
   WINDOWPOS                          *window_pos;

   INF("window configure notify");

   e = calloc(1, sizeof(Ecore_Win32_Event_Window_Configure));
   if (!e) return;

   window_pos = (WINDOWPOS *)msg->data_param;
   wi.cbSize = sizeof(WINDOWINFO);
   if (!GetWindowInfo(window_pos->hwnd, &wi))
     {
        free(e);
        return;
     }

   e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);
   e->abovewin = (void *)GetWindowLongPtr(window_pos->hwndInsertAfter, GWLP_USERDATA);
   e->x = wi.rcClient.left;
   e->y = wi.rcClient.top;
   e->width = wi.rcClient.right - wi.rcClient.left;
   e->height = wi.rcClient.bottom - wi.rcClient.top;
   e->timestamp = _ecore_win32_event_last_time;

   ecore_event_add(ECORE_WIN32_EVENT_WINDOW_CONFIGURE, e, NULL, NULL);
}

void
_ecore_win32_event_handle_resize(Ecore_Win32_Callback_Data *msg)
{
   RECT                             rect;
   Ecore_Win32_Event_Window_Resize *e;

   INF("window resize");

   if (!GetClientRect(msg->window, &rect))
     return;

   e = calloc(1, sizeof(Ecore_Win32_Event_Window_Resize));
   if (!e) return;

   e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);
   e->width = rect.right - rect.left;
   e->height = rect.bottom - rect.top;
   e->timestamp = _ecore_win32_event_last_time;

   ecore_event_add(ECORE_WIN32_EVENT_WINDOW_RESIZE, e, NULL, NULL);
}

void
_ecore_win32_event_handle_delete_request(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Win32_Event_Window_Delete_Request *e;

   INF("window delete request");

   e = calloc(1, sizeof(Ecore_Win32_Event_Window_Delete_Request));
   if (!e) return;

   e->window = (void *)GetWindowLongPtr(msg->window, GWLP_USERDATA);
   e->timestamp = _ecore_win32_event_last_time;

   ecore_event_add(ECORE_WIN32_EVENT_WINDOW_DELETE_REQUEST, e, NULL, NULL);
}


/***** Private functions definitions *****/

static void
_ecore_win32_event_free_key_down(void *data __UNUSED__,
                                 void *ev)
{
   Ecore_Event_Key *e;

   e = ev;
   if (e->keyname) free((char *)e->keyname);
   if (e->key) free((char *)e->key);
   if (e->string) free((char *)e->string);
   free(e);
}

static void
_ecore_win32_event_free_key_up(void *data __UNUSED__,
                               void *ev)
{
   Ecore_Event_Key *e;

   e = ev;
   if (e->keyname) free((char *)e->keyname);
   if (e->key) free((char *)e->key);
   if (e->string) free((char *)e->string);
   free(e);
}

static int
_ecore_win32_event_keystroke_get(Ecore_Win32_Callback_Data *msg,
                                 Eina_Bool     is_down,
                                 char        **keyname,
                                 char        **keysymbol,
                                 char        **keycompose,
                                 unsigned int *modifiers)
{
  WCHAR buf[3];
  char delete_string[2] = { 0x7f, 0 };
  char *kn = NULL;
  char *ks = NULL;
  char *kc = NULL;
  int key;
  int is_extended;

  key = msg->window_param;
  is_extended = msg->data_param & 0x01000000;

  *keyname = NULL;
  *keysymbol = NULL;
  *keycompose = NULL;

   switch (key)
     {
       /* Keystroke */
     case VK_PRIOR:
       if (is_extended)
         {
            kn = "Prior";
            ks = "Prior";
            kc = NULL;
         }
       else
         {
            kn = "KP_Prior";
            ks = "KP_9";
            kc = "KP_Prior";
         }
       break;
     case VK_NEXT:
       if (is_extended)
         {
            kn = "Next";
            ks = "Next";
            kc = NULL;
         }
       else
         {
            kn = "KP_Next";
            ks = "KP_3";
            kc = "KP_Next";
         }
       break;
     case VK_END:
       if (is_extended)
         {
            kn = "End";
            ks = "End";
            kc = NULL;
         }
       else
         {
            kn = "KP_End";
            ks = "KP_1";
            kc = "KP_End";
         }
       break;
     case VK_HOME:
       if (is_extended)
         {
            kn = "Home";
            ks = "Home";
            kc = NULL;
         }
       else
         {
            kn = "KP_Home";
            ks = "KP_7";
            kc = "KP_Home";
         }
       break;
     case VK_LEFT:
       if (is_extended)
         {
            kn = "Left";
            ks = "Left";
            kc = NULL;
         }
       else
         {
            kn = "KP_Left";
            ks = "KP_4";
            kc = "KP_Left";
         }
       break;
     case VK_UP:
       if (is_extended)
         {
            kn = "Up";
            ks = "Up";
            kc = NULL;
         }
       else
         {
            kn = "KP_Up";
            ks = "KP_8";
            kc = "KP_Up";
         }
       break;
     case VK_RIGHT:
       if (is_extended)
         {
           kn = "Right";
           ks = "Right";
           kc = NULL;
         }
       else
         {
           kn = "KP_Right";
           ks = "KP_6";
           kc = "KP_Right";
         }
       break;
     case VK_DOWN:
       if (is_extended)
         {
           kn = "Down";
           ks = "Down";
           kc = NULL;
         }
       else
         {
           kn = "KP_Down";
           ks = "KP_2";
           kc = "KP_Down";
         }
       break;
     case VK_INSERT:
       if (is_extended)
         {
           kn = "Insert";
           ks = "Insert";
           kc = NULL;
         }
       else
         {
           kn = "KP_Insert";
           ks = "KP_0";
           kc = "KP_Insert";
         }
       break;
     case VK_DELETE:
       if (is_extended)
         {
           kn = "Delete";
           ks = "Delete";
           kc = delete_string;
         }
       else
         {
           kn = "KP_Delete";
           ks = "KP_Decimal";
           kc = "KP_Delete";
         }
       break;
     case VK_SHIFT:
       {
          SHORT res;

          if (is_down)
            {
               res = GetKeyState(VK_LSHIFT);
               if (res & 0x8000)
                 {
                    _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_LSHIFT;
                    kn = "Shift_L";
                    ks = "Shift_L";
                    kc = "";
                 }
               res = GetKeyState(VK_RSHIFT);
               if (res & 0x8000)
                 {
                    _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_RSHIFT;
                    kn = "Shift_R";
                    ks = "Shift_R";
                    kc = "";
                 }
               *modifiers &= ~ECORE_EVENT_MODIFIER_SHIFT;
            }
          else /* is_up */
            {
               res = GetKeyState(VK_LSHIFT);
               if (!(res & 0x8000) &&
                   (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_LSHIFT))
                 {
                    kn = "Shift_L";
                    ks = "Shift_L";
                    kc = "";
                    _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_LSHIFT;
                 }
               res = GetKeyState(VK_RSHIFT);
               if (!(res & 0x8000) &&
                   (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_RSHIFT))
                 {
                    kn = "Shift_R";
                    ks = "Shift_R";
                    kc = "";
                    _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_RSHIFT;
                 }
               *modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
            }
          break;
       }
     case VK_CONTROL:
       {
          SHORT res;

          if (msg->discard_ctrl)
            return 0;

          if (is_down)
            {
               res = GetKeyState(VK_LCONTROL);
               if (res & 0x8000)
                 {
                    _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_LCONTROL;
                    kn = "Control_L";
                    ks = "Control_L";
                    kc = "";
                    break;
                 }
               res = GetKeyState(VK_RCONTROL);
               if (res & 0x8000)
                 {
                    _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_RCONTROL;
                    kn = "Control_R";
                    ks = "Control_R";
                    kc = "";
                    break;
                 }
               *modifiers |= ECORE_EVENT_MODIFIER_CTRL;
            }
          else /* is_up */
            {
               res = GetKeyState(VK_LCONTROL);
               if (!(res & 0x8000) &&
                   (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_LCONTROL))
                 {
                    kn = "Control_L";
                    ks = "Control_L";
                    kc = "";
                    _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_LCONTROL;
                    break;
                 }
               res = GetKeyState(VK_RCONTROL);
               if (!(res & 0x8000) &&
                   (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_RCONTROL))
                 {
                    kn = "Control_R";
                    ks = "Control_R";
                    kc = "";
                    _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_RCONTROL;
                    break;
                 }
               *modifiers &= ~ECORE_EVENT_MODIFIER_CTRL;
            }
          break;
       }
     case VK_MENU:
       {
          SHORT res;

          if (is_down)
            {
               res = GetKeyState(VK_LMENU);
               if (res & 0x8000)
                 {
                    _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_LMENU;
                    kn = "Alt_L";
                    ks = "Alt_L";
                    kc = "";
                 }
               res = GetKeyState(VK_RMENU);
               if (res & 0x8000)
                 {
                    _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_RMENU;
                    kn = "Alt_R";
                    ks = "Alt_R";
                    kc = "";
                 }
               *modifiers |= ECORE_EVENT_MODIFIER_ALT;
            }
          else /* is_up */
            {
               res = GetKeyState(VK_LMENU);
               if (!(res & 0x8000) &&
                   (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_LMENU))
                 {
                    kn = "Alt_L";
                    ks = "Alt_L";
                    kc = "";
                    _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_LMENU;
                 }
               res = GetKeyState(VK_RMENU);
               if (!(res & 0x8000) &&
                   (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_RMENU))
                 {
                    kn = "Alt_R";
                    ks = "Alt_R";
                    kc = "";
                    _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_RMENU;
                 }
               *modifiers &= ~ECORE_EVENT_MODIFIER_ALT;
            }
          break;
       }
     case VK_LWIN:
       {
          if (is_down)
            {
               kn = "Super_L";
               ks = "Super_L";
               kc = "";
               *modifiers |= ECORE_EVENT_MODIFIER_WIN;
            }
          else /* is_up */
            {
               kn = "Super_L";
               ks = "Super_L";
               kc = "";
               *modifiers &= ~ECORE_EVENT_MODIFIER_WIN;
            }
          break;
       }
     case VK_RWIN:
       {
          if (is_down)
            {
               kn = "Super_R";
               ks = "Super_R";
               kc = "";
               *modifiers |= ECORE_EVENT_MODIFIER_WIN;
            }
          else /* is_up */
            {
               kn = "Super_R";
               ks = "Super_R";
               kc = "";
               *modifiers &= ~ECORE_EVENT_MODIFIER_WIN;
            }
          break;
       }
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
     default:
       {
          /* other non keystroke characters */
          BYTE kbd_state[256];
          int res;

          if (is_down)
            return 0;

          if (!GetKeyboardState(kbd_state))
            return 0;

           res = ToUnicode(msg->window_param,
                           MapVirtualKey(msg->window_param, 2),
                           kbd_state, buf, 3, 0);
           if (res == 1)
             {
               /* FIXME: might be troublesome for non european languages */
               /* in that case, UNICODE should be used, I guess */
               buf[1] = '\0';
               kn = (char *)buf;
               ks = (char *)buf;
               kc = (char *)buf;

               res = GetAsyncKeyState(VK_SHIFT);
               if (res & 0x8000)
                 *modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
               else
                 *modifiers &= ~ECORE_EVENT_MODIFIER_SHIFT;

               res = GetKeyState(VK_CONTROL);
               if (res & 0x8000)
                 *modifiers |= ECORE_EVENT_MODIFIER_CTRL;
               else
                 *modifiers &= ~ECORE_EVENT_MODIFIER_CTRL;

               res = GetKeyState(VK_MENU);
               if (res & 0x8000)
                 *modifiers |= ECORE_EVENT_MODIFIER_ALT;
               else
                 *modifiers &= ~ECORE_EVENT_MODIFIER_ALT;

               break;
             }
           return 0;
         }
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
   if (!kc)
     *keycompose = NULL;
   else
     {
        *keycompose = strdup(kc);
        if (!*keycompose)
          {
             free(*keyname);
             free(*keysymbol);
             *keyname = NULL;
             *keysymbol = NULL;
             return 0;
          }
     }

   return 1;
}

static int
_ecore_win32_event_char_get(int           key,
                            char        **keyname,
                            char        **keysymbol,
                            char        **keycompose,
                            unsigned int *modifiers)
{
  char *kn = NULL;
  char *ks = NULL;
  char *kc = NULL;
  char buf[2];
  SHORT res;

  *keyname = NULL;
  *keysymbol = NULL;
  *keycompose = NULL;

   switch (key)
     {
     case VK_PROCESSKEY:
      break;
     case VK_BACK:
       kn = "BackSpace";
       ks = "BackSpace";
       kc = "\b";
       break;
     case VK_TAB:
       kn = "Tab";
       ks = "Tab";
       kc = "\t";
       break;
     case 0x0a:
       /* Line feed (Shift + Enter) */
       kn = "LineFeed";
       ks = "LineFeed";
       kc = "LineFeed";
       break;
     case VK_RETURN:
       kn = "Return";
       ks = "Return";
       kc = "\n";
       break;
     case VK_ESCAPE:
       kn = "Escape";
       ks = "Escape";
       kc = "\e";
       break;
     case VK_SPACE:
       kn = "space";
       ks = "space";
       kc = " ";
       break;
     default:
       /* displayable characters */
       buf[0] = key;
       buf[1] = '\0';
       kn = buf;
       ks = buf;
       kc = buf;
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

   res = GetAsyncKeyState(VK_SHIFT);
   if (res & 0x8000)
     *modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   else
     *modifiers &= ~ECORE_EVENT_MODIFIER_SHIFT;

   res = GetKeyState(VK_CONTROL);
   if (res & 0x8000)
     *modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   else
     *modifiers &= ~ECORE_EVENT_MODIFIER_CTRL;

   res = GetKeyState(VK_MENU);
   if (res & 0x8000)
     *modifiers |= ECORE_EVENT_MODIFIER_ALT;
   else
     *modifiers &= ~ECORE_EVENT_MODIFIER_ALT;

   return 1;
}
