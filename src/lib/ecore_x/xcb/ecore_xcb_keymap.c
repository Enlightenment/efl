#include "ecore_xcb_private.h"
#define NEED_KEYSYM_TABLE
#define NEED_VTABLE
#include "ecore_xcb_keysym_table.h"
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>

/* local function prototypes */
static int          _ecore_xcb_keymap_mask_get(void        *reply,
                                               xcb_keysym_t sym);
static xcb_keysym_t _ecore_xcb_keymap_string_to_keysym(const char *str);
static int          _ecore_xcb_keymap_translate_key(xcb_keycode_t keycode,
                                                    unsigned int  modifiers,
                                                    unsigned int *modifiers_return,
                                                    xcb_keysym_t *keysym_return);
static int _ecore_xcb_keymap_translate_keysym(xcb_keysym_t keysym,
                                              unsigned int modifiers,
                                              char        *buffer,
                                              int          bytes);

/* local variables */
static xcb_key_symbols_t *_ecore_xcb_keysyms;
static int _ecore_xcb_mode_switch = 0;

/* public variables */
EAPI int ECORE_X_MODIFIER_SHIFT = 0;
EAPI int ECORE_X_MODIFIER_CTRL = 0;
EAPI int ECORE_X_MODIFIER_ALT = 0;
EAPI int ECORE_X_MODIFIER_WIN = 0;
EAPI int ECORE_X_MODIFIER_ALTGR = 0;
EAPI int ECORE_X_LOCK_SCROLL = 0;
EAPI int ECORE_X_LOCK_NUM = 0;
EAPI int ECORE_X_LOCK_CAPS = 0;
EAPI int ECORE_X_LOCK_SHIFT = 0;

void
_ecore_xcb_keymap_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_keysyms = xcb_key_symbols_alloc(_ecore_xcb_conn);
}

void
_ecore_xcb_keymap_finalize(void)
{
   xcb_get_modifier_mapping_cookie_t cookie;
   xcb_get_modifier_mapping_reply_t *reply;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   cookie = xcb_get_modifier_mapping_unchecked(_ecore_xcb_conn);
   reply = xcb_get_modifier_mapping_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply)
     {
        xcb_key_symbols_free(_ecore_xcb_keysyms);
        return;
     }

   _ecore_xcb_mode_switch = _ecore_xcb_keymap_mask_get(reply, XK_Mode_switch);

   ECORE_X_MODIFIER_SHIFT = _ecore_xcb_keymap_mask_get(reply, XK_Shift_L);
   ECORE_X_MODIFIER_CTRL = _ecore_xcb_keymap_mask_get(reply, XK_Control_L);

   ECORE_X_MODIFIER_ALT = _ecore_xcb_keymap_mask_get(reply, XK_Alt_L);
   if (!ECORE_X_MODIFIER_ALT)
     ECORE_X_MODIFIER_ALT = _ecore_xcb_keymap_mask_get(reply, XK_Meta_L);
   if (!ECORE_X_MODIFIER_ALT)
     ECORE_X_MODIFIER_ALT = _ecore_xcb_keymap_mask_get(reply, XK_Super_L);

   ECORE_X_MODIFIER_WIN = _ecore_xcb_keymap_mask_get(reply, XK_Super_L);
   if (!ECORE_X_MODIFIER_WIN)
     ECORE_X_MODIFIER_WIN = _ecore_xcb_keymap_mask_get(reply, XK_Meta_L);

   ECORE_X_MODIFIER_ALTGR = _ecore_xcb_keymap_mask_get(reply, XK_Mode_switch);

   if (ECORE_X_MODIFIER_WIN == ECORE_X_MODIFIER_ALT)
     ECORE_X_MODIFIER_WIN = 0;
   if (ECORE_X_MODIFIER_ALT == ECORE_X_MODIFIER_CTRL)
     ECORE_X_MODIFIER_ALT = 0;

   if (ECORE_X_MODIFIER_ALTGR)
     {
        if ((ECORE_X_MODIFIER_ALTGR == ECORE_X_MODIFIER_SHIFT) ||
            (ECORE_X_MODIFIER_ALTGR == ECORE_X_MODIFIER_CTRL) ||
            (ECORE_X_MODIFIER_ALTGR == ECORE_X_MODIFIER_ALT) ||
            (ECORE_X_MODIFIER_ALTGR == ECORE_X_MODIFIER_WIN))
          {
             ERR("ALTGR conflicts with other modifiers. IGNORE ALTGR");
             ECORE_X_MODIFIER_ALTGR = 0;
          }
     }

   if (ECORE_X_MODIFIER_ALT)
     {
        if ((ECORE_X_MODIFIER_ALT == ECORE_X_MODIFIER_SHIFT) ||
            (ECORE_X_MODIFIER_ALT == ECORE_X_MODIFIER_CTRL) ||
            (ECORE_X_MODIFIER_ALT == ECORE_X_MODIFIER_WIN))
          {
             ERR("ALT conflicts with other modifiers. IGNORE ALT");
             ECORE_X_MODIFIER_ALT = 0;
          }
     }

   if (ECORE_X_MODIFIER_WIN)
     {
        if ((ECORE_X_MODIFIER_WIN == ECORE_X_MODIFIER_SHIFT) ||
            (ECORE_X_MODIFIER_WIN == ECORE_X_MODIFIER_CTRL))
          {
             ERR("WIN conflicts with other modifiers. IGNORE WIN");
             ECORE_X_MODIFIER_WIN = 0;
          }
     }

   if (ECORE_X_MODIFIER_SHIFT)
     {
        if ((ECORE_X_MODIFIER_SHIFT == ECORE_X_MODIFIER_CTRL))
          {
             ERR("CTRL conflicts with other modifiers. IGNORE CTRL");
             ECORE_X_MODIFIER_CTRL = 0;
          }
     }

   ECORE_X_LOCK_SCROLL = _ecore_xcb_keymap_mask_get(reply, XK_Scroll_Lock);
   ECORE_X_LOCK_NUM = _ecore_xcb_keymap_mask_get(reply, XK_Num_Lock);
   ECORE_X_LOCK_CAPS = _ecore_xcb_keymap_mask_get(reply, XK_Caps_Lock);
   ECORE_X_LOCK_SHIFT = _ecore_xcb_keymap_mask_get(reply, XK_Shift_Lock);
}

void
_ecore_xcb_modifiers_get(void)
{
   _ecore_xcb_keymap_finalize();
}

void
_ecore_xcb_keymap_shutdown(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (_ecore_xcb_keysyms) xcb_key_symbols_free(_ecore_xcb_keysyms);
}

void
_ecore_xcb_keymap_refresh(xcb_mapping_notify_event_t *event)
{
   CHECK_XCB_CONN;
   xcb_refresh_keyboard_mapping(_ecore_xcb_keysyms, event);
}

xcb_keysym_t
_ecore_xcb_keymap_keycode_to_keysym(xcb_keycode_t keycode,
                                    int           col)
{
   xcb_keysym_t key0, key1;

   CHECK_XCB_CONN;
   if (col & _ecore_xcb_mode_switch)
     {
        key0 = xcb_key_symbols_get_keysym(_ecore_xcb_keysyms, keycode, 4);
        key1 = xcb_key_symbols_get_keysym(_ecore_xcb_keysyms, keycode, 5);
     }
   else
     {
        key0 = xcb_key_symbols_get_keysym(_ecore_xcb_keysyms, keycode, 0);
        key1 = xcb_key_symbols_get_keysym(_ecore_xcb_keysyms, keycode, 1);
     }

   if (key1 == XCB_NO_SYMBOL)
     key1 = key0;

   if ((col & ECORE_X_LOCK_NUM) &&
       ((xcb_is_keypad_key(key1)) || (xcb_is_private_keypad_key(key1))))
     {
        if ((col & XCB_MOD_MASK_SHIFT) ||
            ((col & XCB_MOD_MASK_LOCK) && (col & ECORE_X_LOCK_SHIFT)))
          return key0;
        else
          return key1;
     }
   else if (!(col & XCB_MOD_MASK_SHIFT) && !(col & XCB_MOD_MASK_LOCK))
     return key0;
   else if (!(col & XCB_MOD_MASK_SHIFT) &&
            (col & XCB_MOD_MASK_LOCK && (col & ECORE_X_LOCK_CAPS)))
     return key1;
   else if ((col & XCB_MOD_MASK_SHIFT) &&
            (col & XCB_MOD_MASK_LOCK) && (col & ECORE_X_LOCK_CAPS))
     return key0;
   else if ((col & XCB_MOD_MASK_SHIFT) ||
            (col & XCB_MOD_MASK_LOCK && (col & ECORE_X_LOCK_SHIFT)))
     return key1;

   return XCB_NO_SYMBOL;
}

xcb_keycode_t *
_ecore_xcb_keymap_keysym_to_keycode(xcb_keysym_t keysym)
{
   CHECK_XCB_CONN;
   return xcb_key_symbols_get_keycode(_ecore_xcb_keysyms, keysym);
}

char *
_ecore_xcb_keymap_keysym_to_string(xcb_keysym_t keysym)
{
   int i = 0, n = 0, h = 0, idx = 0;
   const unsigned char *entry;
   unsigned char val1, val2, val3, val4;

   CHECK_XCB_CONN;
   if (!keysym) return NULL;
   if (keysym == XK_VoidSymbol) keysym = 0;
   if (keysym <= 0x1fffffff)
     {
        val1 = (keysym >> 24);
        val2 = ((keysym >> 16) & 0xff);
        val3 = ((keysym >> 8) & 0xff);
        val4 = (keysym & 0xff);
        i = keysym % VTABLESIZE;
        h = i + 1;
        n = VMAXHASH;
        while ((idx = hashKeysym[i]))
          {
             entry = &_ecore_xcb_keytable[idx];
             if ((entry[0] == val1) && (entry[1] == val2) &&
                 (entry[2] == val3) && (entry[3] == val4))
               return (char *)entry + 4;
             if (!--n) break;
             i += h;
             if (i >= VTABLESIZE) i -= VTABLESIZE;
          }
     }

   if ((keysym >= 0x01000100) && (keysym <= 0x0110ffff))
     {
        xcb_keysym_t val;
        char *s = NULL;
        int i = 0;

        val = (keysym & 0xffffff);
        if (val & 0xff0000)
          i = 10;
        else
          i = 6;

        if (!(s = malloc(i))) return NULL;
        i--;
        s[i--] = '\0';
        for (; i; i--)
          {
             val1 = (val & 0xf);
             val >>= 4;
             if (val1 < 10)
               s[i] = '0' + val1;
             else
               s[i] = 'A' + val1 - 10;
          }
        s[i] = 'U';
        return s;
     }

   return NULL;
}

xcb_keycode_t
_ecore_xcb_keymap_string_to_keycode(const char *key)
{
   if (!strncmp(key, "Keycode-", 8))
     return atoi(key + 8);
   else
     {
        xcb_keysym_t keysym = XCB_NO_SYMBOL;
        xcb_keycode_t *keycodes, keycode = 0;
        int i = 0;

        CHECK_XCB_CONN;

        keysym = _ecore_xcb_keymap_string_to_keysym(key);
        if (keysym == XCB_NO_SYMBOL) return XCB_NO_SYMBOL;

        keycodes = _ecore_xcb_keymap_keysym_to_keycode(keysym);
        if (!keycodes) return XCB_NO_SYMBOL;

        while (keycodes[i] != XCB_NO_SYMBOL)
          {
             if (keycodes[i] != 0)
               {
                  keycode = keycodes[i];
                  break;
               }
             i++;
          }
        return keycode;
     }
}

int
_ecore_xcb_keymap_lookup_string(xcb_keycode_t keycode,
                                int           state,
                                char         *buffer,
                                int           bytes,
                                xcb_keysym_t *sym)
{
   unsigned int modifiers = 0;
   xcb_keysym_t keysym;

   CHECK_XCB_CONN;
   if (!_ecore_xcb_keymap_translate_key(keycode, state, &modifiers, &keysym))
     return 0;

   if (sym) *sym = keysym;

   return _ecore_xcb_keymap_translate_keysym(keysym, state, buffer, bytes);
}

EAPI const char *
ecore_x_keysym_string_get(int keysym)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_keymap_keysym_to_string(keysym);
}

EAPI int 
ecore_x_keysym_keycode_get(const char *keyname)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_keymap_string_to_keycode(keyname);
}

/* local functions */
static int
_ecore_xcb_keymap_mask_get(void        *reply,
                           xcb_keysym_t sym)
{
   xcb_get_modifier_mapping_reply_t *rep;
   xcb_keysym_t sym2;
   int mask = 0;
   const int masks[8] =
   {
      XCB_MOD_MASK_SHIFT, XCB_MOD_MASK_LOCK, XCB_MOD_MASK_CONTROL,
      XCB_MOD_MASK_1, XCB_MOD_MASK_2, XCB_MOD_MASK_3, XCB_MOD_MASK_4,
      XCB_MOD_MASK_5
   };

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   rep = (xcb_get_modifier_mapping_reply_t *)reply;
   if ((rep) && (rep->keycodes_per_modifier > 0))
     {
        int i = 0;
        xcb_keycode_t *modmap;

        modmap = xcb_get_modifier_mapping_keycodes(rep);
        for (i = 0; i < (8 * rep->keycodes_per_modifier); i++)
          {
             int j = 0;

             for (j = 0; j < 8; j++)
               {
                  sym2 = xcb_key_symbols_get_keysym(_ecore_xcb_keysyms,
                                                    modmap[i], j);
                  if (sym2 != 0) break;
               }
             if (sym2 == sym) mask = masks[i / rep->keycodes_per_modifier];
          }
     }
   return mask;
}

static xcb_keysym_t
_ecore_xcb_keymap_string_to_keysym(const char *str)
{
   int i = 0, n = 0, h = 0;
   unsigned long sig = 0;
   const char *p = NULL;
   int c = 0, idx = 0;
   const unsigned char *entry;
   unsigned char sig1, sig2;
   long unsigned int val;

   p = str;
   while ((c = *p++))
     sig = (sig << 1) + c;

   i = (sig % KTABLESIZE);
   h = i + 1;
   sig1 = (sig >> 8) & 0xff;
   sig2 = sig & 0xff;
   n = KMAXHASH;

   while ((idx = hashString[i]))
     {
        entry = &_ecore_xcb_keytable[idx];
        if ((entry[0] == sig1) && (entry[1] == sig2) &&
            !strcmp(str, (char *)entry + 6))
          {
             val = ((entry[2] << 24) | (entry[3] << 16) |
                    (entry[4] << 8) | (entry[5]));
             if (!val) val = 0xffffff;
             return val;
          }
        if (!--n) break;
        i += h;
        if (i >= KTABLESIZE) i -= KTABLESIZE;
     }

   if (*str == 'U')
     {
        val = 0;
        for (p = &str[1]; *p; p++)
          {
             c = *p;
             if (('0' <= c) && (c <= '9'))
               val = (val << 4) + c - '0';
             else if (('a' <= c) && (c <= 'f'))
               val = (val << 4) + c - 'a' + 10;
             else if (('A' <= c) && (c <= 'F'))
               val = (val << 4) + c - 'A' + 10;
             else
               return XCB_NO_SYMBOL;
             if (val > 0x10ffff) return XCB_NO_SYMBOL;
          }
        if ((val < 0x20) || ((val > 0x7e) && (val < 0xa0)))
          return XCB_NO_SYMBOL;
        if (val < 0x100) return val;
        return val | 0x01000000;
     }

   if ((strlen(str) > 2) && (str[0] == '0') && (str[1] == 'x'))
     {
        char *tmp = NULL;

        val = strtoul(str, &tmp, 16);
        if ((val == ULONG_MAX) || ((tmp) && (*tmp != '\0')))
          return XCB_NO_SYMBOL;
        else
          return val;
     }

   if (!strncmp(str, "XF86_", 5))
     {
        long unsigned int ret;
        char *tmp;

        tmp = strdup(str);
        if (!tmp) return XCB_NO_SYMBOL;
        memmove(&tmp[4], &tmp[5], strlen(str) - 5 + 1);
        ret = _ecore_xcb_keymap_string_to_keysym(tmp);
        free(tmp);
        return ret;
     }

   return XCB_NO_SYMBOL;
}

static int
_ecore_xcb_keymap_translate_key(xcb_keycode_t keycode,
                                unsigned int  modifiers,
                                unsigned int *modifiers_return,
                                xcb_keysym_t *keysym_return)
{
   xcb_keysym_t sym;

   if (!_ecore_xcb_keysyms) return 0;

   sym = _ecore_xcb_keymap_keycode_to_keysym(keycode, modifiers);

   if (modifiers_return)
     *modifiers_return = ((XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_LOCK) |
                          _ecore_xcb_mode_switch | ECORE_X_LOCK_NUM);
   if (keysym_return)
     *keysym_return = sym;

   return 1;
}

static int
_ecore_xcb_keymap_translate_keysym(xcb_keysym_t keysym,
                                   unsigned int modifiers,
                                   char        *buffer,
                                   int          bytes)
{
   unsigned long hbytes = 0;
   unsigned char c;

   if (!keysym) return 0;
   hbytes = (keysym >> 8);

   if (!(bytes &&
         ((hbytes == 0) ||
          ((hbytes == 0xFF) &&
           (((keysym >= XK_BackSpace) && (keysym <= XK_Clear)) ||
            (keysym == XK_Return) || (keysym == XK_Escape) ||
            (keysym == XK_KP_Space) || (keysym == XK_KP_Tab) ||
            (keysym == XK_KP_Enter) ||
            ((keysym >= XK_KP_Multiply) && (keysym <= XK_KP_9)) ||
            (keysym == XK_KP_Equal) || (keysym == XK_Delete))))))
     return 0;

   if (keysym == XK_KP_Space)
     c = (XK_space & 0x7F);
   else if (hbytes == 0xFF)
     c = (keysym & 0x7F);
   else
     c = (keysym & 0xFF);

   if (modifiers & ECORE_X_MODIFIER_CTRL)
     {
        if (((c >= '@') && (c < '\177')) || c == ' ')
          c &= 0x1F;
        else if (c == '2')
          c = '\000';
        else if ((c >= '3') && (c <= '7'))
          c -= ('3' - '\033');
        else if (c == '8')
          c = '\177';
        else if (c == '/')
          c = '_' & 0x1F;
     }
   buffer[0] = c;
   return 1;
}

