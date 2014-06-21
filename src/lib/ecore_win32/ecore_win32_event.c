#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <windowsx.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Input.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"
#include "ecore_win32_keysym_table.h"

#define ECORE_WIN32_VK_IS_EXTENDED(p) ((HIWORD(p) & KF_EXTENDED) == KF_EXTENDED)
#define ECORE_WIN32_VK_IS_REPEATED(p) ((HIWORD(p) & KF_REPEAT) == KF_REPEAT)
#define ECORE_WIN32_VK_IS_PRESSED(vk) ((GetKeyState(vk) & KF_UP) == KF_UP)
#define ECORE_WIN32_VK_IS_TOGGLED(vk) ((GetKeyState(vk) & 0x0001) == 0x0001)

typedef enum
{
   ECORE_WIN32_KEY_MASK_LSHIFT = 1 << 0,
   ECORE_WIN32_KEY_MASK_RSHIFT = 1 << 1,
   ECORE_WIN32_KEY_MASK_LCONTROL = 1 << 2,
   ECORE_WIN32_KEY_MASK_RCONTROL = 1 << 3,
   ECORE_WIN32_KEY_MASK_LMENU = 1 << 4,
   ECORE_WIN32_KEY_MASK_RMENU = 1 << 5
} Ecore_Win32_Key_Mask;

static Ecore_Win32_Window  *_ecore_win32_mouse_down_last_window = NULL;
static Ecore_Win32_Window  *_ecore_win32_mouse_down_last_last_window = NULL;
static long                 _ecore_win32_mouse_down_last_time = 0  ;
static long                 _ecore_win32_mouse_down_last_last_time = 0  ;
static int                  _ecore_win32_mouse_down_did_triple = 0;
static int                  _ecore_win32_mouse_up_count = 0;
static Ecore_Win32_Key_Mask _ecore_win32_key_mask = 0;
static Eina_Bool            _ecore_win32_ctrl_fake = EINA_FALSE;

static unsigned int
_ecore_win32_modifiers_get(void)
{
   unsigned int modifiers = 0;

   if (ECORE_WIN32_VK_IS_PRESSED(VK_SHIFT))
     modifiers |= ECORE_EVENT_MODIFIER_SHIFT;

   if (_ecore_win32_ctrl_fake && !ECORE_WIN32_VK_IS_PRESSED(VK_RMENU))
     {
        _ecore_win32_ctrl_fake = EINA_FALSE;
     }
   else
     {
        if (ECORE_WIN32_VK_IS_PRESSED(VK_CONTROL))
          modifiers |= ECORE_EVENT_MODIFIER_CTRL;
     }

   if (ECORE_WIN32_VK_IS_PRESSED(VK_LMENU))
     modifiers |= ECORE_EVENT_MODIFIER_ALT;

   if (ECORE_WIN32_VK_IS_PRESSED(VK_RMENU))
     modifiers |= ECORE_EVENT_MODIFIER_ALTGR;

   if (ECORE_WIN32_VK_IS_PRESSED(VK_LWIN))
     modifiers |= ECORE_EVENT_MODIFIER_WIN;

   if (ECORE_WIN32_VK_IS_PRESSED(VK_RWIN))
     modifiers |= ECORE_EVENT_MODIFIER_WIN;

   if (ECORE_WIN32_VK_IS_TOGGLED(VK_SCROLL))
     modifiers |= ECORE_EVENT_LOCK_SCROLL;

   if (ECORE_WIN32_VK_IS_TOGGLED(VK_NUMLOCK))
     modifiers |= ECORE_EVENT_LOCK_NUM;

   if (ECORE_WIN32_VK_IS_TOGGLED(VK_CAPITAL))
     modifiers |= ECORE_EVENT_LOCK_CAPS;

   return modifiers;
}

static void
_ecore_win32_modifiers_alt_save(BYTE *kbd_state, unsigned short *modifiers)
{
   if (kbd_state[VK_LMENU] & 128)
     {
        *modifiers |= 1 << 0;
        kbd_state[VK_LMENU] &= ~128;
     }
   if (kbd_state[VK_RMENU] & 128)
     {
        *modifiers |= 1 << 1;
           kbd_state[VK_RMENU] &= ~128;
     }
   if (kbd_state[VK_MENU] & 128)
     {
        *modifiers |= 1 << 2;
           kbd_state[VK_MENU] &= ~128;
     }
}

static void
_ecore_win32_modifiers_alt_restore(BYTE *kbd_state, unsigned short modifiers)
{
   if ((modifiers & (1 << 0)) == (1 << 0))
     kbd_state[VK_LMENU] |= 128;
   if ((modifiers & (1 << 1)) == (1 << 1))
     kbd_state[VK_RMENU] |= 128;
   if ((modifiers & (1 << 2)) == (1 << 2))
     kbd_state[VK_MENU] |= 128;
}

static void
_ecore_win32_modifiers_ctrl_save(BYTE *kbd_state, unsigned short *modifiers)
{
   if (kbd_state[VK_LCONTROL] & 128)
     {
        *modifiers |= 1 << 3;
        kbd_state[VK_LCONTROL] &= ~128;
     }
   if (kbd_state[VK_RCONTROL] & 128)
     {
        *modifiers |= 1 << 4;
        kbd_state[VK_RCONTROL] &= ~128;
     }
   if (kbd_state[VK_CONTROL] & 128)
     {
        *modifiers |= 1 << 5;
        kbd_state[VK_CONTROL] &= ~128;
     }
}

static void
_ecore_win32_modifiers_ctrl_restore(BYTE *kbd_state, unsigned short modifiers)
{
   if ((modifiers & (1 << 3)) == (1 << 3))
     kbd_state[VK_LCONTROL] |= 128;
   if ((modifiers & (1 << 4)) == (1 << 4))
     kbd_state[VK_RCONTROL] |= 128;
   if ((modifiers & (1 << 5)) == (1 << 5))
     kbd_state[VK_CONTROL] |= 128;
}

static void
_ecore_win32_modifiers_shift_save(BYTE *kbd_state, unsigned short *modifiers)
{
   if (kbd_state[VK_LSHIFT] & 128)
     {
        *modifiers |= 1 << 6;
        kbd_state[VK_LSHIFT] &= ~128;
     }
   if (kbd_state[VK_RSHIFT] & 128)
     {
        *modifiers |= 1 << 7;
        kbd_state[VK_RSHIFT] &= ~128;
     }
   if (kbd_state[VK_SHIFT] & 128)
     {
        *modifiers |= 1 << 8;
        kbd_state[VK_SHIFT] &= ~128;
     }
}

static void
_ecore_win32_modifiers_shift_restore(BYTE *kbd_state, unsigned short modifiers)
{
   if ((modifiers & (1 << 6)) == (1 << 6))
     kbd_state[VK_LSHIFT] |= 128;
   if ((modifiers & (1 << 7)) == (1 << 7))
     kbd_state[VK_RSHIFT] |= 128;
   if ((modifiers & (1 << 8)) == (1 << 8))
     kbd_state[VK_SHIFT] |= 128;
}

static void
_ecore_win32_modifiers_win_save(BYTE *kbd_state, unsigned short *modifiers)
{
   if (kbd_state[VK_LWIN] & 128)
     {
        *modifiers |= 1 << 9;
        kbd_state[VK_LWIN] &= ~128;
     }
   if (kbd_state[VK_RWIN] & 128)
     {
        *modifiers |= 1 << 10;
        kbd_state[VK_RWIN] &= ~128;
     }
}

static void
_ecore_win32_modifiers_win_restore(BYTE *kbd_state, unsigned short modifiers)
{
   if ((modifiers & (1 << 9)) == (1 << 9))
     kbd_state[VK_LWIN] |= 128;
   if ((modifiers & (1 << 10)) == (1 << 10))
     kbd_state[VK_RWIN] |= 128;
}

static uint32_t
_ecore_win32_keysym_offset_get(uint32_t keysym)
{
   uint32_t start;
   uint32_t end;

   /*
    * statistically, there are more chance to press a key with
    * keysym between 0 and 255
    */
   if (keysym < 256)
     {
        start = 0;
        end = 255;
     }
   else
     {
        start = 256;
        end = sizeof(_ecore_win32_name_to_keysym) / sizeof(ecore_win32_name_keysym) - 1;
     }

   do {
      uint32_t m = (start + end) / 2;

      if (_ecore_win32_name_to_keysym[m].keysym == keysym)
        return _ecore_win32_name_to_keysym[m].offset;
      else
        {
           if (keysym > _ecore_win32_name_to_keysym[m].keysym)
             start = m + 1;
           else
             end = m - 1;
        }
   } while (start <= end);

   return 0xffffffff;
}

static const char *
_ecore_win32_diacritic_get(WCHAR wc)
{
   switch (wc)
     {
      case 0x0060: /* grave accent */
         return "dead_grave";
      case 0x00b4: /* acute accent */
         return "dead_acute";
      case 0x005e: /* circumflex_accent */
         return "dead_circumflex";
      case 0x007e: /* tilde_accent */
         return "dead_tilde";
      case 0x00a8: /* diaeresis_accent */
         return "dead_diaeresis";
         /*
          * FIXME: add more diacritics
          * needs more langages and keyboards
          * see http://en.wikipedia.org/wiki/Diacritic
          */
     }

   return NULL;
}

static Ecore_Event_Key *
_ecore_win32_event_keystroke_get(Ecore_Win32_Callback_Data *msg,
                                 Eina_Bool is_down)
{
   Ecore_Event_Key *e;
   char string[2] = { 0, 0 };
   const char *keyname = NULL;
   const char *key = NULL;
   const char *compose = NULL;

   switch (msg->window_param)
     {
        /* VK_LBUTTON: 0x01 : not launched by message */
        /* VK_RBUTTON: 0x02 : not launched by message */
        /* VK_CANCEL: 0x03 : FIXME: what to do ? */
        /* VK_MBUTTON: 0x04 : not launched by message */
        /* VK_XBUTTON1: 0x05 : not launched by message */
        /* VK_XBUTTON2: 0x06 : not launched by message */

        /* 0x07 : undefined */

      case VK_BACK: /* 0x08 */
         keyname = "Backspace";
         key = "Backspace";
         compose = "\b";
         break;
      case VK_TAB: /* 0x09 */
         keyname = "Tab";
         key = "Tab";
         compose = "\t";
         break;

         /* 0x0A - 0x0B : reserved */

      case VK_CLEAR: /* 0x0C */
         keyname = "KP_Begin";
         key = "KP_Begin";
         compose = NULL;
         break;
      case VK_RETURN: /* 0x0D */
         if (ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
           {
              keyname = "KP_Enter";
              key = "KP_Enter";
           }
         else
           {
              keyname = "Return";
              key = "Return";
           }
         compose = "\n";
         break;

         /* 0x0E - 0x0F : undefined */

      case VK_SHIFT: /* 0x10 */
        {
           if (is_down)
             {
                if (ECORE_WIN32_VK_IS_REPEATED(msg->data_param))
                  return NULL;

                if (ECORE_WIN32_VK_IS_PRESSED(VK_LSHIFT))
                  {
                     _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_LSHIFT;
                     keyname = "Shift_L";
                     key = "Shift_L";
                     compose = NULL;
                  }
                if (ECORE_WIN32_VK_IS_PRESSED(VK_RSHIFT))
                  {
                     _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_RSHIFT;
                     keyname = "Shift_R";
                     key = "Shift_R";
                     compose = NULL;
                  }
             }
           else /* is_up */
             {
                if (!ECORE_WIN32_VK_IS_PRESSED(VK_LSHIFT) &&
                    (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_LSHIFT))
                  {
                     keyname = "Shift_L";
                     key = "Shift_L";
                     compose = NULL;
                     _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_LSHIFT;
                  }
                if (!ECORE_WIN32_VK_IS_PRESSED(VK_RSHIFT) &&
                    (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_RSHIFT))
                  {
                     keyname = "Shift_R";
                     key = "Shift_R";
                     compose = NULL;
                     _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_RSHIFT;
                  }
             }
           break;
        }
      case VK_CONTROL: /* 0x11 */
        {
           if (is_down)
             {
                /* We check if Ctrl has be thrown because of AltGr being stroke */
                if (!ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
                  {
                     BOOL res;
                     MSG next_msg;

                     /*
                      * we check if the next message
                      * - is a WM_KEYDOWN
                      * - has the same timestamp than the Ctrl one
                      * - is the key press of the Alt key
                      */
                     res = PeekMessage(&next_msg, msg->window,
                                       WM_KEYDOWN, WM_KEYDOWN,
                                       PM_NOREMOVE);
                     if (res &&
                         (next_msg.wParam == VK_MENU) &&
                         (next_msg.time == msg->timestamp) &&
                         ECORE_WIN32_VK_IS_EXTENDED(next_msg.lParam))
                       {
                          INF("discard left Ctrl key press (sent by AltGr key press)");
                          _ecore_win32_ctrl_fake = EINA_TRUE;
                          return NULL;
                       }
                  }

                if (ECORE_WIN32_VK_IS_REPEATED(msg->data_param))
                  return NULL;

                if (ECORE_WIN32_VK_IS_PRESSED(VK_LCONTROL))
                  {
                     _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_LCONTROL;
                     keyname = "Control_L";
                     key = "Control_L";
                     compose = NULL;
                     break;
                  }
                if (ECORE_WIN32_VK_IS_PRESSED(VK_RCONTROL))
                  {
                     _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_RCONTROL;
                     keyname = "Control_R";
                     key = "Control_R";
                     compose = NULL;
                     break;
                  }
             }
           else /* is_up */
             {
                /* We check if Ctrl has be thrown because of AltGr being stroke */
                if (!ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
                  {
                     BOOL res;
                     MSG next_msg;

                     /*
                      * we check if the next message
                      * - is a WM_KEYUP or WM_SYSKEYUP
                      * - has the same timestamp than the Ctrl one
                      * - is the key press of the Alt key
                      */
                     res = PeekMessage(&next_msg, msg->window,
                                       WM_KEYUP, WM_SYSKEYUP,
                                       PM_NOREMOVE);
                     if (res &&
                         ((next_msg.message == WM_KEYUP) ||
                          (next_msg.message == WM_SYSKEYUP)) &&
                         (next_msg.wParam == VK_MENU) &&
                         (next_msg.time == msg->timestamp) &&
                         ECORE_WIN32_VK_IS_EXTENDED(next_msg.lParam))
                       {
                          INF("discard left Ctrl key press (sent by AltGr key press)");
                          _ecore_win32_ctrl_fake = EINA_TRUE;
                          return NULL;
                       }
                  }
                if (!ECORE_WIN32_VK_IS_PRESSED(VK_LCONTROL) &&
                    (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_LCONTROL))
                  {
                     keyname = "Control_L";
                     key = "Control_L";
                     compose = NULL;
                     _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_LCONTROL;
                     break;
                  }
                if (!ECORE_WIN32_VK_IS_PRESSED(VK_LCONTROL) &&
                    (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_RCONTROL))
                  {
                     keyname = "Control_R";
                     key = "Control_R";
                     compose = NULL;
                     _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_RCONTROL;
                     break;
                  }
             }
           break;
        }
      case VK_MENU: /* 0x12 */
        {
           if (is_down)
             {
                if (ECORE_WIN32_VK_IS_REPEATED(msg->data_param))
                  return NULL;

                if (ECORE_WIN32_VK_IS_PRESSED(VK_LMENU))
                  {
                     _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_LMENU;
                     keyname = "Alt_L";
                     key = "Alt_L";
                     compose = NULL;
                  }
                if (ECORE_WIN32_VK_IS_PRESSED(VK_RMENU))
                  {
                     _ecore_win32_key_mask |= ECORE_WIN32_KEY_MASK_RMENU;
                     keyname = "ISO_Level3_Shift";
                     key = "ISO_Level3_Shift";
                     compose = NULL;
                  }
             }
           else /* is_up */
             {
                if (!ECORE_WIN32_VK_IS_PRESSED(VK_LMENU) &&
                    (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_LMENU))
                  {
                     keyname = "Alt_L";
                     key = "Alt_L";
                     compose = NULL;
                     _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_LMENU;
                  }
                if (!ECORE_WIN32_VK_IS_PRESSED(VK_RMENU) &&
                    (_ecore_win32_key_mask & ECORE_WIN32_KEY_MASK_RMENU))
                  {
                     keyname = "ISO_Level3_Shift";
                     key = "ISO_Level3_Shift";
                     compose = NULL;
                     _ecore_win32_key_mask &= ~ECORE_WIN32_KEY_MASK_RMENU;
                  }
             }
           break;
        }
      case VK_PAUSE: /* 0x13 */
         keyname = "Pause";
         key = "Pause";
         compose = NULL;
         break;
      case VK_CAPITAL: /* 0x14 */
         keyname = "Caps_Lock";
         key = "Caps_Lock";
         compose = NULL;
         break;

         /* VK_KANA: 0x15 : IME Kana mode */
         /* VK_HANGUEL: 0x15 : IME Hanguel mode (maintained for compatibility; use VK_HANGUL) */
         /* VK_HANGUL: 0x15 : IME Hangul mode */

         /* 0x16 : undefined */

         /* VK_JUNJA: 0x17 : IME Junja mode */
         /* VK_FINAL: 0x18 : IME final mode */
         /* VK_HANJA: 0x19 : IME Hanja mode */
         /* VK_KANJI: 0x19 : IME Kanji mode */

         /* 0x1A : undefined */

      case VK_ESCAPE: /* 0x1B */
         keyname = "Escape";
         key = "Escape";
         string[0] = 0x1b;
         compose = string;
         break;

         /* VK_CONVERT: 0x1C : IME convert */
         /* VK_NONCONVERT: 0x1D : IME nonconvert */
         /* VK_ACCEPT: 0x1E : IME accept */
         /* VK_MODECHANGE: 0x1F : IME mode change request */

      case VK_SPACE: /* 0x20 */
         keyname = "space";
         key = "space";
         compose = " ";
         break;

      case VK_PRIOR: /* 0x21 */
         if (ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
           {
              keyname = "Prior";
              key = "Prior";
              compose = NULL;
           }
         else
           {
              keyname = "KP_Prior";
              key = "KP_Prior";
              compose = NULL;
           }
         break;
      case VK_NEXT: /* 0x22 */
         if (ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
           {
              keyname = "Next";
              key = "Next";
              compose = NULL;
           }
         else
           {
              keyname = "KP_Next";
              key = "KP_Next";
              compose = NULL;
           }
         break;
      case VK_END: /* 0x23 */
         if (ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
           {
              keyname = "End";
              key = "End";
              compose = NULL;
           }
         else
           {
              keyname = "KP_End";
              key = "KP_End";
              compose = NULL;
           }
         break;
      case VK_HOME: /* 0x24 */
         if (ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
           {
              keyname = "Home";
              key = "Home";
              compose = NULL;
           }
         else
           {
              keyname = "KP_HOME";
              key = "KP_HOME";
              compose = NULL;
           }
         break;
      case VK_LEFT: /* 0x25 */
         if (ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
           {
              keyname = "Left";
              key = "Left";
              compose = NULL;
           }
         else
           {
              keyname = "KP_Left";
              key = "KP_Left";
              compose = NULL;
           }
         break;
      case VK_UP: /* 0x26 */
         if (ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
           {
              keyname = "Up";
              key = "Up";
              compose = NULL;
           }
         else
           {
              keyname = "KP_Up";
              key = "KP_Up";
              compose = NULL;
           }
         break;
      case VK_RIGHT: /* 0x27 */
         if (ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
           {
              keyname = "Right";
              key = "Right";
              compose = NULL;
           }
         else
           {
              keyname = "KP_Right";
              key = "KP_Right";
              compose = NULL;
           }
         break;
      case VK_DOWN: /* 0x28 */
         if (ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
           {
              keyname = "Down";
              key = "Down";
              compose = NULL;
           }
         else
           {
              keyname = "KP_Down";
              key = "KP_Down";
              compose = NULL;
           }
         break;

         /* VK_SELECT: 0x29 : SELECT key (for a menu ? on rare keyboards ?) */
         /* VK_PRINT: 0x2A : PRINT key on old IBM PC XT and AT, maybe also on Nokia keyboards */
         /* VK_EXECUTE: 0x2B : EXECUTE key (for a menu ? on rare keyboards ?) */
         /* VK_SNAPSHOT: 0x2C : EXECUTE key (for a menu ? on rare keyboards ?) */

      case VK_INSERT: /* 0x2D */
         if (ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
           {
              keyname = "Insert";
              key = "Insert";
              compose = NULL;
           }
         else
           {
              keyname = "KP_Insert";
              key = "KP_Insert";
              compose = NULL;
           }
         break;
      case VK_DELETE: /* 0x2E */
         if (ECORE_WIN32_VK_IS_EXTENDED(msg->data_param))
           {
              keyname = "Delete";
              key = "Delete";
              string[0] = 0x7f;
              compose = string;
           }
         else
           {
              keyname = "KP_Delete";
              key = "KP_Delete";
              compose = NULL;
           }
         break;

         /* VK_HELP: 0x2F : HELP key (for a menu ? on rare keyboards ?) */

         /* case 0x30: 0 key (default case) */
         /* case 0x31: 1 key (default case) */
         /* case 0x32: 2 key (default case) */
         /* case 0x33: 3 key (default case) */
         /* case 0x34: 4 key (default case) */
         /* case 0x35: 5 key (default case) */
         /* case 0x36: 6 key (default case) */
         /* case 0x37: 7 key (default case) */
         /* case 0x38: 8 key (default case) */
         /* case 0x39: 9 key (default case) */

         /* 0x3A - 0x40 : undefined */

         /* case 0x41: A key (default case) */
         /* case 0x42: B key (default case) */
         /* case 0x43: C key (default case) */
         /* case 0x44: D key (default case) */
         /* case 0x45: E key (default case) */
         /* case 0x46: F key (default case) */
         /* case 0x47: G key (default case) */
         /* case 0x48: H key (default case) */
         /* case 0x49: I key (default case) */
         /* case 0x4a: J key (default case) */
         /* case 0x4b: K key (default case) */
         /* case 0x4c: L key (default case) */
         /* case 0x4d: M key (default case) */
         /* case 0x4e: N key (default case) */
         /* case 0x4f: O key (default case) */
         /* case 0x50: P key (default case) */
         /* case 0x51: Q key (default case) */
         /* case 0x52: R key (default case) */
         /* case 0x53: S key (default case) */
         /* case 0x54: T key (default case) */
         /* case 0x55: U key (default case) */
         /* case 0x56: V key (default case) */
         /* case 0x57: W key (default case) */
         /* case 0x58: X key (default case) */
         /* case 0x59: Y key (default case) */
         /* case 0x5a: Z key (default case) */

      case VK_LWIN: /* 0x5B */
        {
           if (is_down)
             {
                if (ECORE_WIN32_VK_IS_REPEATED(msg->data_param))
                  return NULL;

                keyname = "Super_L";
                key = "Super_L";
                compose = NULL;
             }
           else /* is_up */
             {
                keyname = "Super_L";
                key = "Super_L";
                compose = NULL;
             }
           break;
        }
      case VK_RWIN: /* 0x5C */
        {
           if (is_down)
             {
                if (ECORE_WIN32_VK_IS_REPEATED(msg->data_param))
                  return NULL;

                keyname = "Super_R";
                key = "Super_R";
                compose = NULL;
             }
           else /* is_up */
             {
                keyname = "Super_R";
                key = "Super_R";
                compose = NULL;
             }
           break;
        }
      case VK_APPS: /* 0x5D Menu key on some keyboards, or via the 'function' modifier */
         keyname = "Menu";
         key = "Menu";
         compose = NULL;
         break;

         /* 0x5E : reserved */

         /* case VK_SLEEP: 0x5F SLEEP key (not hookable, the computer goes to sleep immediatly...) */

      case VK_NUMPAD0: /* 0x60 */
         keyname = "KP_Insert";
         key = "KP_0";
         compose = "0";
         break;
      case VK_NUMPAD1: /* 0x61 */
         keyname = "KP_End";
         key = "KP_1";
         compose = "1";
         break;
      case VK_NUMPAD2: /* 0x62 */
         keyname = "KP_Down";
         key = "KP_2";
         compose = "2";
         break;
      case VK_NUMPAD3: /* 0x63 */
         keyname = "KP_Next";
         key = "KP_3";
         compose = "3";
         break;
      case VK_NUMPAD4: /* 0x64 */
         keyname = "KP_Left";
         key = "KP_4";
         compose = "4";
         break;
      case VK_NUMPAD5: /* 0x65 */
         keyname = "KP_Begin";
         key = "KP_5";
         compose = "5";
         break;
      case VK_NUMPAD6: /* 0x66 */
         keyname = "KP_Right";
         key = "KP_6";
         compose = "6";
         break;
      case VK_NUMPAD7: /* 0x67 */
         keyname = "KP_HOME";
         key = "KP_7";
         compose = "7";
         break;
      case VK_NUMPAD8: /* 0x68 */
         keyname = "KP_Up";
         key = "KP_8";
         compose = "8";
         break;
      case VK_NUMPAD9: /* 0x69 */
         keyname = "KP_Prior";
         key = "KP_9";
         compose = "9";
         break;
      case VK_MULTIPLY: /* 0x6A */
         keyname = "KP_Multiply";
         key = "KP_Multiply";
         compose = "*";
         break;
      case VK_ADD: /* 0x6A */
         keyname = "KP_Add";
         key = "KP_Add";
         compose = "+";
         break;
         /* VK_SEPARATOR : 0x6C : FIXME depending on layout : */
         /* . or , for the separator in a floating point number*/
      case VK_SUBTRACT: /* 0x6D */
         keyname = "KP_Subtract";
         key = "KP_Subtract";
         compose = "-";
         break;
      case VK_DECIMAL: /* 0x6E */
         /* '.' key with num lock on */
         keyname = "KP_Delete";
         key = "KP_Decimal";
         /* Value of 'compose' may depend on the layout */
         compose = ".";
         break;
      case VK_DIVIDE: /* 0x6F */
         keyname = "KP_Divide";
         key = "KP_Divide";
         compose = "/";
         break;
      case VK_F1: /* 0x70 */
         keyname = "F1";
         key = "F1";
         compose = NULL;
         break;
      case VK_F2: /* 0x71 */
         keyname = "F2";
         key = "F2";
         compose = NULL;
         break;
      case VK_F3: /* 0x72 */
         keyname = "F3";
         key = "F3";
         compose = NULL;
         break;
      case VK_F4: /* 0x73 */
         keyname = "F4";
         key = "F4";
         compose = NULL;
         break;
      case VK_F5: /* 0x74 */
         keyname = "F5";
         key = "F5";
         compose = NULL;
         break;
      case VK_F6: /* 0x75 */
         keyname = "F6";
         key = "F6";
         compose = NULL;
         break;
      case VK_F7: /* 0x76 */
         keyname = "F7";
         key = "F7";
         compose = NULL;
         break;
      case VK_F8: /* 0x77 */
         keyname = "F8";
         key = "F8";
         compose = NULL;
         break;
      case VK_F9: /* 0x78 */
         keyname = "F9";
         key = "F9";
         compose = NULL;
         break;
      case VK_F10: /* 0x79 */
         keyname = "F10";
         key = "F10";
         compose = NULL;
         break;
      case VK_F11: /* 0x7A */
         keyname = "F11";
         key = "F11";
         compose = NULL;
         break;
      case VK_F12: /* 0x7B */
         keyname = "F12";
         key = "F12";
         compose = NULL;
         break;
      case VK_F13: /* 0x7C */
         keyname = "F13";
         key = "F13";
         compose = NULL;
         break;
      case VK_F14: /* 0x7D */
         keyname = "F14";
         key = "F14";
         compose = NULL;
         break;
      case VK_F15: /* 0x7E */
         keyname = "F15";
         key = "F15";
         compose = NULL;
         break;
      case VK_F16: /* 0x7F */
         keyname = "F16";
         key = "F16";
         compose = NULL;
         break;
      case VK_F17: /* 0x80 */
         keyname = "F17";
         key = "F17";
         compose = NULL;
         break;
      case VK_F18: /* 0x81 */
         keyname = "F18";
         key = "F18";
         compose = NULL;
         break;
      case VK_F19: /* 0x82 */
         keyname = "F19";
         key = "F19";
         compose = NULL;
         break;
      case VK_F20: /* 0x83 */
         keyname = "F20";
         key = "F20";
         compose = NULL;
         break;
      case VK_F21: /* 0x84 */
         keyname = "F21";
         key = "F21";
         compose = NULL;
         break;
      case VK_F22: /* 0x85 */
         keyname = "F22";
         key = "F22";
         compose = NULL;
         break;
      case VK_F23: /* 0x86 */
         keyname = "F23";
         key = "F23";
         compose = NULL;
         break;
      case VK_F24: /* 0x87 */
         keyname = "F24";
         key = "F24";
         compose = NULL;
         break;

         /* 0x88 - 0x8F : unassigned */

      case VK_NUMLOCK: /* 0x90 */
         keyname = "Num_Lock";
         key = "Num_Lock";
         compose = NULL;
         break;
      case VK_SCROLL: /* 0x91 */
         keyname = "Scroll_Lock";
         key = "Scroll_Lock";
         compose = NULL;
         break;

         /* 0x92 - 0x96 : OEM specific */

         /* 0x97 - 0x9F : unassigned */

         /* VK_LSHIT: 0xA0 : not launched by message */
         /* VK_RSHIT: 0xA1 : not launched by message */
         /* VK_LCONTROL: 0xA2 : not launched by message */
         /* VK_RCONTROL: 0xA3 : not launched by message */
         /* VK_LMENU: 0xA4 : not launched by message */
         /* VK_RMENU: 0xA5 : not launched by message */

      case VK_BROWSER_BACK: /* 0xA6 */
         keyname = "XF86Back";
         key = "XF86Back";
         compose = NULL;
         break;
      case VK_BROWSER_FORWARD: /* 0xA7 */
         keyname = "XF86Forward";
         key = "XF86Forward";
         compose = NULL;
         break;
      case VK_BROWSER_REFRESH: /* 0xA8 */
         keyname = "XF86Forward";
         key = "XF86Forward";
         compose = NULL;
         break;
      case VK_BROWSER_STOP: /* 0xA9 */
         keyname = "XF86Stop";
         key = "XF86Stop";
         compose = NULL;
         break;
      case VK_BROWSER_SEARCH: /* 0xAA */
         keyname = "XF86Search";
         key = "XF86Search";
         compose = NULL;
         break;
      case VK_BROWSER_FAVORITES: /* 0xAB */
         keyname = "XF86Favorites";
         key = "XF86Favorites";
         compose = NULL;
         break;
      case VK_BROWSER_HOME: /* 0xAC */
         keyname = "XF86HomePage";
         key = "XF86HomePage";
         compose = NULL;
         break;

      case VK_VOLUME_MUTE: /* 0xAD */
         keyname = "XF86AudioMute";
         key = "XF86AudioMute";
         compose = NULL;
         break;
      case VK_VOLUME_DOWN: /* 0xAE */
         keyname = "XF86AudioLowerVolume";
         key = "XF86AudioLowerVolume";
         compose = NULL;
         break;
      case VK_VOLUME_UP: /* 0xAF */
         keyname = "XF86AudioRaiseVolume";
         key = "XF86AudioRaiseVolume";
         compose = NULL;
         break;

      case VK_MEDIA_NEXT_TRACK: /* 0xB0 */
         keyname = "XF86AudioNext";
         key = "XF86AudioNext";
         compose = NULL;
         break;
      case VK_MEDIA_PREV_TRACK: /* 0xB1 */
         keyname = "XF86AudioPrev";
         key = "XF86AudioPrev";
         compose = NULL;
         break;
      case VK_MEDIA_STOP: /* 0xB2 */
         keyname = "XF86AudioStop";
         key = "XF86AudioStop";
         compose = NULL;
         break;
      case VK_MEDIA_PLAY_PAUSE: /* 0xB3 */
        {
           if (ECORE_WIN32_VK_IS_TOGGLED(VK_MEDIA_PLAY_PAUSE))
             {
                keyname = "XF86AudioPlay";
                key = "XF86AudioPlay";
                compose = NULL;
             }
           else
             {
                keyname = "XF86AudioPause";
                key = "XF86AudioPause";
                compose = NULL;
             }
           break;
        }
      case VK_LAUNCH_MAIL: /* 0xB4 */
         keyname = "XF86Mail";
         key = "XF86Mail";
         compose = NULL;
         break;

         /* VK_LAUNCH_MEDIA_SELECT: 0xB5 : select Media key */
         /* VK_LAUNCH_APP1: 0xB6 : start application 1 key */
         /* VK_LAUNCH_APP2: 0xB7 : start application 2 key */

         /* 0xB8 - 0xB9 : reserved */

         /* case VK_OEM1 : 0xBA : Used for miscellaneous characters; it can vary by keyboard. (default case) */
         /* case VK_PLUS : 0xBB : For any country/region, the '+' key. (default case) */
         /* case VK_COMMA : 0xBC : For any country/region, the ',' key. (default case) */
         /* case VK_MINUS : 0xBD : For any country/region, the '-' key. (default case) */
         /* case VK_PERIOD : 0xBE : For any country/region, the '.' key. (default case) */
         /* case VK_OEM2 : 0xBF : Used for miscellaneous characters; it can vary by keyboard. (default case) */
         /* case VK_OEM3 : 0xC0 : Used for miscellaneous characters; it can vary by keyboard. (default case) */

         /* 0xC1 - 0xD7 : reserved */

         /* 0xD8 - 0xDA : unassigned */

         /* case VK_OEM4 : 0xDB : Used for miscellaneous characters; it can vary by keyboard. (default case) */
         /* case VK_OEM5 : 0xDC : Used for miscellaneous characters; it can vary by keyboard. (default case) */
         /* case VK_OEM6 : 0xDD : Used for miscellaneous characters; it can vary by keyboard. (default case) */
         /* case VK_OEM7 : 0xDE : Used for miscellaneous characters; it can vary by keyboard. (default case) */
         /* case VK_OEM8 : 0xDF : Used for miscellaneous characters; it can vary by keyboard. (default case) */

         /* 0xE0 : reserved */

         /* 0xE1 : OEM specific */

         /* case VK_OEM102 : 0xE2 : Either the angle bracket key or the backslash key on the RT 102-key keyboard. (default case) */

         /* 0xE3 - 0xE4 : OEM specific */

         /* case VK_PROCESSKEY : 0xE5 : IME PROCESS key */

         /* 0xE6 : OEM specific */

         /* case VK_PACKET : 0xE7 : Used to pass Unicode characters as if they were keystrokes. */

         /* 0xE8 : unassigned */

         /* 0xE9 - 0xF5 : OEM specific */

         /* case VK_ATTN : 0xF6 : Attn key */
         /* case VK_CRSEL : 0xF7 : CrSel key */
         /* case VK_EXSEL : 0xF8 : ExSel key */
         /* case VK_EREOF : 0xF9 : Erase EOF key */
         /* case VK_PLAY : 0xFA : Play key */
         /* case VK_ZOOM : 0xFB : Zoom key */

         /* case VK_NONAME : 0xFC : reserved */

         /* case VK_PA1 : 0xFD : PA1 key */
         /* case VK_OEM_CLEAR : 0xFE : Clear key */
      default:
        {
           BYTE kbd_state[256];
           WCHAR buf[4];
           uint32_t offset;
           int res;
           unsigned short modifiers_save = 0;
           Eina_Bool is_dead_key;

           /*** compose and string field ***/

           if (!GetKeyboardState(kbd_state))
             return NULL;

           /*
            * Keep that trick in case it is necessary
            char_value = MapVirtualKey(msg->window_param, MAPVK_VK_TO_CHAR);
            _ecore_win32_is_dead_key = (char_value & 0x80000000) == 0x80000000;
           */
           res = ToUnicode(msg->window_param,
                           MapVirtualKey(msg->window_param, MAPVK_VK_TO_CHAR),
                           kbd_state, buf, 4, 0);
           if (res == -1)
             {
                /* dead key, but managed like normal key */
             }
           else if (res == 0)
             {
                INF("No translatable character found, skipping");
                return NULL;
             }
           else if (res >= 2)
             {
                /*
                 * Most common case : dead key which can not be composed,
                 * or proper symbol.
                 * Call ToUnicode again to get something sane.
                 */
                res = ToUnicode(msg->window_param,
                                MapVirtualKey(msg->window_param, MAPVK_VK_TO_CHAR),
                                kbd_state, buf, 4, 0);
                if ((res != 1) && (res != -1))
                  return NULL;
             }

           string[0] = (char)buf[0];
           compose = string;

           /*** key field ***/

           if (!_ecore_win32_ctrl_fake)
             {
                /* save modifiers for key */
                modifiers_save = 0;

                /* save Control modifier before getting key value */
                _ecore_win32_modifiers_ctrl_save(kbd_state, &modifiers_save);

                if (!SetKeyboardState(kbd_state))
                  return NULL;
             }

           is_dead_key = EINA_FALSE;
           res = ToUnicode(msg->window_param,
                           MapVirtualKey(msg->window_param, MAPVK_VK_TO_CHAR),
                           kbd_state, buf, 4, 0);
           if (res == -1)
             {
                is_dead_key = EINA_TRUE;
             }
           else if (res == 0)
             {
                INF("No translatable character found, skipping");
                return NULL;
             }
           else if (res >= 2)
             {
                /*
                 * Most common case : dead key which can not be composed,
                 * buf[0] is invalid and buf[1] is the dead key.
                 * Call ToUnicode again to get something sane.
                 */
                res = ToUnicode(msg->window_param,
                                MapVirtualKey(msg->window_param, MAPVK_VK_TO_CHAR),
                                kbd_state, buf, 4, 0);
                if (res == -1)
                  is_dead_key = EINA_TRUE;
                if ((res != 1) && (res != -1))
                  return NULL;
             }

           if (is_dead_key)
             key = _ecore_win32_diacritic_get(buf[0]);
           else
             {
                offset = _ecore_win32_keysym_offset_get(buf[0]);
                if (offset != 0xffffffff)
                  key = _ecore_win32_keysym_names + offset;
             }

           if (!_ecore_win32_ctrl_fake)
             {
                /* Restaure Control modifier */
                _ecore_win32_modifiers_ctrl_restore(kbd_state, modifiers_save);

                if (!SetKeyboardState(kbd_state))
                  return NULL;
             }

           if (!key)
             {
                WRN("no keysym found for keycode %d\n", string[0]);
                return NULL;
             }

           /*** keyname field ***/

           /* save modifiers for keyname */
           modifiers_save = 0;

           _ecore_win32_modifiers_alt_save(kbd_state, &modifiers_save);
           _ecore_win32_modifiers_ctrl_save(kbd_state, &modifiers_save);
           _ecore_win32_modifiers_shift_save(kbd_state, &modifiers_save);
           _ecore_win32_modifiers_win_save(kbd_state, &modifiers_save);

           if (!SetKeyboardState(kbd_state))
             return NULL;

           is_dead_key = EINA_FALSE;
           res = ToUnicode(msg->window_param,
                           MapVirtualKey(msg->window_param, MAPVK_VK_TO_CHAR),
                           kbd_state, buf, 4, 0);

           if (res == -1)
             {
                is_dead_key = EINA_TRUE;
             }
           else if (res == 0)
             {
                INF("No translatable character found, skipping");
                return NULL;
             }
           else if (res >= 2)
             {
                /*
                 * Most common case : dead key which can not be composed,
                 * buf[0] is invalid and buf[1] is the dead key.
                 * Call ToUnicode again to get something sane.
                 */
                res = ToUnicode(msg->window_param,
                                MapVirtualKey(msg->window_param, MAPVK_VK_TO_CHAR),
                                kbd_state, buf, 4, 0);
                if (res == -1)
                  is_dead_key = EINA_TRUE;
                if ((res != 1) && (res != -1))
                  return NULL;
             }

           if (is_dead_key)
             keyname = _ecore_win32_diacritic_get(buf[0]);
           else
             {
                offset = _ecore_win32_keysym_offset_get(buf[0]);
                if (offset != 0xffffffff)
                  keyname = _ecore_win32_keysym_names + offset;
             }

           /* Restore modifiers */
           _ecore_win32_modifiers_alt_restore(kbd_state, modifiers_save);
           _ecore_win32_modifiers_ctrl_restore(kbd_state, modifiers_save);
           _ecore_win32_modifiers_shift_restore(kbd_state, modifiers_save);
           _ecore_win32_modifiers_win_restore(kbd_state, modifiers_save);

           if (!SetKeyboardState(kbd_state))
             return NULL;

           if (!keyname)
             {
                WRN("no keysym found for keycode %d\n", string[0]);
                return NULL;
             }
        }
     }

   e = (Ecore_Event_Key *)calloc(1, sizeof(Ecore_Event_Key) +
                                 strlen(keyname) + 1 +
                                 strlen(key) + 1 +
                                 (compose ? (strlen(compose) + 1) : 0));
   if (!e)
     return NULL;

   e->keyname = (char *)(e + 1);
   e->key = e->keyname + strlen(keyname) + 1;
   e->compose = NULL;
   if (compose)
     e->compose = (e->key + strlen(key) + 1);
   e->string = e->compose;

   memcpy((char *)e->keyname, keyname, strlen(keyname));
   memcpy((char *)e->key, key, strlen(key));
   if (compose) memcpy((char *)e->compose, compose, strlen(compose));

   return e;
}

/***** Global functions definitions *****/

void
_ecore_win32_event_handle_key_press(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Event_Key *e = NULL;

   INF("key pressed");

   e = _ecore_win32_event_keystroke_get(msg, EINA_TRUE);
   if (!e)
     return;

   e->window = (Ecore_Window)GetWindowLongPtr(msg->window, GWLP_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }
   e->root_window = (Ecore_Window)GetAncestor(msg->window, GA_ROOT);
   e->event_window = e->window;
   e->same_screen = 1;
   e->timestamp = msg->timestamp;
   e->modifiers = _ecore_win32_modifiers_get();

   _ecore_win32_event_last_time = e->timestamp;

   ecore_event_add(ECORE_EVENT_KEY_DOWN, e, NULL, NULL);
}

void
_ecore_win32_event_handle_key_release(Ecore_Win32_Callback_Data *msg)
{
   Ecore_Event_Key *e;

   INF("key released");

   e = _ecore_win32_event_keystroke_get(msg, EINA_FALSE);
   if (!e)
     return;

   e->window = (Ecore_Window)GetWindowLongPtr(msg->window, GWLP_USERDATA);
   if (!e->window)
     {
        free(e);
        return;
     }
   e->root_window = (Ecore_Window)GetAncestor(msg->window, GA_ROOT);
   e->event_window = e->window;
   e->same_screen = 1;
   e->timestamp = msg->timestamp;
   e->modifiers = _ecore_win32_modifiers_get();

   _ecore_win32_event_last_time = e->timestamp;

   ecore_event_add(ECORE_EVENT_KEY_UP, e, NULL, NULL);
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
        e->modifiers = _ecore_win32_modifiers_get();

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
           e->modifiers = _ecore_win32_modifiers_get();

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
           e->modifiers = _ecore_win32_modifiers_get();

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
      e->modifiers = _ecore_win32_modifiers_get();

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
      e->modifiers = _ecore_win32_modifiers_get();

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
   e->modifiers = _ecore_win32_modifiers_get();

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
      e->modifiers = _ecore_win32_modifiers_get();

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
      e->modifiers = _ecore_win32_modifiers_get();

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
      e->modifiers = _ecore_win32_modifiers_get();

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
      e->modifiers = _ecore_win32_modifiers_get();

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
