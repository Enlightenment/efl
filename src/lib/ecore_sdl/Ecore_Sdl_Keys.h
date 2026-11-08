#ifndef         ECORE_SDL_KEYS_H__
# define        ECORE_SDL_KEYS_H__

struct _ecore_sdl_keys_s
{
   SDL_Keycode code;
   const char*  name;
   const char*  compose; /* NULL for keys that produce no character */
};

static const struct _ecore_sdl_keys_s  keystable[] =
{
   { SDLK_UNKNOWN,      "0x00",         NULL },
   { SDLK_BACKSPACE,    "BackSpace",    "\010" },
   { SDLK_TAB,          "Tab",          "\011" },
   { SDLK_CLEAR,        "Clear",        NULL },
   { SDLK_RETURN,       "Return",       "\015" },
   { SDLK_PAUSE,        "Pause",        NULL },
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
   { SDLK_UP,           "Up",           NULL },
   { SDLK_DOWN,         "Down",         NULL },
   { SDLK_RIGHT,        "Right",        NULL },
   { SDLK_LEFT,         "Left",         NULL },
   { SDLK_INSERT,       "Insert",       NULL },
   { SDLK_HOME,         "Home",         NULL },
   { SDLK_END,          "End",          NULL },
   { SDLK_PAGEUP,       "Page_Up",      NULL },
   { SDLK_PAGEDOWN,     "Page_Down",    NULL },

   /* Function keys */
   { SDLK_F1,           "F1",           NULL },
   { SDLK_F2,           "F2",           NULL },
   { SDLK_F3,           "F3",           NULL },
   { SDLK_F4,           "F4",           NULL },
   { SDLK_F5,           "F5",           NULL },
   { SDLK_F6,           "F6",           NULL },
   { SDLK_F7,           "F7",           NULL },
   { SDLK_F8,           "F8",           NULL },
   { SDLK_F9,           "F9",           NULL },
   { SDLK_F10,          "F10",          NULL },
   { SDLK_F11,          "F11",          NULL },
   { SDLK_F12,          "F12",          NULL },
   { SDLK_F13,          "F13",          NULL },
   { SDLK_F14,          "F14",          NULL },
   { SDLK_F15,          "F15",          NULL },

   /* Key state modifier keys */
   { SDLK_NUMLOCKCLEAR, "Num_Lock",     NULL },
   { SDLK_CAPSLOCK,     "Caps_Lock",    NULL },
   { SDLK_SCROLLLOCK,   "Scroll_Lock",  NULL },
   { SDLK_RSHIFT,       "Shift_R",      NULL },
   { SDLK_LSHIFT,       "Shift_L",      NULL },
   { SDLK_RCTRL,        "Control_R",    NULL },
   { SDLK_LCTRL,        "Control_L",    NULL },
   { SDLK_RALT,         "Alt_R",        NULL },
   { SDLK_LALT,         "Alt_L",        NULL },
   { SDLK_LGUI,         "Super_L",      NULL },   /* Left "Windows" key */
   { SDLK_RGUI,         "Super_R",      NULL },   /* Right "Windows" key */
   { SDLK_MODE,         "Mode",         NULL },   /* "Alt Gr" key */

   /* Miscellaneous function keys */
   { SDLK_HELP,         "Help",         NULL },
   { SDLK_PRINTSCREEN,  "Print",        NULL },
   { SDLK_SYSREQ,       "SysReq",       NULL },
   { SDLK_MENU,         "Menu",         NULL },
   { SDLK_POWER,        "Power",        NULL },   /* Power Macintosh power key */
   { SDLK_UNDO,         "Undo",         NULL }    /* Atari keyboard has Undo */
};

#endif  /* ECORE_SDL_KEYS_H__ */
