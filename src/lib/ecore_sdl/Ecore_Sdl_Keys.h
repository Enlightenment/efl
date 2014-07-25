#ifndef         ECORE_SDL_KEYS_H__
# define        ECORE_SDL_KEYS_H__

struct _ecore_sdl_keys_s
{
   SDL_Keycode code;
   const char*  name;
   const char*  compose;
};

static const struct _ecore_sdl_keys_s  keystable[] =
{
   { SDLK_UNKNOWN,      "0x00",         "" },
   { SDLK_BACKSPACE,    "BackSpace",    "\010" },
   { SDLK_TAB,          "Tab",          "\011" },
   { SDLK_CLEAR,        "Clear",        "Clear" },
   { SDLK_RETURN,       "Return",       "\015" },
   { SDLK_PAUSE,        "Pause",        "Pause" },
   { SDLK_ESCAPE,       "Escape",       "\033" },
   { SDLK_SPACE,        "space",        " " },
   { SDLK_EXCLAIM,      "exclam",       "!" },
   { SDLK_QUOTEDBL,     "quotedbl",     "\"" },
   { SDLK_HASH,         "numbersign",   "#" },
   { SDLK_DOLLAR,       "dollar",       "$" },
   { SDLK_AMPERSAND,    "ampersand",    "&" },
   { SDLK_QUOTE,        "apostrophe",   "'" },
   { SDLK_LEFTPAREN,    "parenleft",    "(" },
   { SDLK_RIGHTPAREN,   "parenright",   ")" },
   { SDLK_ASTERISK,     "asterisk",     "*" },
   { SDLK_PLUS,         "plus",         "+" },
   { SDLK_COMMA,        "comma",        "," },
   { SDLK_MINUS,        "minus",        "-" },
   { SDLK_PERIOD,       "period",       "." },
   { SDLK_SLASH,        "slash",        "/" },
   { SDLK_0,            "0",            "0" },
   { SDLK_1,            "1",            "1" },
   { SDLK_2,            "2",            "2" },
   { SDLK_3,            "3",            "3" },
   { SDLK_4,            "4",            "4" },
   { SDLK_5,            "5",            "5" },
   { SDLK_6,            "6",            "6" },
   { SDLK_7,            "7",            "7" },
   { SDLK_8,            "8",            "8" },
   { SDLK_9,            "9",            "9" },
   { SDLK_COLON,        "colon",        ":" },
   { SDLK_SEMICOLON,    "semicolon",    ";" },
   { SDLK_LESS,         "less",         "<" },
   { SDLK_EQUALS,       "equal",        "=" },
   { SDLK_GREATER,      "greater",      ">" },
   { SDLK_QUESTION,     "question",     "?" },
   { SDLK_AT,           "at",           "@" },

   /* Skip uppercase letters */
   { SDLK_LEFTBRACKET,  "bracketleft",  "[" },
   { SDLK_BACKSLASH,    "backslash",    "\\" },
   { SDLK_RIGHTBRACKET, "bracketright", "]" },
   { SDLK_CARET,        "asciicircumm", "^" },
   { SDLK_UNDERSCORE,   "underscore",   "_" },
   { SDLK_BACKQUOTE,    "asciitilde",   "`" },
   { SDLK_a,            "a",            "a" },
   { SDLK_b,            "b",            "b" },
   { SDLK_c,            "c",            "c" },
   { SDLK_d,            "d",            "d" },
   { SDLK_e,            "e",            "e" },
   { SDLK_f,            "f",            "f" },
   { SDLK_g,            "g",            "g" },
   { SDLK_h,            "h",            "h" },
   { SDLK_i,            "i",            "i" },
   { SDLK_j,            "j",            "j" },
   { SDLK_k,            "k",            "k" },
   { SDLK_l,            "l",            "l" },
   { SDLK_m,            "m",            "m" },
   { SDLK_n,            "n",            "n" },
   { SDLK_o,            "o",            "o" },
   { SDLK_p,            "p",            "p" },
   { SDLK_q,            "q",            "q" },
   { SDLK_r,            "r",            "r" },
   { SDLK_s,            "s",            "s" },
   { SDLK_t,            "t",            "t" },
   { SDLK_u,            "u",            "u" },
   { SDLK_v,            "v",            "v" },
   { SDLK_w,            "w",            "w" },
   { SDLK_x,            "x",            "x" },
   { SDLK_y,            "y",            "y" },
   { SDLK_z,            "z",            "z" },
   { SDLK_DELETE,       "Delete",       "\177" },
   /* End of ASCII mapped keysyms */

   /* Numeric keypad */
   { SDLK_KP_0,         "KP0",          "0" },
   { SDLK_KP_1,         "KP1",          "1" },
   { SDLK_KP_2,         "KP2",          "2" },
   { SDLK_KP_3,         "KP3",          "3" },
   { SDLK_KP_4,         "KP4",          "4" },
   { SDLK_KP_5,         "KP5",          "5" },
   { SDLK_KP_6,         "KP6",          "6" },
   { SDLK_KP_7,         "KP7",          "7" },
   { SDLK_KP_8,         "KP8",          "8" },
   { SDLK_KP_9,         "KP9",          "9" },
   { SDLK_KP_PERIOD,    "period",       "." },
   { SDLK_KP_DIVIDE,    "KP_Divide",    "/" },
   { SDLK_KP_MULTIPLY,  "KP_Multiply",  "*" },
   { SDLK_KP_MINUS,     "KP_Minus",     "-" },
   { SDLK_KP_PLUS,      "KP_Plus",      "+" },
   { SDLK_KP_ENTER,     "KP_Enter",     "\015" },
   { SDLK_KP_EQUALS,    "KP_Equals",    "=" },

   /* Arrows + Home/End pad */
   { SDLK_UP,           "Up",           "Up" },
   { SDLK_DOWN,         "Down",         "Down" },
   { SDLK_RIGHT,        "Right",        "Right" },
   { SDLK_LEFT,         "Left",         "Left" },
   { SDLK_INSERT,       "Insert",       "Insert" },
   { SDLK_HOME,         "Home",         "Home" },
   { SDLK_END,          "End",          "End" },
   { SDLK_PAGEUP,       "Page_Up",      "Page_Up" },
   { SDLK_PAGEDOWN,     "Page_Down",    "Page_Down" },

   /* Function keys */
   { SDLK_F1,           "F1",           "F1" },
   { SDLK_F2,           "F2",           "F2" },
   { SDLK_F3,           "F3",           "F3" },
   { SDLK_F4,           "F4",           "F4" },
   { SDLK_F5,           "F5",           "F5" },
   { SDLK_F6,           "F6",           "F6" },
   { SDLK_F7,           "F7",           "F7" },
   { SDLK_F8,           "F8",           "F8" },
   { SDLK_F9,           "F9",           "F9" },
   { SDLK_F10,          "F10",          "F10" },
   { SDLK_F11,          "F11",          "F11" },
   { SDLK_F12,          "F12",          "F12" },
   { SDLK_F13,          "F13",          "F13" },
   { SDLK_F14,          "F14",          "F14" },
   { SDLK_F15,          "F15",          "F15" },

   /* Key state modifier keys */
   { SDLK_NUMLOCKCLEAR, "Num_Lock",     "Num_Lock" },
   { SDLK_CAPSLOCK,     "Caps_Lock",    "Caps_Lock" },
   { SDLK_SCROLLLOCK,   "Scroll_Lock",  "Scroll_Lock" },
   { SDLK_RSHIFT,       "Shift_R",      "Shift_R" },
   { SDLK_LSHIFT,       "Shift_L",      "Shift_L" },
   { SDLK_RCTRL,        "Control_R",    "Control_R" },
   { SDLK_LCTRL,        "Control_L",    "Control_L" },
   { SDLK_RALT,         "Alt_R",        "Alt_R" },
   { SDLK_LALT,         "Alt_L",        "Alt_L" },
   { SDLK_LGUI,         "Super_L",      "Super_L" },   /* Left "Windows" key */
   { SDLK_RGUI,         "Super_R",      "Super_R" },   /* Right "Windows" key */
   { SDLK_MODE,         "Mode",         "Mode" },   /* "Alt Gr" key */

   /* Miscellaneous function keys */
   { SDLK_HELP,         "Help",         "Help" },
   { SDLK_PRINTSCREEN,  "Print",        "Print" },
   { SDLK_SYSREQ,       "SysReq",       "SysReq" },
   { SDLK_MENU,         "Menu",         "Menu" },
   { SDLK_POWER,        "Power",        "Power" },   /* Power Macintosh power key */
   { SDLK_UNDO,         "Undo",         "Undo" }    /* Atari keyboard has Undo */
};

#endif  /* ECORE_SDL_KEYS_H__ */
