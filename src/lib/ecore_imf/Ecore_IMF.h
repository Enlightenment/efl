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
 * @internal
 * @defgroup Ecore_IMF_Group Ecore_IMF - Ecore Input Method Library Functions
 * @ingroup Ecore_Group
 *
 * Utility functions that set up and shut down the Ecore Input Method
 * library.
 *
 * @{
 */

/**
 * @internal
 * @defgroup Ecore_IMF_Context_Group Ecore Input Method Context Functions
 * @ingroup Ecore_IMF_Group
 *
 * @brief This group discusses the functions that operate on Ecore Input Method Context objects.

 * @remarks Ecore Input Method Context Function defines the interface for EFL input methods.
 *          An input method is used by EFL text input widgets like elm_entry
 *          (based on edje_entry) to map from key events to Unicode character strings.
 *
 * @remarks The default input method can be set through setting the ECORE_IMF_MODULE environment variable.
 *          eg: export ECORE_IMF_MODULE=xim (or scim or ibus)
 *
 * @remarks An input method may consume multiple key events in sequence and finally give the composed result as output.
 *          This is called preediting, and an input method may provide feedback about
 *          this process by displaying the intermediate composition states as preedit text.
 *
 * @remarks Immodule is a plugin to connect your application and input method framework such as SCIM, ibus, and so on. \n
 *          ecore_imf_init() should be called to initialize and load immodule. \n
 *          ecore_imf_shutdown() is used to shutdown and unload immodule.
 *
 * @{
 */

/**
 * @brief Enumeration of Ecore IMF Input Panel State type
 * @see ecore_imf_context_input_panel_event_callback_add
 */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_STATE_EVENT,              /**< Called when the state of the input panel is changed @since 1.7 */
   ECORE_IMF_INPUT_PANEL_LANGUAGE_EVENT,           /**< Called when the language of the input panel is changed @since 1.7 */
   ECORE_IMF_INPUT_PANEL_SHIFT_MODE_EVENT,         /**< Called when the shift key state of the input panel is changed @since 1.7 */
   ECORE_IMF_INPUT_PANEL_GEOMETRY_EVENT,           /**< Called when the size of the input panel is changed @since 1.7 */
   ECORE_IMF_CANDIDATE_PANEL_STATE_EVENT,          /**< Called when the state of the candidate word panel is changed @since 1.7 */
   ECORE_IMF_CANDIDATE_PANEL_GEOMETRY_EVENT        /**< Called when the size of the candidate word panel is changed @since 1.7 */
} Ecore_IMF_Input_Panel_Event;

/**
 * @brief Enumeration of Ecore IMF Input Panel State type
 */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_STATE_SHOW,        /**< Notification after the display of the input panel @since 1.7 */
   ECORE_IMF_INPUT_PANEL_STATE_HIDE,        /**< Notification prior to the dismissal of the input panel @since 1.7 */
   ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW    /**< Notification prior to the display of the input panel @since 1.7 */
} Ecore_IMF_Input_Panel_State;

/**
 * @brief Enumeration of Ecore IMF Input Panel Shift Mode type
 */
typedef enum
{
    ECORE_IMF_INPUT_PANEL_SHIFT_MODE_OFF, /**< @since 1.7 */
    ECORE_IMF_INPUT_PANEL_SHIFT_MODE_ON   /**< @since 1.7 */
} Ecore_IMF_Input_Panel_Shift_Mode;

/**
 * @brief Enumeration of Ecore IMF Candidate Panel type
 */
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
typedef struct _Ecore_IMF_Event_Selection          Ecore_IMF_Event_Selection;

/* Events to the filter */
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
typedef struct _Ecore_IMF_Context_Info             Ecore_IMF_Context_Info;             /**< Input Method Context info */

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
 * @brief Enumeration that defines the Ecore IMF Event callback types.
 *
 * @see ecore_imf_context_event_callback_add()
 */
typedef enum
{
   ECORE_IMF_CALLBACK_PREEDIT_START,      /**< "PREEDIT_START" is called when a new preediting sequence starts @since 1.2 */
   ECORE_IMF_CALLBACK_PREEDIT_END,        /**< "PREEDIT_END" is called when a preediting sequence has been completed or cancelled @since 1.2 */
   ECORE_IMF_CALLBACK_PREEDIT_CHANGED,    /**< "PREEDIT_CHANGED" is called whenever the preedit sequence currently being entered has changed @since 1.2 */
   ECORE_IMF_CALLBACK_COMMIT,             /**< "COMMIT" is called when a complete input sequence has been entered by the user @since 1.2 */
   ECORE_IMF_CALLBACK_DELETE_SURROUNDING, /**< "DELETE_SURROUNDING" is called when the input method needs to delete all or part of the context surrounding the cursor @since 1.2 */
   ECORE_IMF_CALLBACK_SELECTION_SET,      /**< "SELECTION_SET" is called when the input method needs to set the selection @since 1.9 */
   ECORE_IMF_CALLBACK_PRIVATE_COMMAND_SEND /**< "PRIVATE_COMMAND_SEND" is called when the input method sends a private command @since 1.12 */
} Ecore_IMF_Callback_Type;

/**
 * @typedef Ecore_IMF_Event_Type
 *
 * @brief Enumeration that defines the Ecore IMF event types.
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
 * @brief Enumeration that defines the types of Ecore_IMF keyboard modifiers.
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
 * @brief Enumeration that defines the types of Ecore_IMF keyboard locks.
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
 * @brief Enumeration that defines the types of Ecore_IMF mouse flags.
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
 * @brief Enumeration that defines the Ecore IMF Preedit style types.
 *
 * @see ecore_imf_context_preedit_string_with_attributes_get()
 */
typedef enum
{
   ECORE_IMF_PREEDIT_TYPE_NONE, /**< None style @since 1.1 */
   ECORE_IMF_PREEDIT_TYPE_SUB1, /**< Substring style 1 @since 1.1 */
   ECORE_IMF_PREEDIT_TYPE_SUB2, /**< Substring style 2 @since 1.1 */
   ECORE_IMF_PREEDIT_TYPE_SUB3, /**< Substring style 3 @since 1.1 */
   ECORE_IMF_PREEDIT_TYPE_SUB4, /**< Substring style 4 @since 1.8 */
   ECORE_IMF_PREEDIT_TYPE_SUB5, /**< Substring style 5 @since 1.8 */
   ECORE_IMF_PREEDIT_TYPE_SUB6, /**< Substring style 6 @since 1.8 */
   ECORE_IMF_PREEDIT_TYPE_SUB7  /**< Substring style 7 @since 1.8 */
} Ecore_IMF_Preedit_Type;

/**
 * @typedef Ecore_IMF_Autocapital_Type
 *
 * @brief Enumeration that defines the auto-capitalization types.
 *
 * @see ecore_imf_context_autocapital_type_set()
 */
typedef enum
{
   ECORE_IMF_AUTOCAPITAL_TYPE_NONE,         /**< No auto-capitalization when typing @since 1.1 */
   ECORE_IMF_AUTOCAPITAL_TYPE_WORD,         /**< Auto-capitalize each type word @since 1.1 */
   ECORE_IMF_AUTOCAPITAL_TYPE_SENTENCE,     /**< Auto-capitalize the start of each sentence @since 1.1 */
   ECORE_IMF_AUTOCAPITAL_TYPE_ALLCHARACTER, /**< Auto-capitalize all letters @since 1.1 */
} Ecore_IMF_Autocapital_Type;

/**
 * @typedef Ecore_IMF_Input_Panel_Layout
 *
 * @brief Enumeration that defines the input panel (virtual keyboard) layout types.
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
   ECORE_IMF_INPUT_PANEL_LAYOUT_TERMINAL,        /**< Command-line terminal layout including the ESC, Alt, Ctrl key, and so on (no auto-correct, no auto-capitalization) @since 1.2 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD,        /**< Like normal, but no auto-correct, no auto-capitalization, and so on @since 1.2 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_DATETIME,        /**< Date and time layout @since 1.8 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_EMOTICON         /**< Emoticon layout @since 1.10 */
} Ecore_IMF_Input_Panel_Layout;

/**
 * @typedef Ecore_IMF_Input_Panel_Lang
 *
 * @brief Enumeration that defines the input panel (virtual keyboard) language modes.
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
 * @brief Enumeration that defines the "Return" key types on the input panel (virtual keyboard).
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
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEND,    /**< Send @since 1.2 */
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SIGNIN   /**< Sign-in @since 1.8 */
} Ecore_IMF_Input_Panel_Return_Key_Type;

/**
 * @typedef Ecore_IMF_Input_Hints
 * @brief Enumeration that defines the types of Ecore_IMF Input Hints.
 * @since 1.12
 */
typedef enum
{
   ECORE_IMF_INPUT_HINT_NONE                = 0,        /**< No active hints @since 1.12 */
   ECORE_IMF_INPUT_HINT_AUTO_COMPLETE       = 1 << 0,   /**< Suggest word auto completion @since 1.12 */
   ECORE_IMF_INPUT_HINT_SENSITIVE_DATA      = 1 << 1,   /**< Typed text should not be stored. @since 1.12 */
} Ecore_IMF_Input_Hints;

enum
{
   ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_NORMAL,            /**< The plain normal layout @since 1.12 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_FILENAME,          /**< Filename layout. Symbols such as '/' should be disabled. @since 1.12 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_PERSON_NAME        /**< The name of a person. @since 1.12 */
};

enum
{
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_NORMAL,            /**< The plain normal number layout @since 1.8 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED,            /**< The number layout to allow a positive or negative sign at the start @since 1.8 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_DECIMAL,           /**< The number layout to allow decimal point to provide fractional value @since 1.8 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED_AND_DECIMAL /**< The number layout to allow decimal point and negative sign @since 1.8 */
};

enum
{
   ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD_VARIATION_NORMAL,          /**< The normal password layout @since 1.12 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD_VARIATION_NUMBERONLY       /**< The password layout to allow only number @since 1.12 */
};

/**
 * @typedef Ecore_IMF_BiDi_Direction
 * @brief Enumeration that defines the types of Ecore_IMF bidirectionality
 * @since 1.12
 */
typedef enum
{
   ECORE_IMF_BIDI_DIRECTION_NEUTRAL,    /**< The neutral mode @since 1.12 */
   ECORE_IMF_BIDI_DIRECTION_LTR,        /**< The Left to Right mode @since 1.12 */
   ECORE_IMF_BIDI_DIRECTION_RTL         /**< The Right to Left mode @since 1.12 */
} Ecore_IMF_BiDi_Direction;

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

struct _Ecore_IMF_Event_Selection
{
   Ecore_IMF_Context *ctx;
   int                start;
   int                end;
};

struct _Ecore_IMF_Event_Mouse_Down
{
   int button;                             /**< The button that has been pressed */
   struct {
      int x, y;
   } output;
   struct {
      int x, y;
   } canvas;
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The active keyboard modifiers when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The active keyboard locks when the event has been emitted */
   Ecore_IMF_Mouse_Flags        flags;     /**< The flags corresponding to a mouse click (single, double, or triple click) */
   unsigned int                 timestamp; /**< The timestamp when the event occurred */
};

struct _Ecore_IMF_Event_Mouse_Up
{
   int button;                             /**< The button that has been pressed */
   struct {
      int x, y;
   } output;
   struct {
      int x, y;
   } canvas;
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The active keyboard modifiers when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The active keyboard locks when the event has been emitted */
   Ecore_IMF_Mouse_Flags        flags;     /**< The flags corresponding to a mouse click (single, double, or triple click) */
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
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The active keyboard modifiers when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The active keyboard locks when the event has been emitted */
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
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The active keyboard modifiers when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The active keyboard locks when the event has been emitted */
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
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The active keyboard modifiers when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The active keyboard locks when the event has been emitted */
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
   Ecore_IMF_Keyboard_Modifiers modifiers; /**< The active keyboard modifiers when the event has been emitted */
   Ecore_IMF_Keyboard_Locks     locks;     /**< The active keyboard locks when the event has been emitted */
   unsigned int                 timestamp; /**< The timestamp when the event occurred */
};

struct _Ecore_IMF_Event_Key_Down
{
   const char                   *keyname;   /**< The string name of the key that has been pressed */
   Ecore_IMF_Keyboard_Modifiers  modifiers; /**< The active keyboard modifiers when the event has been emitted */
   Ecore_IMF_Keyboard_Locks      locks;     /**< The active keyboard locks when the event has been emitted */
   const char                   *key;       /**< The logical key : (eg shift+1 == exclamation) */
   const char                   *string;    /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char                   *compose;   /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int                  timestamp; /**< The timestamp when the event occurred */
};

struct _Ecore_IMF_Event_Key_Up
{
   const char                   *keyname;   /**< The string name of the key that has been pressed */
   Ecore_IMF_Keyboard_Modifiers  modifiers; /**< The active keyboard modifiers when the event has been emitted */
   Ecore_IMF_Keyboard_Locks      locks;     /**< The active keyboard locks when the event has been emitted */
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
   Ecore_IMF_Preedit_Type preedit_type; /**< The preedit style type */
   unsigned int start_index;            /**< The start index of the range (in bytes) */
   unsigned int end_index;              /**< The end index of the range (in bytes) */
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
   void (*input_hint_set) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Hints input_hints);
   void (*bidi_direction_set) (Ecore_IMF_Context *ctx, Ecore_IMF_BiDi_Direction direction);
};

struct _Ecore_IMF_Context_Info
{
   const char *id;              /**< ID */
   const char *description;     /**< Human readable description */
   const char *default_locales; /**< Languages for which this context is the default, separated by : */
   const char *canvas_type;     /**< The canvas type used by the input method. Eg.: evas */
   int         canvas_required; /**< Whether canvas usage is required for this input method */
};

/**
 * @brief Initialises the Ecore_IMF library.
 *
 * @return  The number of times the library has been initialised without being
 *          shut down
 * @ingroup Ecore_IMF_Group
 */
EAPI int                           ecore_imf_init(void);

/**
 * @brief Shuts down the Ecore_IMF library.
 *
 * @return  The number of times the library has been initialised without being
 *          shut down
 * @ingroup Ecore_IMF_Group
 */
EAPI int                           ecore_imf_shutdown(void);

EAPI void                          ecore_imf_module_register(const Ecore_IMF_Context_Info *info, Ecore_IMF_Context *(*imf_module_create)(void), Ecore_IMF_Context *(*imf_module_exit)(void));

/**
 * @brief Hides the input panel.
 *
 * @since 1.8.0
 *
 * @return  @c EINA_TRUE if the input panel is hidden,
 *          otherwise @c EINA_FALSE if the input panel is already in the hidden state
 * @ingroup Ecore_IMF_Group
 */
EAPI Eina_Bool                     ecore_imf_input_panel_hide(void);

/**
 * @brief Gets the list of available Input Method Context IDs.
 *
 * @remarks Note that the caller is responsible for freeing the Eina_List
 *          when he is finished with it. There is no need to finish the list strings.
 *
 * @return An Eina_List of strings,
 *         otherwise @c NULL on failure
 */
EAPI Eina_List                    *ecore_imf_context_available_ids_get(void);

EAPI Eina_List                    *ecore_imf_context_available_ids_by_canvas_type_get(const char *canvas_type);

/**
 * @brief Gets the ID of the default Input Method Context.
 *
 * @remarks The ID may be used to create a new instance of an Input Method
 *          Context object.
 *
 * @return A string containing the ID of the default Input Method Context,
 *         otherwise @c NULL on failure
 */
EAPI const char                   *ecore_imf_context_default_id_get(void);

EAPI const char                   *ecore_imf_context_default_id_by_canvas_type_get(const char *canvas_type);

/**
 * Retrieve the info for the Input Method Context with @p id.
 *
 * @param[in] id The Input Method Context id to query for.
 * @return Return a #Ecore_IMF_Context_Info for the Input Method Context with @p id;
 *         on failure it returns NULL.
 * @ingroup Ecore_IMF_Context_Group
 *
 * Example
 * @code
 *
 * const char *ctx_id;
 * const Ecore_IMF_Context_Info *ctx_info;
 * Ecore_IMF_Context *imf_context;
 * ctx_id = ecore_imf_context_default_id_get();
 * if (ctx_id)
 *   {
 *      ctx_info = ecore_imf_context_info_by_id_get(ctx_id);
 *      if (!ctx_info->canvas_type ||
 *          strcmp(ctx_info->canvas_type, "evas") == 0)
 *        {
 *           imf_context = ecore_imf_context_add(ctx_id);
 *        }
 *      else
 *        {
 *           ctx_id = ecore_imf_context_default_id_by_canvas_type_get("evas");
 *           if (ctx_id)
 *             {
 *                imf_context = ecore_imf_context_add(ctx_id);
 *             }
 *        }
 *   }
 * @endcode
 */
EAPI const Ecore_IMF_Context_Info *ecore_imf_context_info_by_id_get(const char *id);

/**
 * @brief Gets the info for the Input Method Context with ID @a id.
 *
 * @param[in] id The Input Method Context ID to query for
 * @return An #Ecore_IMF_Context_Info for the Input Method Context with ID @a id,
 *         otherwise @c NULL on failure
 */
EAPI Ecore_IMF_Context            *ecore_imf_context_add(const char *id);

/**
 * @brief Gets the info for the given Input Method Context.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return An #Ecore_IMF_Context_Info for the given Input Method Context,
 *         otherwise @c NULL on failure
 */
EAPI const Ecore_IMF_Context_Info *ecore_imf_context_info_get(Ecore_IMF_Context *ctx);

/**
 * @brief Deletes the given Input Method Context and frees its memory.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 */
EAPI void                          ecore_imf_context_del(Ecore_IMF_Context *ctx);

/**
 * @brief Sets the client window for the Input Method Context. This is the
 *        Ecore_X_Window when using X11, Ecore_Win32_Window when using Win32, and so on.
 *
 * @remarks This window is used in order to correctly position status windows and may
 *          also be used for purposes that are internal to the Input Method Context.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] window The client windowc\n
 *               This may be @c NULL to indicate
 *               that the previous client window no longer exists.
 */
EAPI void                          ecore_imf_context_client_window_set(Ecore_IMF_Context *ctx, void *window);

/**
 * @brief Gets the client window of the Input Method Context.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The client window
 *
 * @see ecore_imf_context_client_window_set()
 */
EAPI void                         *ecore_imf_context_client_window_get(Ecore_IMF_Context *ctx);

/**
 * @brief Sets the client canvas for the Input Method Context. This is the
 *        canvas in which the input appears.
 *
 * @remarks The canvas type can be determined by using the context canvas type.
 *          Actually only a canvas with type "evas" (Evas *) is supported.
 * 
 * @remarks This canvas may be used in order to correctly position status windows, and may
 *          also be used for purposes that are internal to the Input Method Context.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] canvas The client canvas \n
 *               This may be @c NULL to indicate
 *               that the previous client canvas no longer exists.
 */
EAPI void                          ecore_imf_context_client_canvas_set(Ecore_IMF_Context *ctx, void *canvas);

/**
 * @brief Gets the client canvas of the Input Method Context.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The client canvas
 *
 * @see ecore_imf_context_client_canvas_set()
 */
EAPI void                         *ecore_imf_context_client_canvas_get(Ecore_IMF_Context *ctx);

/**
 * @brief Asks the Input Method Context to show itself.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 */
EAPI void                          ecore_imf_context_show(Ecore_IMF_Context *ctx);

/**
 * @brief Asks the Input Method Context to hide itself.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 */
EAPI void                          ecore_imf_context_hide(Ecore_IMF_Context *ctx);

/**
 * @brief Gets the current preedit string and cursor position
 *        for the Input Method Context.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[out] str The location to store the retrieved string \n
 *            The string retrieved must be freed with free().
 * @param[out] cursor_pos The location to store the position of the cursor (in characters)
 *                   within the preedit string
 */
EAPI void                          ecore_imf_context_preedit_string_get(Ecore_IMF_Context *ctx, char **str, int *cursor_pos);

/**
 * @brief Gets the current preedit string, attributes, and
 *        cursor position for the Input Method Context.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[out] str The location to store the retrieved string \n
 *            The string retrieved must be freed with free().
 * @param[in] attrs An Eina_List of attributes
 * @param[out] cursor_pos The location to store the position of the cursor (in characters)
 *                   within the preedit string
 *
 * Example:
 * @code
 * char *preedit_string;
 * int cursor_pos;
 * Eina_List *attrs = NULL, *l = NULL;
 * Ecore_IMF_Preedit_Attr *attr;
 *
 * ecore_imf_context_preedit_string_with_attributes_get(imf_context,
 *                                                      &preedit_string,
 *                                                      &attrs, &cursor_pos);
 * if (!preedit_string) return;
 *
 *  if (strlen(preedit_string) > 0)
 *    {
 *       if (attrs)
 *         {
 *            EINA_LIST_FOREACH(attrs, l, attr)
 *              {
 *                 if (attr->preedit_type == ECORE_IMF_PREEDIT_TYPE_SUB1)
 *                   {
 *                      // Something to do
 *                   }
 *                 else if (attr->preedit_type == ECORE_IMF_PREEDIT_TYPE_SUB2)
 *                   {
 *                      // Something to do
 *                   }
 *                 else if (attr->preedit_type == ECORE_IMF_PREEDIT_TYPE_SUB3)
 *                   {
 *                      // Something to do
 *                   }
 *              }
 *         }
 *    }
 *
 * // delete attribute list
 * EINA_LIST_FREE(attrs, attr) free(attr);
 *
 * free(preedit_string);
 * @endcode
 */
EAPI void                          ecore_imf_context_preedit_string_with_attributes_get(Ecore_IMF_Context *ctx, char **str, Eina_List **attrs, int *cursor_pos);

/**
 * @brief Notifies the Input Method Context that the widget to which it
 *        corresponds has gained focus.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 *
 * Example:
 * @code
 * static void
 * _focus_in_cb(void *data, Evas_Object *o, const char *emission, const char *source)
 * {
 *    Ecore_IMF_Context *imf_context = data;
 *    ecore_imf_context_focus_in(imf_context);
 * }
 *
 * evas_object_event_callback_add(obj, EVAS_CALLBACK_FOCUS_IN, _focus_in_cb, imf_context);
 * @endcode
 */
EAPI void                          ecore_imf_context_focus_in(Ecore_IMF_Context *ctx);

/**
 * @brief Notifies the Input Method Context that the widget to which it
 *        corresponds has lost focus.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 *
 * Example:
 * @code
 * static void
 * _focus_out_cb(void *data, Evas_Object *o, const char *emission, const char *source)
 * {
 *    Ecore_IMF_Context *imf_context = data;
 *    ecore_imf_context_reset(imf_context);
 *    ecore_imf_context_focus_out(imf_context);
 * }
 *
 * evas_object_event_callback_add(obj, EVAS_CALLBACK_FOCUS_OUT, _focus_out_cb, ed);
 * @endcode
 */
EAPI void                          ecore_imf_context_focus_out(Ecore_IMF_Context *ctx);

/**
 * @brief Notifies the Input Method Context that a change, such as a
 *        change in the cursor position, has been made. This typically
 *        causes the Input Method Context to clear the preedit state or commit the preedit string.
 *
 * @remarks The operation of ecore_imf_context_reset() depends on the specific characteristics of
 *          each language. For example, the preedit string is cleared in the Chinese and Japanese Input Method Engine.
 *          However, The preedit string is committed and then cleared in the Korean Input Method Engine.
 *
 * @remarks This function should be called for the focus-out and mouse down event callback function.
 *          In addition, it should be called before inserting some text.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 *
 * Example:
 * @code
 * static void
 * _focus_out_cb(void *data, Evas_Object *o, const char *emission, const char *source)
 * {
 *    Ecore_IMF_Context *imf_context = data;
 *    ecore_imf_context_reset(imf_context);
 *    ecore_imf_context_focus_out(imf_context);
 * }
 *
 * evas_object_event_callback_add(obj, EVAS_CALLBACK_FOCUS_OUT, _focus_out_cb, imf_context);
 * @endcode
 */
EAPI void                          ecore_imf_context_reset(Ecore_IMF_Context *ctx);

/**
 * @brief Notifies the Input Method Context that a change in the cursor
 *        position has been made.
 *
 * @remarks This function should be called when the cursor position is changed or a mouse up event is generated.
 *          Some input methods that do a heavy job using this event can give a critical performance latency problem.
 *          For better typing performance, we suggest that the cursor position change events need to occur
 *          only if the cursor position is on a confirmed status, not on a moving status.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] cursor_pos The new cursor position in characters
 */
EAPI void                          ecore_imf_context_cursor_position_set(Ecore_IMF_Context *ctx, int cursor_pos);

/**
 * @brief Notifies the Input Method Context that a change in the cursor
 *        location has been made. The location is relative to the canvas.
 *
 * @since 1.1.0
 *
 * @remarks The cursor location can be used to determine the position of
 *          the candidate word window in the immodule.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] x The cursor's x position
 * @param[in] y The cursor's y position
 * @param[in] w The cursor width
 * @param[in] h The cursor height
 */
EAPI void                          ecore_imf_context_cursor_location_set(Ecore_IMF_Context *ctx, int x, int y, int w, int h);

/**
 * @brief Sets whether the IM context should use the preedit string
 *        to display feedback. 
 *
 * @remarks If @a use_preedit is @c EINA_FALSE (default
 *          is @c EINA_TRUE), then the IM context may use some other method to display
 *          feedback, such as displaying it in a child of the root window.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] use_preedit The boolean value that indicates whether the IM context should use the preedit string
 */
EAPI void                          ecore_imf_context_use_preedit_set(Ecore_IMF_Context *ctx, Eina_Bool use_preedit);

/**
 * @brief Sets the callback to be used on the surrounding_get request.
 *
 * @remarks This callback is called when the Input Method Context
 *          module requests the surrounding context.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] func The callback to be called
 * @param[in] data The data pointer to be passed to @a func
 */
EAPI void                          ecore_imf_context_retrieve_surrounding_callback_set(Ecore_IMF_Context *ctx, Eina_Bool (*func)(void *data, Ecore_IMF_Context *ctx, char **text, int *cursor_pos), const void *data);

/**
 * @brief Sets the callback to be used on the selection_get request.
 *
 * @since 1.9.0
 *
 * @remarks This callback is called when the Input Method Context
 *          module requests the selection context.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] func The callback to be called
 * @param[in] data The data pointer to be passed to @a func
 */
EAPI void                          ecore_imf_context_retrieve_selection_callback_set(Ecore_IMF_Context *ctx, Eina_Bool (*func)(void *data, Ecore_IMF_Context *ctx, char **text), const void *data);

/**
 * @brief Sets the callback to be used on the surrounding_get request.
 *
 * @remarks The input mode can be one of the input modes defined in
 *          Ecore_IMF_Input_Mode. The default input mode is
 *          @c ECORE_IMF_INPUT_MODE_FULL.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] input_mode The input mode to be used by @a ctx
 */
EAPI void                          ecore_imf_context_input_mode_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode);

/**
 * @brief Gets the input mode being used by the Ecore Input Context.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The input mode being used by @a ctx
 *
 * @see ecore_imf_context_input_mode_set
 */
EAPI Ecore_IMF_Input_Mode          ecore_imf_context_input_mode_get(Ecore_IMF_Context *ctx);

/**
 * @brief Allows an Ecore Input Context to internally handle an event.
 *
 * @remarks If this function returns @c EINA_TRUE, then no further processing
 *          should be done for this event.
 *
 * @remarks Input methods must be able to accept all types of events (simply
 *          returning @c EINA_FALSE if the event is not handled), but there is no
 *          obligation of any events to be submitted to this function.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] type The type of event defined by #Ecore_IMF_Event_Type
 * @param[in] event The event itself
 * @return @c EINA_TRUE if the event is handled,
 *         otherwise @c EINA_FALSE
 *
 * Example:
 * @code
 * static void
 * _key_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
 * {
 *    Evas_Event_Key_Down *ev = event_info;
 *    if (!ev->key) return;
 *
 *    if (imf_context)
 *      {
 *         Ecore_IMF_Event_Key_Down ecore_ev;
 *         ecore_imf_evas_event_key_down_wrap(ev, &ecore_ev);
 *         if (ecore_imf_context_filter_event(imf_context,
 *                                            ECORE_IMF_EVENT_KEY_DOWN,
 *                                            (Ecore_IMF_Event *)&ecore_ev))
 *           return;
 *      }
 * }
 *
 * evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_DOWN, _key_down_cb, data);
 * @endcode
 */
EAPI Eina_Bool                     ecore_imf_context_filter_event(Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event);

/* Plugin specific functions */

/**
 * @brief Creates a new Input Method Context with the class specified by @a ctxc.
 *
 * @remarks This method should be used by modules implementing the Input
 *          Method Context interface.
 *
 * @param[in] ctxc An #Ecore_IMF_Context_Class
 * @return A new #Ecore_IMF_Context,
 *         otherwise @c NULL on failure
 */
EAPI Ecore_IMF_Context            *ecore_imf_context_new(const Ecore_IMF_Context_Class *ctxc);

/**
 * @brief Sets the Input Method Context specific data.
 *
 * @remarks Note that this method should be used by modules to set
 *          the Input Method Context specific data and it's not meant to
 *          be used by applications to store application specific data.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] data The Input Method Context specific data
 * @return A new #Ecore_IMF_Context,
 *         otherwise @c NULL on failure
 */
EAPI void                          ecore_imf_context_data_set(Ecore_IMF_Context *ctx, void *data);

/**
 * @brief Gets the Input Method Context specific data.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The Input Method Context specific data
 *
 * @see ecore_imf_context_data_set()
 */
EAPI void                         *ecore_imf_context_data_get(Ecore_IMF_Context *ctx);

/**
 * @brief Gets the context around the insertion point.
 *
 * @remarks Input methods typically want context in order to constrain the input text based on existing text.
 *          This is important for languages such as Thai where only some sequences of characters are allowed.
 *          In addition, the text around the insertion point can be used for supporting the auto-capital feature.
 *
 * @remarks This function is implemented by calling the
 *          Ecore_IMF_Context::retrieve_surrounding_func
 *          (set using #ecore_imf_context_retrieve_surrounding_callback_set).
 *
 * @remarks There is no obligation for a widget to respond to
 *          retrieve_surrounding_func, so input methods must be prepared
 *          to function without context.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[out] text The location to store a UTF-8 encoded string of text
 *             holding context around the insertion point \n
 *             If the function returns @c EINA_TRUE, then you must free
 *             the result stored in this location using free().
 * @param[out] cursor_pos The location to store the position in characters of
 *                   the insertion cursor within @a text
 * @return @c EINA_TRUE if the surrounding text is provided,
 * 		   otherwise @c EINA_FALSE
 */
EAPI Eina_Bool                     ecore_imf_context_surrounding_get(Ecore_IMF_Context *ctx, char **text, int *cursor_pos);

/**
 * @brief Gets the selected text.
 *
 * @since 1.9.0
 *
 * @remarks This function is implemented by calling
 *          Ecore_IMF_Context::retrieve_selection_func
 *          (set using #ecore_imf_context_retrieve_selection_callback_set).
 *
 * @remarks There is no obligation for a widget to respond to
 *          retrieve_surrounding_func, so input methods must be prepared
 *          to function without context.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[out] text The location to store a UTF-8 encoded string of the selected text \n
 *             If the function returns @c EINA_TRUE, then you must free
 *             the result stored in this location using free().
 * @return @c EINA_TRUE if the selected text is provided,
 *         otherwise @c EINA_FALSE
 */
EAPI Eina_Bool                     ecore_imf_context_selection_get(Ecore_IMF_Context *ctx, char **text);

/**
 * @brief Adds @c ECORE_IMF_EVENT_PREEDIT_START to the event queue.
 *
 * @remarks @c ECORE_IMF_EVENT_PREEDIT_START should be added when a new preedit sequence starts.
 *          It's an asynchronous method to put an event to the event queue.
 *          ecore_imf_context_event_callback_call() can be used as a synchronous method.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 */
EAPI void                          ecore_imf_context_preedit_start_event_add(Ecore_IMF_Context *ctx);

/**
 * @brief Adds @c ECORE_IMF_EVENT_PREEDIT_END to the event queue.
 *
 * @remarks @c ECORE_IMF_EVENT_PREEDIT_END should be added when a new preedit sequence has been completed or cancelled.
 *          It's an asynchronous method to put an event to the event queue.
 *          ecore_imf_context_event_callback_call() can be used as a synchronous method.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 */
EAPI void                          ecore_imf_context_preedit_end_event_add(Ecore_IMF_Context *ctx);

/**
 * @brief Adds @c ECORE_IMF_EVENT_PREEDIT_CHANGED to the event queue.
 *
 * @remarks It's an asynchronous method to put an event to the event queue.
 *          ecore_imf_context_event_callback_call() can be used as a synchronous method.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 */
EAPI void                          ecore_imf_context_preedit_changed_event_add(Ecore_IMF_Context *ctx);

/**
 * @brief Adds @c ECORE_IMF_EVENT_COMMIT to the event queue.
 *
 * @remarks It's an asynchronous method to put an event to the event queue.
 *          ecore_imf_context_event_callback_call() can be used as a synchronous method.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] str The committed string
 */
EAPI void                          ecore_imf_context_commit_event_add(Ecore_IMF_Context *ctx, const char *str);

/**
 * @brief Adds @c ECORE_IMF_EVENT_DELETE_SURROUNDING to the event queue.
 *
 * @remarks This asks the widget that the input context is attached to delete characters around the cursor position
 *          by adding @c ECORE_IMF_EVENT_DELETE_SURROUNDING to the event queue.
 *          Note that @a offset and @a n_chars are in characters and not in bytes.
 *
 * @remarks It's an asynchronous method to put the @c ECORE_IMF_EVENT_DELETE_SURROUNDING event to the event queue.
 *          ecore_imf_context_event_callback_call() can be used as a synchronous method.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] offset The start offset of the surrounding to be deleted
 * @param[in] n_chars The number of characters to be deleted
 */
EAPI void                          ecore_imf_context_delete_surrounding_event_add(Ecore_IMF_Context *ctx, int offset, int n_chars);

/**
 * @}
 */

/**
 * @brief Adds (registers) a callback function to a given context event.
 *
 * @details This function adds a function callback to the context @a ctx when the
 *          event of type @a type occurs on it. The function pointer is
 *          @a func.
 *
 * @since 1.2.0
 *
 * @remarks The event type @a type to trigger the function may be one of
 *          #ECORE_IMF_CALLBACK_PREEDIT_START, #ECORE_IMF_CALLBACK_PREEDIT_END,
 *          #ECORE_IMF_CALLBACK_PREEDIT_CHANGED, #ECORE_IMF_CALLBACK_COMMIT, or
 *          #ECORE_IMF_CALLBACK_DELETE_SURROUNDING.
 *
 * @param[in] ctx The Ecore_IMF_Context to attach a callback to
 * @param[in] type The type of event that triggers the callback
 * @param[in] func The (callback) function to be called when the event is
 *             triggered
 * @param[in] data The data pointer to be passed to @a func
 *
 * Example:
 * @code
 * static void
 * _imf_event_commit_cb(void *data, Ecore_IMF_Context *ctx, void *event_info)
 * {
 *    char *commit_str = event_info;
 *    // something to do
 * }
 *
 * ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_COMMIT, _imf_event_commit_cb, data);
 * @endcode
 */
EAPI void                          ecore_imf_context_event_callback_add(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, Ecore_IMF_Event_Cb func, const void *data);

/**
 * @brief Deletes (unregisters) a callback function registered to a given
 *        context event.
 *
 * @details This function removes a function callback from the context @a ctx when the
 *          event of type @a type occurs on it. The function pointer is
 *          @a func.
 *
 * @since 1.2.0
 *
 * @param[in] ctx The Ecore_IMF_Context to remove a callback from
 * @param[in] type The type of event that triggers the callback
 * @param[in] func The (callback) function to be called when the event is triggered
 * @return The data pointer
 *
 * @see ecore_imf_context_event_callback_add()
 */
EAPI void                         *ecore_imf_context_event_callback_del(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, Ecore_IMF_Event_Cb func);

/**
 * @brief Calls a given callback on the context @a ctx.
 *
 * @since 1.2.0
 *
 * @remarks The ecore_imf_context_preedit_start_event_add(), ecore_imf_context_preedit_end_event_add(),
 *          ecore_imf_context_preedit_changed_event_add(), ecore_imf_context_commit_event_add(), and
 *          ecore_imf_context_delete_surrounding_event_add() APIs are asynchronous
 *          because those APIs add each event to the event queue.
 *
 * @remarks This API provides a way to call each callback function immediately.
 *
 * @param[in] ctx An Ecore_IMF_Context
 * @param[in] type The type of event that triggers the callback
 * @param[in] event_info A pointer to an event specific struct or information to
 *                   pass to the callback functions registered on this event
 */
EAPI void                          ecore_imf_context_event_callback_call(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, void *event_info);

/**
 * @brief Sets whether the IM context should allow text prediction.
 *
 * @since 1.1.0
 *
 * @remarks If @a prediction is @c EINA_FALSE (default is @c EINA_TRUE), then the IM
 *          context does not display the text prediction window.
 *          Default value is @c EINA_TRUE.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] prediction The boolean value that indicates whether the IM context should allow text prediction
 */
EAPI void                          ecore_imf_context_prediction_allow_set(Ecore_IMF_Context *ctx, Eina_Bool prediction);

/**
 * @brief Gets whether the IM context should allow text prediction.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return @c EINA_TRUE if it allows text prediction,
 *         otherwise @c EINA_FALSE
 */
EAPI Eina_Bool                     ecore_imf_context_prediction_allow_get(Ecore_IMF_Context *ctx);

/**
 * @brief Sets the auto-capitalization type on the immodule.
 *
 * @since 1.1.0
 *
 * @remarks Default type is @c ECORE_IMF_AUTOCAPITAL_TYPE_SENTENCE.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] autocapital_type The auto-capitalization type
 */
EAPI void                          ecore_imf_context_autocapital_type_set(Ecore_IMF_Context *ctx, Ecore_IMF_Autocapital_Type autocapital_type);

/**
 * @brief Gets the auto-capitalization type.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The autocapital type being used by @a ctx
 */
EAPI Ecore_IMF_Autocapital_Type    ecore_imf_context_autocapital_type_get(Ecore_IMF_Context *ctx);

/**
 * @brief Sets the input hint which allows input methods to fine-tune their behavior.
 *
 * @since 1.12
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] hints input hint
 * @note The default input hint is ECORE_IMF_INPUT_HINT_AUTO_COMPLETE.
 */
EAPI void                          ecore_imf_context_input_hint_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Hints hints);

/**
 * @brief Gets the value of input hint.
 *
 * @since 1.12
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The value of input hint
 */
EAPI Ecore_IMF_Input_Hints         ecore_imf_context_input_hint_get(Ecore_IMF_Context *ctx);

/**
 * @brief Asks the Input Method Context to show the control panel for using the Input Method.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 */
EAPI void                          ecore_imf_context_control_panel_show(Ecore_IMF_Context *ctx);

/**
 * @brief Asks the Input Method Context to hide the control panel for using the Input Method.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 */
EAPI void                          ecore_imf_context_control_panel_hide(Ecore_IMF_Context *ctx);

/**
 * @brief Asks the Input Method Context to show the input panel (virtual keyboard).
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 */
EAPI void                          ecore_imf_context_input_panel_show(Ecore_IMF_Context *ctx);

/**
 * @brief Asks the Input Method Context to hide the input panel.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 */
EAPI void                          ecore_imf_context_input_panel_hide(Ecore_IMF_Context *ctx);

/**
 * @brief Sets the layout of the input panel.
 *
 * @since 1.1.0
 *
 * @remarks Default layout type is @c ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] layout The #Ecore_IMF_Input_Panel_Layout
 */
EAPI void                          ecore_imf_context_input_panel_layout_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Layout layout);

/**
 * @brief Gets the layout of the current active input panel.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return layout The #Ecore_IMF_Input_Panel_Layout
 */
EAPI Ecore_IMF_Input_Panel_Layout  ecore_imf_context_input_panel_layout_get(Ecore_IMF_Context *ctx);

/**
 * @brief Sets the layout variation of the current active input panel.
 *
 * @since 1.8.0
 *
 * @remarks Default layout variation type is @c NORMAL.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] variation The layout variation
 */
EAPI void                          ecore_imf_context_input_panel_layout_variation_set(Ecore_IMF_Context *ctx, int variation);

/**
 * @brief Gets the layout variation of the current active input panel.
 *
 * @since 1.8.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The layout variation
 */
EAPI int                           ecore_imf_context_input_panel_layout_variation_get(Ecore_IMF_Context *ctx);

/**
 * @brief Sets the language of the input panel.
 *
 * @since 1.1.0
 *
 * @remarks This API can be used when you want to show the English keyboard.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] lang The language to be set for the input panel
 */
EAPI void                          ecore_imf_context_input_panel_language_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Lang lang);

/**
 * @brief Gets the language of the input panel.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The Ecore_IMF_Input_Panel_Lang
 *
 * @see ecore_imf_context_input_panel_language_set()
 */
EAPI Ecore_IMF_Input_Panel_Lang    ecore_imf_context_input_panel_language_get(Ecore_IMF_Context *ctx);

/**
 * @brief Sets whether the Input Method Context should request to show the input panel automatically
 * 		  when the widget has focus.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] enabled If @c true the input panel is shown when the widget is clicked or has focus,
 *                otherwise @c false
 */
EAPI void                          ecore_imf_context_input_panel_enabled_set(Ecore_IMF_Context *ctx, Eina_Bool enabled);

/**
 * @brief Gets whether the Input Method Context requests to show the input panel automatically.
 *
 * @since 1.1.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The attribute to show the input panel automatically
 */
EAPI Eina_Bool                     ecore_imf_context_input_panel_enabled_get(Ecore_IMF_Context *ctx);

/**
 * @brief Sets the input panel-specific data to deliver to the input panel.
 * 
 * @since 1.2.0
 *
 * @remarks This API is used by applications to deliver specific data to the input panel.
 *          The data format MUST be negotiated by both the application and the input panel.
 *          The size and format of data is defined by the input panel.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] data The specific data to be set to the input panel
 * @param[in] len The length of data, in bytes, to send to the input panel
 */
EAPI void                          ecore_imf_context_input_panel_imdata_set(Ecore_IMF_Context *ctx, const void *data, int len);

/**
 * @brief Gets the specific data of the current active input panel.
 *
 * @since 1.2.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[out] data The specific data to be obtained from the input panel
 * @param[out] len The length of the data
 */
EAPI void                          ecore_imf_context_input_panel_imdata_get(Ecore_IMF_Context *ctx, void *data, int *len);

/**
 * @brief Sets the "return" key type. This type is used to set a string or icon on the "return" key of the input panel.
 *
 * @since 1.2.0
 *
 * @remarks An input panel displays the string or icon associated to this type.
 *
 * @remarks Default type is @c ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] return_key_type The type of "return" key on the input panel
 */
EAPI void                          ecore_imf_context_input_panel_return_key_type_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Return_Key_Type return_key_type);

/**
 * @brief Gets the "return" key type.
 *
 * @since 1.2.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The type of "return" key on the input panel
 *
 * @see ecore_imf_context_input_panel_return_key_type_set()
 */
EAPI Ecore_IMF_Input_Panel_Return_Key_Type ecore_imf_context_input_panel_return_key_type_get(Ecore_IMF_Context *ctx);

/**
 * @brief Sets the return key on the input panel to be disabled.
 *
 * @since 1.2.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] disabled The return key state
 */
EAPI void                          ecore_imf_context_input_panel_return_key_disabled_set(Ecore_IMF_Context *ctx, Eina_Bool disabled);

/**
 * @brief Gets whether the return key on the input panel should be disabled.
 *
 * @since 1.2.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return @c EINA_TRUE if it should be disabled,
 *         otherwise @c EINA_FALSE
 */
EAPI Eina_Bool                     ecore_imf_context_input_panel_return_key_disabled_get(Ecore_IMF_Context *ctx);

/**
 * @brief Sets the caps lock mode on the input panel.
 *
 * @since 1.2.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] mode If @c EINA_TRUE caps lock on the input panel is turned on,
 *             otherwise @c EINA_FALSE
 */
EAPI void                          ecore_imf_context_input_panel_caps_lock_mode_set(Ecore_IMF_Context *ctx, Eina_Bool mode);

/**
 * @brief Gets the caps lock mode on the input panel.
 *
 * @since 1.2.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return @c EINA_TRUE if caps lock is turned on,
 *         otherwise @c EINA_FALSE
 */
EAPI Eina_Bool                     ecore_imf_context_input_panel_caps_lock_mode_get(Ecore_IMF_Context *ctx);

/**
 * @brief Gets the position of the current active input panel.
 *
 * @since 1.30
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[out] x The top-left x co-ordinate of the input panel
 * @param[out] y The top-left y co-ordinate of the input panel
 * @param[out] w The width of the input panel
 * @param[out] h The height of the input panel
 */
EAPI void                          ecore_imf_context_input_panel_geometry_get(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h);

/**
 * @brief Gets the state of the current active input panel.
 *
 * @since 1.3
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The state of the input panel
 */
EAPI Ecore_IMF_Input_Panel_State   ecore_imf_context_input_panel_state_get(Ecore_IMF_Context *ctx);

/**
 * @brief Registers a callback function which is called if there is a change in the input panel state, language, mode, and so on.
 * 
 * @since 1.3
 *
 * @remarks In order to unregister the callback function
 *          use @ref ecore_imf_context_input_panel_event_callback_del.
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] type The event type
 * @param[in] func The callback function
 * @param[in] data The application-input panel specific data
 */
EAPI void                          ecore_imf_context_input_panel_event_callback_add(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, void (*func) (void *data, Ecore_IMF_Context *ctx, int value), const void *data);

/**
 * @brief Unregisters a callback function which is called if there is a change in the input panel state, language, mode, and so on.
 *
 * @since 1.3
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] type An #Ecore_IMF_Input_Panel_Event
 * @param[in] func The callback function
 */
EAPI void                          ecore_imf_context_input_panel_event_callback_del(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, void (*func) (void *data, Ecore_IMF_Context *ctx, int value));

/**
 * @brief Calls a given input panel callback on the context @a ctx.
 *
 * @since 1.8.0
 *
 * @param[in] ctx An Ecore_IMF_Context
 * @param[in] type The type of event that triggers the callback
 * @param[in] value The event value
 */
EAPI void                          ecore_imf_context_input_panel_event_callback_call(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, int value);

/**
 * @brief Deletes all input panel callbacks on the context @a ctx.
 *
 * @details This deletes all input panel callbacks to be registered by ecore_imf_context_input_panel_event_callback_add().
 *
 * @since 1.8.0
 *
 * @param[in] ctx An Ecore_IMF_Context
 */
EAPI void                          ecore_imf_context_input_panel_event_callback_clear(Ecore_IMF_Context *ctx);

/**
 * @brief Gets the current language locale of the input panel, eg: fr_FR.
 *
 * @since 1.3
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[out] lang The location to store the retrieved language string \n
 *             The string retrieved must be freed with free().
 */
EAPI void                          ecore_imf_context_input_panel_language_locale_get(Ecore_IMF_Context *ctx, char **lang);

/**
 * @brief Gets the geometry information of the candidate panel.
 *
 * @since 1.3
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[out] x The top-left x co-ordinate of the candidate panel
 * @param[out] y The top-left y co-ordinate of the candidate panel
 * @param[out] w The width of the candidate panel
 * @param[out] h The height of the candidate panel
 */
EAPI void                          ecore_imf_context_candidate_panel_geometry_get(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h);

/**
 * @brief Sets whether the Input Method Context should request to show the input panel if only one user's explicit Mouse Up event has occurred.
 *		  It doesn't request to show the input panel even though the Input Method Context has focus.
 *
 * @since 1.8.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] ondemand If @c true the input panel is shown when only one user's Mouse up event occurs, (Focus event is ignored)
 *                 otherwise @c false
 */
EAPI void                          ecore_imf_context_input_panel_show_on_demand_set(Ecore_IMF_Context *ctx, Eina_Bool ondemand);

/**
 * @brief Gets whether the Input Method Context should request to show the input panel if only one user's explicit Mouse Up event has occurred.
 *
 * @since 1.8.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return @c EINA_TRUE if the input panel is shown when only one user's Mouse up event occurs,
 *         otherwise @c EINA_FALSE
 */
EAPI Eina_Bool                     ecore_imf_context_input_panel_show_on_demand_get(Ecore_IMF_Context *ctx);

/**
 * @brief Sets the bidirectionality at the current cursor position.
 *
 * @since 1.12.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] direction the direction mode
 */
EAPI void                          ecore_imf_context_bidi_direction_set(Ecore_IMF_Context *ctx, Ecore_IMF_BiDi_Direction direction);

/**
 * @brief Gets the bidirectionality at the current cursor position.
 *
 * @since 1.12.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return the direction mode
 */
EAPI Ecore_IMF_BiDi_Direction      ecore_imf_context_bidi_direction_get(Ecore_IMF_Context *ctx);

/* The following entry points must be exported by each input method module.
 */

/*
 * int                imf_module_init   (const Ecore_IMF_Context_Info **info);
 * void               imf_module_exit   (void);
 * Ecore_IMF_Context *imf_module_create (void);
 */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
