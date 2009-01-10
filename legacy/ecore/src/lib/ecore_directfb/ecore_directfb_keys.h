typedef struct _Ecore_DirectFB_Key_Symbols Ecore_DirectFB_Key_Symbols;
struct _Ecore_DirectFB_Key_Symbols
{
	char *string;
	char *name;
	unsigned int id;
};

static const Ecore_DirectFB_Key_Symbols _ecore_directfb_key_symbols[] = {
	{"\010", "BackSpace",DIKS_BACKSPACE},
	{"\011", "Tab", DIKS_TAB},
	{"\015", "Return", DIKS_RETURN},
	{"", "Cancel", DIKS_CANCEL},
	{"", "Escape", DIKS_ESCAPE},
	{" ", "space", DIKS_SPACE},
	{"!", "exclam", DIKS_EXCLAMATION_MARK},
	{"\"", "quotedbl", DIKS_QUOTATION},
	{"#", "numbersign", DIKS_NUMBER_SIGN},
	{"$", "dollar", DIKS_DOLLAR_SIGN},
	{"%", "percent", DIKS_PERCENT_SIGN},
	{"&", "ampersand", DIKS_AMPERSAND},
	{"'", "apostrophe", DIKS_APOSTROPHE},
	{"(", "parenleft", DIKS_PARENTHESIS_LEFT},
	{")", "parenright", DIKS_PARENTHESIS_RIGHT},
	{"*", "asterisk", DIKS_ASTERISK},
	{"+", "plus", DIKS_PLUS_SIGN},
	{",", "comma", DIKS_COMMA},
	{"-", "minus", DIKS_MINUS_SIGN},
	{".", "period", DIKS_PERIOD},
	{"/", "slash", DIKS_SLASH},
	{"0", "0", DIKS_0},
	{"1", "1", DIKS_1},
	{"2", "2", DIKS_2},
	{"3", "3", DIKS_3},
	{"4", "4", DIKS_4},
	{"5", "5", DIKS_5},
	{"6", "6", DIKS_6},
	{"7", "7", DIKS_7},
	{"8", "8", DIKS_8},
	{"9", "9", DIKS_9},
	{":", "colon", DIKS_COLON},
	{";", "semicolon", DIKS_SEMICOLON},
	{"<", "less", DIKS_LESS_THAN_SIGN},
	{"=", "equal", DIKS_EQUALS_SIGN},
	{">", "greater", DIKS_GREATER_THAN_SIGN},
	{"?", "question", DIKS_QUESTION_MARK},
	{"@", "at", DIKS_AT},
	{"A", "A", DIKS_CAPITAL_A },
	{"B", "B", DIKS_CAPITAL_B },
	{"C", "C", DIKS_CAPITAL_C },
	{"D", "D", DIKS_CAPITAL_D },
	{"E", "E", DIKS_CAPITAL_E },
	{"F", "F", DIKS_CAPITAL_F },
	{"G", "G", DIKS_CAPITAL_G },
	{"H", "H", DIKS_CAPITAL_H },
	{"I", "I", DIKS_CAPITAL_I },
	{"J", "J", DIKS_CAPITAL_J },
	{"K", "K", DIKS_CAPITAL_K },
	{"L", "L", DIKS_CAPITAL_L },
	{"M", "M", DIKS_CAPITAL_M },
	{"N", "N", DIKS_CAPITAL_N },
	{"O", "O", DIKS_CAPITAL_O },
	{"P", "P", DIKS_CAPITAL_P },
	{"Q", "Q", DIKS_CAPITAL_Q },
	{"R", "R", DIKS_CAPITAL_R },
	{"S", "S", DIKS_CAPITAL_S },
	{"T", "T", DIKS_CAPITAL_T },
	{"U", "U", DIKS_CAPITAL_U },
	{"V", "V", DIKS_CAPITAL_V },
	{"W", "W", DIKS_CAPITAL_W },
	{"X", "X", DIKS_CAPITAL_X },
	{"Y", "Y", DIKS_CAPITAL_Y },
	{"Z", "Z", DIKS_CAPITAL_Z },
	{"[", "bracketleft", DIKS_SQUARE_BRACKET_LEFT },
	{"\\", "backslash", DIKS_BACKSLASH },
	{"]", "bracketright", DIKS_SQUARE_BRACKET_RIGHT },
	{"^", "asciicircum", DIKS_CIRCUMFLEX_ACCENT },
	{"_", "underscore", DIKS_UNDERSCORE },
	{"`", "grave", DIKS_GRAVE_ACCENT},
	{"a", "a", DIKS_SMALL_A },
	{"b","b", DIKS_SMALL_B },
	{"c","c", DIKS_SMALL_C },
	{"d","d", DIKS_SMALL_D },
	{"e","e", DIKS_SMALL_E },
	{"f","f", DIKS_SMALL_F },
	{"g","g", DIKS_SMALL_G },
	{"h","h", DIKS_SMALL_H },
	{"i","i", DIKS_SMALL_I },
	{"j","j", DIKS_SMALL_J },
	{"k","k", DIKS_SMALL_K },
	{"l","l", DIKS_SMALL_L },
	{"m","m", DIKS_SMALL_M },
	{"n","n", DIKS_SMALL_N },
	{"o", "o", DIKS_SMALL_O },
	{"p", "p", DIKS_SMALL_P },
	{"q", "q", DIKS_SMALL_Q },
	{"r", "r", DIKS_SMALL_R },
	{"s", "s", DIKS_SMALL_S },
	{"t", "t", DIKS_SMALL_T },
	{"u", "u", DIKS_SMALL_U },
	{"v", "v", DIKS_SMALL_V },
	{"w", "w", DIKS_SMALL_W },
	{"x", "x", DIKS_SMALL_X },
	{"y", "y", DIKS_SMALL_Y },
	{"z", "z", DIKS_SMALL_Z },
	{"{", "braceleft",DIKS_CURLY_BRACKET_LEFT },
	{"|", "bar", DIKS_VERTICAL_BAR },
	{"}", "braceright", DIKS_CURLY_BRACKET_RIGHT },
	{"~", "asciitilde", DIKS_TILDE },
	{"\177", "Delete", DIKS_DELETE },
	{"", "Left", DIKS_CURSOR_LEFT },
	{"", "Right", DIKS_CURSOR_RIGHT},
	{"", "Up", DIKS_CURSOR_UP},
	{"", "Down", DIKS_CURSOR_DOWN},
	{"", "Insert", DIKS_INSERT},
	{"", "Home", DIKS_HOME},
	{"", "End", DIKS_END},
	{"", "Page_Up", DIKS_PAGE_UP},
	{"", "Page_Down", DIKS_PAGE_DOWN},
	{"", "Print", DIKS_PRINT},
	{"", "Pause", DIKS_PAUSE},
	/* ok */
	{"", "Select",DIKS_SELECT},
	/* goto */
	{"", "Clear", DIKS_CLEAR},
	/* power */
	/* power 2 */
	/* option */
	{"", "Menu",DIKS_MENU},
	{"", "Help",DIKS_HELP},
	/* info */
	/* time */
	/* vendor */
	/* archive */
	/* program */
	/* channel */
	/* favorites */
	/* hasta next */
	{"", "Next",DIKS_NEXT},
	{"", "Begin",DIKS_BEGIN},
	/* digits */
	/* teen */
	/* twen */
	{"", "Break", DIKS_BREAK},
	/* exit */
	/* setup */
	{"", "upleftcorner", DIKS_CURSOR_LEFT_UP },
	{"", "lowleftcorner", DIKS_CURSOR_LEFT_DOWN },
	{"", "uprightcorner", DIKS_CURSOR_UP_RIGHT },
	{"", "lowrightcorner",DIKS_CURSOR_DOWN_RIGHT },
	{"", "F1",DIKS_F1},
	{"", "F2",DIKS_F2},
	{"", "F3",DIKS_F3},
	{"", "F4",DIKS_F4},
	{"", "F5",DIKS_F5},
	{"", "F6",DIKS_F6},
	{"", "F7",DIKS_F7},
	{"", "F8",DIKS_F8},
	{"", "F9",DIKS_F9},
	{"", "F10",DIKS_F10},
	{"", "F11",DIKS_F11},
	{"", "F12",DIKS_F12},
	/* this are only mapped to one, not left right */
	{"", "Shift_L", DIKS_SHIFT},
	/*{"Shift_R",0xFFE2},*/
	{"", "Control_L", DIKS_CONTROL},
	/*{"Control_R",0xFFE4},*/
	{"", "Meta_L", DIKS_META},
	/* {"Meta_R",0xFFE8},*/
	{"", "Alt_L", DIKS_ALT},
	{"", "Alt_R", DIKS_ALTGR},
	{"", "Super_L", DIKS_SUPER},
	/*{"Super_R",0xFFEC},*/
	{"", "Hyper_L", DIKS_HYPER},
	/*{"Hyper_R",0xFFEE},*/
		
	{"", "Caps_Lock", DIKS_CAPS_LOCK},
	{"", "Num_Lock", DIKS_NUM_LOCK},
	{"", "Scroll_Lock", DIKS_SCROLL_LOCK},
	/* not included the dead keys */
	/* not included the custom keys */
	{"", "VoidSymbol", DIKS_NULL}
};
static int _ecore_directfb_key_symbols_count = sizeof(_ecore_directfb_key_symbols)/sizeof(Ecore_DirectFB_Key_Symbols);
