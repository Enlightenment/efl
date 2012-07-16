#ifndef _ECORE_IMF_H
#define _ECORE_IMF_H

#include <Eina.h>

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
# endif /* ! EFL_ECORE_IMF_BUILD */
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

/**
 * @addtogroup Ecore_IMF_Context_Group
 *
 * @{
 */
/* ecore_imf_context_input_panel_event_callback_add() flag */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_STATE_EVENT,              /**< called when the state of the input panel is changed. @since 1.7 */
   ECORE_IMF_INPUT_PANEL_LANGUAGE_EVENT,           /**< called when the language of the input panel is changed. @since 1.7 */
   ECORE_IMF_INPUT_PANEL_SHIFT_MODE_EVENT,         /**< called when the shift key state of the input panel is changed @since 1.7 */
   ECORE_IMF_INPUT_PANEL_GEOMETRY_EVENT,           /**< called when the size of the input panel is changed. @since 1.7 */
   ECORE_IMF_CANDIDATE_PANEL_STATE_EVENT,          /**< called when the state of the candidate word panel is changed. @since 1.7 */
   ECORE_IMF_CANDIDATE_PANEL_GEOMETRY_EVENT        /**< called when the size of the candidate word panel is changed. @since 1.7 */
} Ecore_IMF_Input_Panel_Event;

typedef enum
{
   ECORE_IMF_INPUT_PANEL_STATE_SHOW,        /**< Notification after the display of the input panel @since 1.7 */
   ECORE_IMF_INPUT_PANEL_STATE_HIDE,        /**< Notification prior to the dismissal of the input panel @since 1.7 */
   ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW    /**< Notification prior to the display of the input panel @since 1.7 */
} Ecore_IMF_Input_Panel_State;

typedef enum
{
    ECORE_IMF_INPUT_PANEL_SHIFT_MODE_OFF, /**< @since 1.7 */
    ECORE_IMF_INPUT_PANEL_SHIFT_MODE_ON   /**< @since 1.7 */
} Ecore_IMF_Input_Panel_Shift_Mode;

typedef enum
{
   ECORE_IMF_CANDIDATE_PANEL_SHOW,        /**< Notification after the display of the candidate word panel @since 1.7 */
   ECORE_IMF_CANDIDATE_PANEL_HIDE         /**< Notification prior to the dismissal of the candidate word panel @since 1.7 */
} Ecore_IMF_Candidate_Panel_State;

/* Events sent by the Input Method */
typedef struct _Ecore_IMF_Event_Preedit_Start      Ecore_IMF_Event_Preedit_Start;
typedef struct _Ecore_IMF_Event_Preedit_End        Ecore_IMF_Event_Preedit_End;
typedef struct _Ecore_IMF_Event_Preedit_Changed    Ecore_IMF_Event_Preedit_Changed;
typedef struct _Ecore_IMF_Event_Commit             Ecore_IMF_Event_Commit;
typedef struct _Ecore_IMF_Event_Delete_Surrounding Ecore_IMF_Event_Delete_Surrounding;

/* Events to filter */
typedef struct _Ecore_IMF_Event_Mouse_Down         Ecore_IMF_Event_Mouse_Down;
typedef struct _Ecore_IMF_Event_Mouse_Up           Ecore_IMF_Event_Mouse_Up;
typedef struct _Ecore_IMF_Event_Mouse_In           Ecore_IMF_Event_Mouse_In;
typedef struct _Ecore_IMF_Event_Mouse_Out          Ecore_IMF_Event_Mouse_Out;
typedef struct _Ecore_IMF_Event_Mouse_Move         Ecore_IMF_Event_Mouse_Move;
typedef struct _Ecore_IMF_Event_Mouse_Wheel        Ecore_IMF_Event_Mouse_Wheel;
typedef struct _Ecore_IMF_Event_Key_Down           Ecore_IMF_Event_Key_Down;
typedef struct _Ecore_IMF_Event_Key_Up             Ecore_IMF_Event_Key_Up;
typedef union  _Ecore_IMF_Event                    Ecore_IMF_Event;

typedef struct _Ecore_IMF_Context                  Ecore_IMF_Context;                  /**< An Input Method Context */
typedef struct _Ecore_IMF_Context_Class            Ecore_IMF_Context_Class;            /**< An Input Method Context class */
typedef struct _Ecore_IMF_Context_Info             Ecore_IMF_Context_Info;             /**< An Input Method Context info */

/* Preedit attribute info */
typedef struct _Ecore_IMF_Preedit_Attr             Ecore_IMF_Preedit_Attr;

EAPI extern int ECORE_IMF_EVENT_PREEDIT_START;
EAPI extern int ECORE_IMF_EVENT_PREEDIT_END;
EAPI extern int ECORE_IMF_EVENT_PREEDIT_CHANGED;
EAPI extern int ECORE_IMF_EVENT_COMMIT;
EAPI extern int ECORE_IMF_EVENT_DELETE_SURROUNDING;

typedef void (*Ecore_IMF_Event_Cb) (void *data, Ecore_IMF_Context *ctx, void *event_info);

/**
 * @typedef Ecore_IMF_Callback_Type
 *
 * Ecore IMF Event callback types.
 *
 * @see ecore_imf_context_event_callback_add()
 */
typedef enum
{
   ECORE_IMF_CALLBACK_PREEDIT_START,      /**< "PREEDIT_START" is called when a new preediting sequence starts. @since 1.2 */
   ECORE_IMF_CALLBACK_PREEDIT_END,        /**< "PREEDIT_END" is called when a preediting sequence has been completed or canceled. @since 1.2 */
   ECORE_IMF_CALLBACK_PREEDIT_CHANGED,    /**< "PREEDIT_CHANGED" is called whenever the preedit sequence currently being entered has changed. @since 1.2 */
   ECORE_IMF_CALLBACK_COMMIT,             /**< "COMMIT" is called when a complete input sequence has been entered by the user @since 1.2 */
   ECORE_IMF_CALLBACK_DELETE_SURROUNDING  /**< "DELETE_SURROUNDING" is called when the input method needs to delete all or part of the context surrounding the cursor @since 1.2 */
} Ecore_IMF_Callback_Type;

/**
 * @typedef Ecore_IMF_Event_Type
 *
 * Ecore IMF event types.
 *
 * @see ecore_imf_context_filter_event()
 */
typedef enum
{
   ECORE_IMF_EVENT_MOUSE_DOWN,  /**< Mouse Down event */
   ECORE_IMF_EVENT_MOUSE_UP,    /**< Mouse Up event */
   ECORE_IMF_EVENT_MOUSE_IN,    /**< Mouse In event */
   ECORE_IMF_EVENT_MOUSE_OUT,   /**< Mouse Out event */
   ECORE_IMF_EVENT_MOUSE_MOVE,  /**< Mouse Move event */
   ECORE_IMF_EVENT_MOUSE_WHEEL, /**< Mouse Wheel event */
   ECORE_IMF_EVENT_KEY_DOWN,    /**< Key Down event */
   ECORE_IMF_EVENT_KEY_UP       /**< Key Up event */
} Ecore_IMF_Event_Type;
/**
 * @typedef Ecore_IMF_Keyboard_Modifiers
 * Type for Ecore_IMF keyboard modifiers
 */
typedef enum
{
   ECORE_IMF_KEYBOARD_MODIFIER_NONE  = 0,      /**< No active modifiers */
   ECORE_IMF_KEYBOARD_MODIFIER_CTRL  = 1 << 0, /**< "Control" is pressed */
   ECORE_IMF_KEYBOARD_MODIFIER_ALT   = 1 << 1, /**< "Alt" is pressed */
   ECORE_IMF_KEYBOARD_MODIFIER_SHIFT = 1 << 2, /**< "Shift" is pressed */
   ECORE_IMF_KEYBOARD_MODIFIER_WIN   = 1 << 3, /**< "Win" (between "Ctrl" and "Alt") is pressed */
   ECORE_IMF_KEYBOARD_MODIFIER_ALTGR = 1 << 4  /**< "AltGr" is pressed @since 1.7 */
} Ecore_IMF_Keyboard_Modifiers;

/**
 * @typedef Ecore_IMF_Keyboard_Locks
 * Type for Ecore_IMF keyboard locks
 */
typedef enum
{
   ECORE_IMF_KEYBOARD_LOCK_NONE      = 0,      /**< No locks are active */
   ECORE_IMF_KEYBOARD_LOCK_NUM       = 1 << 0, /**< "Num" lock is active */
   ECORE_IMF_KEYBOARD_LOCK_CAPS      = 1 << 1, /**< "Caps" lock is active */
   ECORE_IMF_KEYBOARD_LOCK_SCROLL    = 1 << 2  /**< "Scroll" lock is active */
} Ecore_IMF_Keyboard_Locks;

/**
 * @typedef Ecore_IMF_Mouse_Flags
 * Type for Ecore_IMF mouse flags
 */
typedef enum
{
   ECORE_IMF_MOUSE_NONE              = 0,      /**< A single click */
   ECORE_IMF_MOUSE_DOUBLE_CLICK      = 1 << 0, /**< A double click */
   ECORE_IMF_MOUSE_TRIPLE_CLICK      = 1 << 1  /**< A triple click */
} Ecore_IMF_Mouse_Flags;

typedef enum
{
   ECORE_IMF_INPUT_MODE_ALPHA        = 1 << 0,
   ECORE_IMF_INPUT_MODE_NUMERIC      = 1 << 1,
   ECORE_IMF_INPUT_MODE_SPECIAL      = 1 << 2,
   ECORE_IMF_INPUT_MODE_HEXA         = 1 << 3,
   ECORE_IMF_INPUT_MODE_TELE         = 1 << 4,
   ECORE_IMF_INPUT_MODE_FULL         = (ECORE_IMF_INPUT_MODE_ALPHA | ECORE_IMF_INPUT_MODE_NUMERIC | ECORE_IMF_INPUT_MODE_SPECIAL),
   ECORE_IMF_INPUT_MODE_INVISIBLE    = 1 << 29,
   ECORE_IMF_INPUT_MODE_AUTOCAP      = 1 << 30
} Ecore_IMF_Input_Mode;

/**
 * @typedef Ecore_IMF_Preedit_Type
 *
 * Ecore IMF Preedit style types
 *
 * @see ecore_imf_context_preedit_string_with_attributes_get()
 */
typedef enum
{
   ECORE_IMF_PREEDIT_TYPE_NONE, /**< None style @since 1.1 */
   ECORE_IMF_PREEDIT_TYPE_SUB1, /**< Substring style 1 @since 1.1 */
   ECORE_IMF_PREEDIT_TYPE_SUB2, /**< Substring style 2 @since 1.1 */
   ECORE_IMF_PREEDIT_TYPE_SUB3  /**< Substring style 3 @since 1.1 */
} Ecore_IMF_Preedit_Type;

/**
 * @typedef Ecore_IMF_Autocapital_Type
 *
 * Autocapitalization Types.
 *
 * @see ecore_imf_context_autocapital_type_set()
 */
typedef enum
{
   ECORE_IMF_AUTOCAPITAL_TYPE_NONE,         /**< No auto-capitalization when typing @since 1.1 */
   ECORE_IMF_AUTOCAPITAL_TYPE_WORD,         /**< Autocapitalize each word typed @since 1.1 */
   ECORE_IMF_AUTOCAPITAL_TYPE_SENTENCE,     /**< Autocapitalize the start of each sentence @since 1.1 */
   ECORE_IMF_AUTOCAPITAL_TYPE_ALLCHARACTER, /**< Autocapitalize all letters @since 1.1 */
} Ecore_IMF_Autocapital_Type;

/**
 * @typedef Ecore_IMF_Input_Panel_Layout
 *
 * Input panel (virtual keyboard) layout types.
 *
 * @see ecore_imf_context_input_panel_layout_set()
 */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL,          /**< Default layout */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBER,          /**< Number layout */
   ECORE_IMF_INPUT_PANEL_LAYOUT_EMAIL,           /**< Email layout */
   ECORE_IMF_INPUT_PANEL_LAYOUT_URL,             /**< URL layout */
   ECORE_IMF_INPUT_PANEL_LAYOUT_PHONENUMBER,     /**< Phone Number layout */
   ECORE_IMF_INPUT_PANEL_LAYOUT_IP,              /**< IP layout */
   ECORE_IMF_INPUT_PANEL_LAYOUT_MONTH,           /**< Month layout */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY,      /**< Number Only layout */
   ECORE_IMF_INPUT_PANEL_LAYOUT_INVALID,         /**< Never use this */
   ECORE_IMF_INPUT_PANEL_LAYOUT_HEX,             /**< Hexadecimal layout @since 1.2 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_TERMINAL,        /**< Command-line terminal layout @since 1.2 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD         /**< Like normal, but no auto-correct, no auto-capitalization etc. @since 1.2 */
} Ecore_IMF_Input_Panel_Layout;

/**
 * @typedef Ecore_IMF_Input_Panel_Lang
 *
 * Input panel (virtual keyboard) language modes.
 *
 * @see ecore_imf_context_input_panel_language_set()
 */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_LANG_AUTOMATIC,    /**< Automatic @since 1.2 */
   ECORE_IMF_INPUT_PANEL_LANG_ALPHABET      /**< Alphabet @since 1.2 */
} Ecore_IMF_Input_Panel_Lang;

/**
 * @typedef Ecore_IMF_Input_Panel_Return_Key_Type
 *
 * "Return" Key types on the input panel (virtual keyboard).
 *
 * @see ecore_imf_context_input_panel_return_key_type_set()
 */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT, /**< Default @since 1.2 */
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE,    /**< Done @since 1.2 */
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_GO,      /**< Go @since 1.2 */
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_JOIN,    /**< Join @since 1.2 */
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_LOGIN,   /**< Login @since 1.2 */
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_NEXT,    /**< Next @since 1.2 */
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH,  /**< Search or magnifier icon @since 1.2 */
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEND     /**< Send @since 1.2 */
} Ecore_IMF_Input_Panel_Return_Key_Type;

struct _Ecore_IMF_Event_Preedit_Start
{
   Ecore_IMF_Context *ctx;
};

struct _Ecore_IMF_Event_Preedit_End
{
   Ecore_IMF_Context *ctx;
};

struct _Ecore_IMF_Event_Preedit_Changed
{
   Ecore_IMF_Context *ctx;
};

struct _Ecore_IMF_Event_Commit
{
   Ecore_IMF_Context *ctx;
   char              *str;
};

struct _Ecore_IMF_Event_Delete_Surrounding
{
   Ecore_IMF_Context *ctx;
   int                offset;
   int                n_chars;
};

struct _Ecore_IMF_Event_Mouse_Down
{
   int button;                             /**< The button which has been pressed */
   struct {
      int x, y;
   } output;
   struct {
      int x, y;
   } canvas;
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The keyboard modifiers active when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The keyboard locks active when the event has been emitted */
   Ecore_IMF_Mouse_Flags        flags;     /**< The flags corresponding the mouse click (single, double or triple click) */
   unsigned int                 timestamp; /**< The timestamp when the event occurred */
};

struct _Ecore_IMF_Event_Mouse_Up
{
   int button;                             /**< The button which has been pressed */
   struct {
      int x, y;
   } output;
   struct {
      int x, y;
   } canvas;
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The keyboard modifiers active when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The keyboard locks active when the event has been emitted */
   Ecore_IMF_Mouse_Flags        flags;     /**< The flags corresponding the mouse click (single, double or triple click) */
   unsigned int                 timestamp; /**< The timestamp when the event occurred */
};

struct _Ecore_IMF_Event_Mouse_In
{
   int buttons;
   struct {
      int x, y;
   } output;
   struct {
      int x, y;
   } canvas;
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The keyboard modifiers active when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The keyboard locks active when the event has been emitted */
   unsigned int                 timestamp; /**< The timestamp when the event occurred */
};

struct _Ecore_IMF_Event_Mouse_Out
{
   int buttons;
   struct {
      int x, y;
   } output;
   struct {
      int x, y;
   } canvas;
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The keyboard modifiers active when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The keyboard locks active when the event has been emitted */
   unsigned int                 timestamp; /**< The timestamp when the event occurred */
};

struct _Ecore_IMF_Event_Mouse_Move
{
   int buttons;
   struct {
      struct {
         int x, y;
      } output;
      struct {
         int x, y;
      } canvas;
   } cur, prev;
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The keyboard modifiers active when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The keyboard locks active when the event has been emitted */
   unsigned int                 timestamp; /**< The timestamp when the event occurred */
};

struct _Ecore_IMF_Event_Mouse_Wheel
{
   int direction;                         /* 0 = default up/down wheel */
   int z;                                 /* ...,-2,-1 = down, 1,2,... = up */
   struct {
      int x, y;
   } output;
   struct {
      int x, y;
   } canvas;
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The keyboard modifiers active when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The keyboard locks active when the event has been emitted */
   unsigned int                 timestamp; /**< The timestamp when the event occurred */
};

struct _Ecore_IMF_Event_Key_Down
{
   const char                   *keyname;   /**< The string name of the key pressed */
   Ecore_IMF_Keyboard_Modifiers  modifiers; /**< The keyboard modifiers active when the event has been emitted */
   Ecore_IMF_Keyboard_Locks      locks;     /**< The keyboard locks active when the event has been emitted */
   const char                   *key;       /**< The logical key : (eg shift+1 == exclamation) */
   const char                   *string;    /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char                   *compose;   /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int                  timestamp; /**< The timestamp when the event occurred */
};

struct _Ecore_IMF_Event_Key_Up
{
   const char                   *keyname;   /**< The string name of the key pressed */
   Ecore_IMF_Keyboard_Modifiers  modifiers; /**< The keyboard modifiers active when the event has been emitted */
   Ecore_IMF_Keyboard_Locks      locks;     /**< The keyboard locks active when the event has been emitted */
   const char                   *key;       /**< The logical key : (eg shift+1 == exclamation) */
   const char                   *string;    /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char                   *compose;   /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int                  timestamp; /**< The timestamp when the event occurred */
};

union _Ecore_IMF_Event
{
   Ecore_IMF_Event_Mouse_Down  mouse_down;
   Ecore_IMF_Event_Mouse_Up    mouse_up;
   Ecore_IMF_Event_Mouse_In    mouse_in;
   Ecore_IMF_Event_Mouse_Out   mouse_out;
   Ecore_IMF_Event_Mouse_Move  mouse_move;
   Ecore_IMF_Event_Mouse_Wheel mouse_wheel;
   Ecore_IMF_Event_Key_Down    key_down;
   Ecore_IMF_Event_Key_Up      key_up;
};

struct _Ecore_IMF_Preedit_Attr
{
   Ecore_IMF_Preedit_Type preedit_type; /**< preedit style type */
   unsigned int start_index;            /**< start index of the range (in bytes) */
   unsigned int end_index;              /**< end index of the range (in bytes) */
};

struct _Ecore_IMF_Context_Class
{
   void (*add)                 (Ecore_IMF_Context *ctx);
   void (*del)                 (Ecore_IMF_Context *ctx);
   void (*client_window_set)   (Ecore_IMF_Context *ctx, void *window);
   void (*client_canvas_set)   (Ecore_IMF_Context *ctx, void *canvas);
   void (*show)                (Ecore_IMF_Context *ctx);
   void (*hide)                (Ecore_IMF_Context *ctx);
   void (*preedit_string_get)  (Ecore_IMF_Context *ctx, char **str, int *cursor_pos);
   void (*focus_in)            (Ecore_IMF_Context *ctx);
   void (*focus_out)           (Ecore_IMF_Context *ctx);
   void (*reset)               (Ecore_IMF_Context *ctx);
   void (*cursor_position_set) (Ecore_IMF_Context *ctx, int cursor_pos);
   void (*use_preedit_set)     (Ecore_IMF_Context *ctx, Eina_Bool use_preedit);
   void (*input_mode_set)      (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode);
   Eina_Bool (*filter_event)   (Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event);
   void (*preedit_string_with_attributes_get) (Ecore_IMF_Context *ctx, char **str, Eina_List **attrs, int *cursor_pos);
   void (*prediction_allow_set)(Ecore_IMF_Context *ctx, Eina_Bool prediction);
   void (*autocapital_type_set)(Ecore_IMF_Context *ctx, Ecore_IMF_Autocapital_Type autocapital_type);
   void (*control_panel_show)   (Ecore_IMF_Context *ctx);
   void (*control_panel_hide)   (Ecore_IMF_Context *ctx);
   void (*input_panel_layout_set) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Layout layout);
   Ecore_IMF_Input_Panel_Layout (*input_panel_layout_get) (Ecore_IMF_Context *ctx);
   void (*input_panel_language_set) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Lang lang);
   Ecore_IMF_Input_Panel_Lang (*input_panel_language_get) (Ecore_IMF_Context *ctx);
   void (*cursor_location_set) (Ecore_IMF_Context *ctx, int x, int y, int w, int h);
   void (*input_panel_imdata_set)(Ecore_IMF_Context *ctx, const void* data, int len);
   void (*input_panel_imdata_get)(Ecore_IMF_Context *ctx, void* data, int *len);
   void (*input_panel_return_key_type_set) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Return_Key_Type return_key_type);
   void (*input_panel_return_key_disabled_set) (Ecore_IMF_Context *ctx, Eina_Bool disabled);
   void (*input_panel_caps_lock_mode_set) (Ecore_IMF_Context *ctx, Eina_Bool mode);
   void (*input_panel_geometry_get)(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h);
   Ecore_IMF_Input_Panel_State (*input_panel_state_get) (Ecore_IMF_Context *ctx);
   void (*input_panel_event_callback_add) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, void (*func) (void *data, Ecore_IMF_Context *ctx, int value), void *data);
   void (*input_panel_event_callback_del) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, void (*func) (void *data, Ecore_IMF_Context *ctx, int value));
   void (*input_panel_language_locale_get) (Ecore_IMF_Context *ctx, char **lang);
   void (*candidate_panel_geometry_get)(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h);
};

struct _Ecore_IMF_Context_Info
{
   const char *id;              /* ID */
   const char *description;     /* Human readable description */
   const char *default_locales; /* Languages for which this context is the default, separated by : */
   const char *canvas_type;     /* The canvas type used by the input method. Eg.: evas */
   int         canvas_required; /* Whether the canvas usage is required for this input method */
};

/**
 * @}
 */

EAPI int                           ecore_imf_init(void);
EAPI int                           ecore_imf_shutdown(void);

EAPI void                          ecore_imf_module_register(const Ecore_IMF_Context_Info *info, Ecore_IMF_Context *(*imf_module_create)(void), Ecore_IMF_Context *(*imf_module_exit)(void));

EAPI Eina_List                    *ecore_imf_context_available_ids_get(void);
EAPI Eina_List                    *ecore_imf_context_available_ids_by_canvas_type_get(const char *canvas_type);
EAPI const char                   *ecore_imf_context_default_id_get(void);
EAPI const char                   *ecore_imf_context_default_id_by_canvas_type_get(const char *canvas_type);
EAPI const Ecore_IMF_Context_Info *ecore_imf_context_info_by_id_get(const char *id);

EAPI Ecore_IMF_Context            *ecore_imf_context_add(const char *id);
EAPI const Ecore_IMF_Context_Info *ecore_imf_context_info_get(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_del(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_client_window_set(Ecore_IMF_Context *ctx, void *window);
EAPI void                         *ecore_imf_context_client_window_get(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_client_canvas_set(Ecore_IMF_Context *ctx, void *canvas);
EAPI void                         *ecore_imf_context_client_canvas_get(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_show(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_hide(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_preedit_string_get(Ecore_IMF_Context *ctx, char **str, int *cursor_pos);
EAPI void                          ecore_imf_context_preedit_string_with_attributes_get(Ecore_IMF_Context *ctx, char **str, Eina_List **attrs, int *cursor_pos);
EAPI void                          ecore_imf_context_focus_in(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_focus_out(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_reset(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_cursor_position_set(Ecore_IMF_Context *ctx, int cursor_pos);
EAPI void                          ecore_imf_context_cursor_location_set(Ecore_IMF_Context *ctx, int x, int y, int w, int h);
EAPI void                          ecore_imf_context_use_preedit_set(Ecore_IMF_Context *ctx, Eina_Bool use_preedit);
EAPI void                          ecore_imf_context_retrieve_surrounding_callback_set(Ecore_IMF_Context *ctx, Eina_Bool (*func)(void *data, Ecore_IMF_Context *ctx, char **text, int *cursor_pos), const void *data);
EAPI void                          ecore_imf_context_input_mode_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode);
EAPI Ecore_IMF_Input_Mode          ecore_imf_context_input_mode_get(Ecore_IMF_Context *ctx);
EAPI Eina_Bool                     ecore_imf_context_filter_event(Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event);

/* plugin specific functions */
EAPI Ecore_IMF_Context            *ecore_imf_context_new(const Ecore_IMF_Context_Class *ctxc);
EAPI void                          ecore_imf_context_data_set(Ecore_IMF_Context *ctx, void *data);
EAPI void                         *ecore_imf_context_data_get(Ecore_IMF_Context *ctx);
EAPI Eina_Bool                     ecore_imf_context_surrounding_get(Ecore_IMF_Context *ctx, char **text, int *cursor_pos);
EAPI void                          ecore_imf_context_preedit_start_event_add(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_preedit_end_event_add(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_preedit_changed_event_add(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_commit_event_add(Ecore_IMF_Context *ctx, const char *str);
EAPI void                          ecore_imf_context_delete_surrounding_event_add(Ecore_IMF_Context *ctx, int offset, int n_chars);
EAPI void                          ecore_imf_context_event_callback_add(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, Ecore_IMF_Event_Cb func, const void *data);
EAPI void                         *ecore_imf_context_event_callback_del(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, Ecore_IMF_Event_Cb func);
EAPI void                          ecore_imf_context_event_callback_call(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, void *event_info);
EAPI void                          ecore_imf_context_prediction_allow_set(Ecore_IMF_Context *ctx, Eina_Bool prediction);
EAPI Eina_Bool                     ecore_imf_context_prediction_allow_get(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_autocapital_type_set(Ecore_IMF_Context *ctx, Ecore_IMF_Autocapital_Type autocapital_type);
EAPI Ecore_IMF_Autocapital_Type    ecore_imf_context_autocapital_type_get(Ecore_IMF_Context *ctx);

EAPI void                          ecore_imf_context_control_panel_show(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_control_panel_hide(Ecore_IMF_Context *ctx);

EAPI void                          ecore_imf_context_input_panel_show(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_input_panel_hide(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_input_panel_layout_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Layout layout);
EAPI Ecore_IMF_Input_Panel_Layout  ecore_imf_context_input_panel_layout_get(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_input_panel_language_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Lang lang);
EAPI Ecore_IMF_Input_Panel_Lang    ecore_imf_context_input_panel_language_get(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_input_panel_enabled_set(Ecore_IMF_Context *ctx, Eina_Bool enable);
EAPI Eina_Bool                     ecore_imf_context_input_panel_enabled_get(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_input_panel_imdata_set(Ecore_IMF_Context *ctx, const void *data, int len);
EAPI void                          ecore_imf_context_input_panel_imdata_get(Ecore_IMF_Context *ctx, void *data, int *len);
EAPI void                          ecore_imf_context_input_panel_return_key_type_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Return_Key_Type return_key_type);
EAPI Ecore_IMF_Input_Panel_Return_Key_Type ecore_imf_context_input_panel_return_key_type_get(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_input_panel_return_key_disabled_set(Ecore_IMF_Context *ctx, Eina_Bool disabled);
EAPI Eina_Bool                     ecore_imf_context_input_panel_return_key_disabled_get(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_input_panel_caps_lock_mode_set(Ecore_IMF_Context *ctx, Eina_Bool mode);
EAPI Eina_Bool                     ecore_imf_context_input_panel_caps_lock_mode_get(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_input_panel_geometry_get(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h);
EAPI Ecore_IMF_Input_Panel_State   ecore_imf_context_input_panel_state_get(Ecore_IMF_Context *ctx);
EAPI void                          ecore_imf_context_input_panel_event_callback_add(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, void (*func) (void *data, Ecore_IMF_Context *ctx, int value), const void *data);
EAPI void                          ecore_imf_context_input_panel_event_callback_del(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, void (*func) (void *data, Ecore_IMF_Context *ctx, int value));
EAPI void                          ecore_imf_context_input_panel_language_locale_get(Ecore_IMF_Context *ctx, char **lang);
EAPI void                          ecore_imf_context_candidate_panel_geometry_get(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h);

/* The following entry points must be exported by each input method module
 */

/*
 * int                imf_module_init   (const Ecore_IMF_Context_Info **info);
 * void               imf_module_exit   (void);
 * Ecore_IMF_Context *imf_module_create (void);
 */

#ifdef __cplusplus
}
#endif

#endif
