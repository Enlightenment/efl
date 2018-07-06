#include <linux/input-event-codes.h>
#include <rfb/keysym.h>
#include <stdlib.h>
#include <limits.h>
#include <Ecore_Input.h>

static const char *_ecore_fb_li_kbd_syms[144 * 7] =
{
#include "ecore_fb_keytable.h"
};

#include "ecore_evas_vnc_server_fb_keymap.h"

static unsigned int
_x11_to_fb(rfbKeySym key, unsigned int *offset)
{
   switch (key)
     {
      case XK_Num_Lock:
         return KEY_NUMLOCK;
      case XK_KP_Delete:
         return KEY_DELETE;
      case XK_KP_Equal:
         return KEY_KPEQUAL;
      case XK_KP_Multiply:
         return KEY_KPASTERISK;
      case XK_KP_Add:
         return KEY_KPPLUS;
      case XK_KP_Subtract:
         return KEY_KPMINUS;
      case XK_KP_Decimal:
         return KEY_KPDOT;
      case XK_KP_Divide:
         return KEY_KPSLASH;
      case XK_plus:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_equal:
         return KEY_EQUAL;
      case XK_underscore:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_minus:
         return KEY_MINUS;
      case XK_quotedbl:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_apostrophe:
         return KEY_APOSTROPHE;
      case XK_Shift_L:
         return KEY_LEFTSHIFT;
      case XK_Shift_R:
         return KEY_RIGHTSHIFT;
      case XK_Control_L:
         return KEY_LEFTCTRL;
      case XK_Control_R:
         return KEY_RIGHTCTRL;
      case XK_Caps_Lock:
         return KEY_CAPSLOCK;
      case XK_Meta_L:
         return KEY_LEFTMETA;
      case XK_Meta_R:
         return KEY_RIGHTMETA;
      case XK_Alt_L:
         return KEY_LEFTALT;
      case XK_Alt_R:
         return KEY_RIGHTALT;
      case XK_space:
         return KEY_SPACE;
      case XK_period:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_greater:
         return KEY_DOT;
      case XK_bar:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_backslash:
         return KEY_BACKSLASH;
      case XK_question:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_slash:
         return KEY_SLASH;
      case XK_braceleft:
      case XK_bracketleft:
         return KEY_LEFTBRACE;
      case XK_braceright:
      case XK_bracketright:
         return KEY_RIGHTBRACE;
      case XK_colon:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_semicolon:
         return KEY_SEMICOLON;
      case XK_asciitilde:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_grave:
         return KEY_GRAVE;
      case XK_less:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_comma:
         return KEY_COMMA;
      case XK_F1:
         return KEY_F1;
      case XK_F2:
         return KEY_F2;
       case XK_F3:
         return KEY_F3;
      case XK_F4:
         return KEY_F4;
      case XK_F5:
         return KEY_F5;
      case XK_F6:
         return KEY_F6;
      case XK_F7:
         return KEY_F7;
      case XK_F8:
         return KEY_F8;
      case XK_F9:
         return KEY_F9;
      case XK_F10:
         return KEY_F10;
      case XK_F11:
         return KEY_F11;
      case XK_F12:
         return KEY_F12;
      case XK_BackSpace:
         return KEY_BACKSPACE;
      case XK_Tab:
         return KEY_TAB;
      case XK_Return:
         return KEY_ENTER;
      case XK_Pause:
         return KEY_PAUSE;
      case XK_Escape:
         return KEY_ESC;
      case XK_Delete:
         return KEY_DELETE;
      case XK_Linefeed:
         return KEY_LINEFEED;
      case XK_Scroll_Lock:
         return KEY_SCROLLLOCK;
      case XK_Sys_Req:
         return KEY_SYSRQ;
      case XK_Home:
         return KEY_HOME;
      case XK_Left:
         return KEY_LEFT;
      case XK_Up:
         return KEY_UP;
      case XK_Right:
         return KEY_RIGHT;
      case XK_Down:
         return KEY_DOWN;
      case XK_Page_Up:
         return KEY_PAGEUP;
      case XK_Page_Down:
         return KEY_PAGEDOWN;
      case XK_End:
         return KEY_END;
      case XK_KP_0:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_KP_Insert:
         return KEY_KP0;
      case XK_KP_1:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_KP_End:
         return KEY_KP1;
      case XK_KP_2:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_KP_Down:
         return KEY_KP2;
      case XK_KP_3:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_KP_Next:
         return KEY_KP3;
      case XK_KP_4:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_KP_Left:
         return KEY_KP4;
      case XK_KP_5:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_KP_Begin:
         return KEY_KP5;
      case XK_KP_6:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_KP_Right:
         return KEY_KP6;
      case XK_KP_7:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_KP_Home:
         return KEY_KP7;
      case XK_KP_8:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_KP_Up:
         return KEY_KP8;
      case XK_KP_9:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_KP_Prior:
         return KEY_KP9;
      case XK_KP_Enter:
         return KEY_KPENTER;
      case XK_parenright:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_0:
         return KEY_0;
      case XK_exclam:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_1:
         return KEY_1;
      case XK_at:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_2:
         return KEY_2;
      case XK_numbersign:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_3:
         return KEY_3;
      case XK_dollar:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_4:
         return KEY_4;
      case XK_percent:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_5:
         return KEY_5;
      case XK_asciicircum:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_6:
         return KEY_6;
      case XK_ampersand:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_7:
         return KEY_7;
      case XK_asterisk:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_8:
         return KEY_8;
      case XK_parenleft:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_9:
         return KEY_9;
      case XK_A:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_a:
         return KEY_A;
      case XK_B:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_b:
         return KEY_B;
      case XK_C:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_c:
         return KEY_C;
      case XK_D:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_d:
         return KEY_D;
      case XK_E:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_e:
         return KEY_E;
      case XK_F:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_f:
         return KEY_F;
      case XK_G:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_g:
         return KEY_G;
      case XK_H:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_h:
         return KEY_H;
      case XK_I:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_i:
         return KEY_I;
      case XK_J:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_j:
         return KEY_J;
      case XK_K:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_k:
         return KEY_K;
      case XK_L:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_l:
         return KEY_L;
      case XK_M:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_m:
         return KEY_M;
      case XK_N:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_n:
         return KEY_N;
      case XK_O:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_o:
         return KEY_O;
      case XK_P:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_p:
         return KEY_P;
      case XK_Q:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_q:
         return KEY_Q;
      case XK_R:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_r:
         return KEY_R;
      case XK_S:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_s:
         return KEY_S;
      case XK_T:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_t:
         return KEY_T;
      case XK_U:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_u:
         return KEY_U;
      case XK_V:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_v:
         return KEY_V;
      case XK_W:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_w:
         return KEY_W;
      case XK_X:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_x:
         return KEY_X;
      case XK_Y:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_y:
         return KEY_Y;
      case XK_Z:
         *offset = 1;
         EINA_FALLTHROUGH;
      case XK_z:
         return KEY_Z;
      default:
         return UINT_MAX;
     }
}

Eina_Bool
ecore_evas_vnc_server_keysym_to_fb_translate(rfbKeySym key,
                                             const char **key_name,
                                             const char **key_str,
                                             const char **compose)
{
   unsigned int offset = 0;
   unsigned int id = _x11_to_fb(key, &offset);

   if (id == UINT_MAX)
     return EINA_FALSE;

   *key_name = _ecore_fb_li_kbd_syms[id * 7];
   *key_str = _ecore_fb_li_kbd_syms[(id * 7) + offset];
   *compose = _ecore_fb_li_kbd_syms[(id* 7) + 3 + offset];
   return EINA_TRUE;
}
