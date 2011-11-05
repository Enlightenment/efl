#ifndef         ECORE_PSL1GHT_KEYS_H__
# define        ECORE_PSL1GHT_KEYS_H__

struct _ecore_psl1ght_keys_s
{
   int         code;
   const char *name;
   const char *compose;
};

static const struct _ecore_psl1ght_keys_s keystable[] =
{
   { KB_RAWKEY_NO_EVENT, "0x00", "" },
   { KB_RAWKEY_BS, "BackSpace", "\010" },
   { KB_RAWKEY_TAB, "Tab", "\011" },
   { KB_RAWKEY_ENTER, "Return", "\015" },
   { KB_RAWKEY_PAUSE, "Pause", "Pause" },
   { KB_RAWKEY_ESCAPE, "Escape", "\033" },
   { KB_RAWKEY_SPACE, "space", " " },

   /* Skip uppercase letters */
   { KB_RAWKEY_LEFT_BRACKET_101, "bracketleft", "[" },
   { KB_RAWKEY_BACKSLASH_101, "backslash", "\\" },
   { KB_RAWKEY_RIGHT_BRACKET_101, "bracketright", "]" },
   { KB_RAWKEY_ACCENT_CIRCONFLEX_106, "asciicircumm", "^" },
   { KB_RAWKEY_DELETE, "Delete", "\177" },
   /* End of ASCII mapped keysyms */

   /* Numeric keypad */
   { KB_RAWKEY_KPAD_0, "KP0", "0" },
   { KB_RAWKEY_KPAD_1, "KP1", "1" },
   { KB_RAWKEY_KPAD_2, "KP2", "2" },
   { KB_RAWKEY_KPAD_3, "KP3", "3" },
   { KB_RAWKEY_KPAD_4, "KP4", "4" },
   { KB_RAWKEY_KPAD_5, "KP5", "5" },
   { KB_RAWKEY_KPAD_6, "KP6", "6" },
   { KB_RAWKEY_KPAD_7, "KP7", "7" },
   { KB_RAWKEY_KPAD_8, "KP8", "8" },
   { KB_RAWKEY_KPAD_9, "KP9", "9" },
   { KB_RAWKEY_KPAD_PERIOD, "period", "." },
   { KB_RAWKEY_KPAD_SLASH, "KP_Divide", "/" },
   { KB_RAWKEY_KPAD_ASTERISK, "KP_Multiply", "*" },
   { KB_RAWKEY_KPAD_MINUS, "KP_Minus", "-" },
   { KB_RAWKEY_KPAD_PLUS, "KP_Plus", "+" },
   { KB_RAWKEY_KPAD_ENTER, "KP_Enter", "\015" },

   /* Arrows + Home/End pad */
   { KB_RAWKEY_UP_ARROW, "Up", "Up" },
   { KB_RAWKEY_DOWN_ARROW, "Down", "Down" },
   { KB_RAWKEY_RIGHT_ARROW, "Right", "Right" },
   { KB_RAWKEY_LEFT_ARROW, "Left", "Left" },
   { KB_RAWKEY_INSERT, "Insert", "Insert" },
   { KB_RAWKEY_HOME, "Home", "Home" },
   { KB_RAWKEY_END, "End", "End" },
   { KB_RAWKEY_PAGE_UP, "Page_Up", "Page_Up" },
   { KB_RAWKEY_PAGE_DOWN, "Page_Down", "Page_Down" },

   /* Function keys */
   { KB_RAWKEY_F1, "F1", "F1" },
   { KB_RAWKEY_F2, "F2", "F2" },
   { KB_RAWKEY_F3, "F3", "F3" },
   { KB_RAWKEY_F4, "F4", "F4" },
   { KB_RAWKEY_F5, "F5", "F5" },
   { KB_RAWKEY_F6, "F6", "F6" },
   { KB_RAWKEY_F7, "F7", "F7" },
   { KB_RAWKEY_F8, "F8", "F8" },
   { KB_RAWKEY_F9, "F9", "F9" },
   { KB_RAWKEY_F10, "F10", "F10" },
   { KB_RAWKEY_F11, "F11", "F11" },
   { KB_RAWKEY_F12, "F12", "F12" },

   /* Key state modifier keys */
   { KB_RAWKEY_KPAD_NUMLOCK, "Num_Lock", "Num_Lock" },
   { KB_RAWKEY_CAPS_LOCK, "Caps_Lock", "Caps_Lock" },
   { KB_RAWKEY_SCROLL_LOCK, "Scroll_Lock", "Scroll_Lock" },
};

#endif  /* ECORE_PSL1GHT_KEYS_H__ */
