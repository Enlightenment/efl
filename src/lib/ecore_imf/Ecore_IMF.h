#ifndef _ECORE_IMF_H
#define _ECORE_IMF_H

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ECORE_IMF_INPUT_HINT_AUTOFILL_MASK 0xff00

/**
 * @defgroup Ecore_IMF_Lib_Group Ecore_IMF - Ecore Input Method Library Functions
 * @ingroup Ecore
 *
 * Utility functions that set up and shut down the Ecore Input Method
 * library.
 */

/**
 * @defgroup Ecore_IMF_Context_Group Ecore Input Method Context Functions
 * @ingroup Ecore_IMF_Lib_Group
 *
 * @section intro Introduction
 *
 * Functions that operate on Ecore Input Method Context objects.

 * Ecore Input Method Context Function defines the interface for EFL input methods.
 * An input method is used by EFL text input widgets like elm_entry
 * (based on edje_entry) to map from key events to Unicode character strings.
 *
 * The default input method can be set through setting the ECORE_IMF_MODULE environment variable.
 * eg) export ECORE_IMF_MODULE=xim (or scim or ibus)
 *
 * An input method may consume multiple key events in sequence and finally output the composed result.
 * This is called preediting, and an input method may provide feedback about
 * this process by displaying the intermediate composition states as preedit text.
 *
 * Immodule is plugin to connect your application and input method framework such as SCIM, ibus, and so on.@n
 * ecore_imf_init() should be called to initialize and load immodule.@n
 * ecore_imf_shutdown() is used for shutdowning and unloading immodule.
 *
 * @section how-to-compose How to process key event for composition or prediction
 *
 * To input Chinese, Japanese, Korean and other complex languages, the editor widget (as known as entry) should be connected with input method framework.@n
 * Each editor widget should have each input context to connect with input service framework.@n
 * Key event is processed by input method engine. The result is notified to application through ECORE_IMF_CALLBACK_PREEDIT_CHANGED and ECORE_IMF_CALLBACK_COMMIT event.@n
 * @n
 * The following example demonstrates how to connect input method framework and handle preedit and commit string from input method framework.
 * @li @ref ecore_imf_example_c
 *
 * @section media-content How to receive media contents from input method editor
 *
 * Users sometimes wants to send images and other rich content with their input method editor (as known as virtual keyboard or soft keyboard).@n
 * According to this requirement, the way to receive the media content URI such as images and other rich content as well as text have been provided since 1.20.@n
 * @n
 * The following code shows how to receive the media content URI.
 *
 * @code
 * #include <glib.h>
 *
 * static void
 * _imf_event_commit_content_cb(void *data, Ecore_IMF_Context *ctx, void *event_info)
 * {
 *    Ecore_IMF_Event_Commit_Content *commit_content = (Ecore_IMF_Event_Commit_Content *)event;
 *    if (!commit_content) return;
 *
 *    // convert URI to filename
 *    gchar *filepath = g_filename_from_uri(commit_content->content_uri, NULL, NULL);
 *    printf("filepath : %s, description : %s, mime types : %s\n", filepath, commit_content->description, commit_content->mime_types);
 *
 *    // do something to use filepath
 *
 *    if (filepath)
 *       g_free(filepath);
 * }
 *
 * ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_COMMIT_CONTENT, _imf_event_commit_content_cb, data);
 * @endcode
 *
 */

/**
 * @addtogroup Ecore_IMF_Context_Group
 *
 * @{
 */

/**
 * @example ecore_imf_example.c
 * Shows how to write simple editor using the Ecore_IMF library.
 */

/* ecore_imf_context_input_panel_event_callback_add() flag */

/**
 * @typedef Ecore_IMF_Input_Panel_Event
 * Enum containing input panel events.
 */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_STATE_EVENT,              /**< called when the state of the input panel is changed. @since 1.7 */
   ECORE_IMF_INPUT_PANEL_LANGUAGE_EVENT,           /**< called when the language of the input panel is changed. @since 1.7 */
   ECORE_IMF_INPUT_PANEL_SHIFT_MODE_EVENT,         /**< called when the shift key state of the input panel is changed @since 1.7 */
   ECORE_IMF_INPUT_PANEL_GEOMETRY_EVENT,           /**< called when the size of the input panel is changed. @since 1.7 */
   ECORE_IMF_CANDIDATE_PANEL_STATE_EVENT,          /**< called when the state of the candidate word panel is changed. @since 1.7 */
   ECORE_IMF_CANDIDATE_PANEL_GEOMETRY_EVENT,       /**< called when the size of the candidate word panel is changed. @since 1.7 */
   ECORE_IMF_INPUT_PANEL_KEYBOARD_MODE_EVENT       /**< called when the keyboard mode state of the input panel is changed @since 1.20 */
} Ecore_IMF_Input_Panel_Event;

/**
 * @typedef Ecore_IMF_Input_Panel_State
 * Enum containing input panel state notifications.
 */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_STATE_SHOW,        /**< Notification after the display of the input panel @since 1.7 */
   ECORE_IMF_INPUT_PANEL_STATE_HIDE,        /**< Notification prior to the dismissal of the input panel @since 1.7 */
   ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW    /**< Notification prior to the display of the input panel @since 1.7 */
} Ecore_IMF_Input_Panel_State;

/**
 * @typedef Ecore_IMF_Input_Panel_Shift_Mode
 * Enum containing input shift mode states.
 */
typedef enum
{
    ECORE_IMF_INPUT_PANEL_SHIFT_MODE_OFF, /**< @since 1.7 */
    ECORE_IMF_INPUT_PANEL_SHIFT_MODE_ON   /**< @since 1.7 */
} Ecore_IMF_Input_Panel_Shift_Mode;

/**
 * @typedef Ecore_IMF_Candidate_Panel_State
 * Enum containing candidate word panel state notifications.
 */
typedef enum
{
   ECORE_IMF_CANDIDATE_PANEL_SHOW,        /**< Notification after the display of the candidate word panel @since 1.7 */
   ECORE_IMF_CANDIDATE_PANEL_HIDE         /**< Notification prior to the dismissal of the candidate word panel @since 1.7 */
} Ecore_IMF_Candidate_Panel_State;

/**
 * @typedef Ecore_IMF_Input_Panel_Keyboard_Mode
 * Enum containing keyboard mode states.
 */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_HW_KEYBOARD_MODE, /**< @since 1.20 */
   ECORE_IMF_INPUT_PANEL_SW_KEYBOARD_MODE  /**< @since 1.20 */
} Ecore_IMF_Input_Panel_Keyboard_Mode;

/* Events sent by the Input Method */
typedef struct _Ecore_IMF_Event_Preedit_Start      Ecore_IMF_Event_Preedit_Start;
typedef struct _Ecore_IMF_Event_Preedit_End        Ecore_IMF_Event_Preedit_End;
typedef struct _Ecore_IMF_Event_Preedit_Changed    Ecore_IMF_Event_Preedit_Changed;
typedef struct _Ecore_IMF_Event_Commit             Ecore_IMF_Event_Commit;
typedef struct _Ecore_IMF_Event_Delete_Surrounding Ecore_IMF_Event_Delete_Surrounding;
typedef struct _Ecore_IMF_Event_Selection          Ecore_IMF_Event_Selection;
typedef struct _Ecore_IMF_Event_Commit_Content     Ecore_IMF_Event_Commit_Content;

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

/**
 * @typedef Ecore_IMF_Event_Cb
 *
 * @brief Called when a Ecore_IMF event happens.
 *
 * @see  ecore_imf_context_event_callback_add()
 */
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
   ECORE_IMF_CALLBACK_DELETE_SURROUNDING, /**< "DELETE_SURROUNDING" is called when the input method needs to delete all or part of the context surrounding the cursor @since 1.2 */
   ECORE_IMF_CALLBACK_SELECTION_SET,      /**< "SELECTION_SET" is called when the input method needs to set the selection @since 1.9 */
   ECORE_IMF_CALLBACK_PRIVATE_COMMAND_SEND, /**< "PRIVATE_COMMAND_SEND" is called when the input method sends a private command @since 1.12 */
   ECORE_IMF_CALLBACK_COMMIT_CONTENT,     /**< "COMMIT_CONTENT" is called when the input method commits content such as an image @since 1.20 */
   ECORE_IMF_CALLBACK_TRANSACTION_START,  /**< "TRANSACTION_START" is called when a new transaction sequence starts. @since 1.21 */
   ECORE_IMF_CALLBACK_TRANSACTION_END     /**< "TRANSACTION_END" is called when a new transaction sequence starts. @since 1.21 */
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
 * Types for Ecore_IMF keyboard modifiers
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
 * Types for Ecore_IMF keyboard locks
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
 * Types for Ecore_IMF mouse flags
 */
typedef enum
{
   ECORE_IMF_MOUSE_NONE              = 0,      /**< A single click */
   ECORE_IMF_MOUSE_DOUBLE_CLICK      = 1 << 0, /**< A double click */
   ECORE_IMF_MOUSE_TRIPLE_CLICK      = 1 << 1  /**< A triple click */
} Ecore_IMF_Mouse_Flags;

/**
 * @typedef Ecore_IMF_Input_Mode
 * Types for Ecore_IMF input mode
 */
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
   ECORE_IMF_PREEDIT_TYPE_SUB3, /**< Substring style 3 @since 1.1 */
   ECORE_IMF_PREEDIT_TYPE_SUB4, /**< Substring style 4 @since 1.8 */
   ECORE_IMF_PREEDIT_TYPE_SUB5, /**< Substring style 5 @since 1.8 */
   ECORE_IMF_PREEDIT_TYPE_SUB6, /**< Substring style 6 @since 1.8 */
   ECORE_IMF_PREEDIT_TYPE_SUB7  /**< Substring style 7 @since 1.8 */
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
   ECORE_IMF_INPUT_PANEL_LAYOUT_TERMINAL,        /**< Command-line terminal layout including ESC, Alt, Ctrl key, so on (no auto-correct, no auto-capitalization) @since 1.2 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD,        /**< Like normal, but no auto-correct, no auto-capitalization etc. @since 1.2 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_DATETIME,        /**< Date and time layout @since 1.8 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_EMOTICON,        /**< Emoticon layout @since 1.10 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_VOICE            /**< Voice layout, but if the IME does not support voice layout, then normal layout will be shown @since 1.19 */
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
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEND,    /**< Send @since 1.2 */
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SIGNIN   /**< Sign-in @since 1.8 */
} Ecore_IMF_Input_Panel_Return_Key_Type;

/**
 * @typedef Ecore_IMF_Input_Hints
 * @brief Enumeration for defining the types of Ecore_IMF Input Hints.
 * @since 1.12
 */
typedef enum
{
   ECORE_IMF_INPUT_HINT_NONE                = 0,        /**< No active hints @since 1.12 */
   ECORE_IMF_INPUT_HINT_AUTO_COMPLETE       = 1 << 0,   /**< Suggest word auto completion @since 1.12 */
   ECORE_IMF_INPUT_HINT_SENSITIVE_DATA      = 1 << 1,   /**< Typed text should not be stored. @since 1.12 */
   ECORE_IMF_INPUT_HINT_MULTILINE           = 1 << 2,   /**< Multiline text @since 1.18 */
   ECORE_IMF_INPUT_HINT_AUTOFILL_CREDIT_CARD_EXPIRATION_DATE   = 0x100, /**< Autofill hint for a credit card expiration date @since 1.21 */
   ECORE_IMF_INPUT_HINT_AUTOFILL_CREDIT_CARD_EXPIRATION_DAY    = 0x200, /**< Autofill hint for a credit card expiration day @since 1.21 */
   ECORE_IMF_INPUT_HINT_AUTOFILL_CREDIT_CARD_EXPIRATION_MONTH  = 0x300, /**< Autofill hint for a credit card expiration month @since 1.21 */
   ECORE_IMF_INPUT_HINT_AUTOFILL_CREDIT_CARD_EXPIRATION_YEAR   = 0x400, /**< Autofill hint for a credit card expiration year @since 1.21 */
   ECORE_IMF_INPUT_HINT_AUTOFILL_CREDIT_CARD_NUMBER            = 0x500, /**< Autofill hint for a credit card number @since 1.21 */
   ECORE_IMF_INPUT_HINT_AUTOFILL_EMAIL_ADDRESS                 = 0x600, /**< Autofill hint for an email address @since 1.21 */
   ECORE_IMF_INPUT_HINT_AUTOFILL_NAME                          = 0x700, /**< Autofill hint for a user's real name @since 1.21 */
   ECORE_IMF_INPUT_HINT_AUTOFILL_PHONE                         = 0x800, /**< Autofill hint for a phone number @since 1.21 */
   ECORE_IMF_INPUT_HINT_AUTOFILL_POSTAL_ADDRESS                = 0x900, /**< Autofill hint for a postal address @since 1.21 */
   ECORE_IMF_INPUT_HINT_AUTOFILL_POSTAL_CODE                   = 0xA00, /**< Autofill hint for a postal code @since 1.21 */
   ECORE_IMF_INPUT_HINT_AUTOFILL_ID                            = 0xB00  /**< Autofill hint for a user's ID @since 1.21 */
} Ecore_IMF_Input_Hints;

/**
 * @typedef Ecore_IMF_Input_Panel_Layout_Normal_Variation
 * @brief Enumeration for defining the types of Ecore_IMF Input Panel layout for normal variation.
 * @since 1.12
 */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_NORMAL,            /**< The plain normal layout @since 1.12 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_FILENAME,          /**< Filename layout. Symbols such as '/' should be disabled. @since 1.12 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_PERSON_NAME        /**< The name of a person. @since 1.12 */
} Ecore_IMF_Input_Panel_Layout_Normal_Variation;

/**
 * @typedef Ecore_IMF_Input_Panel_Layout_Numberonly_Variation
 * @brief Enumeration for defining the types of Ecore_IMF Input Panel layout for numberonly variation
 * @since 1.8
 */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_NORMAL,            /**< The plain normal number layout @since 1.8 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED,            /**< The number layout to allow a positive or negative sign at the start @since 1.8 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_DECIMAL,           /**< The number layout to allow decimal point to provide fractional value @since 1.8 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED_AND_DECIMAL /**< The number layout to allow decimal point and negative sign @since 1.8 */
} Ecore_IMF_Input_Panel_Layout_Numberonly_Variation;

/**
 * @typedef Ecore_IMF_Input_Panel_Layout_Password_Variation
 * @brief Enumeration for defining the types of Ecore_IMF Input Panel layout for password variation
 * @since 1.12
 */
typedef enum
{
   ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD_VARIATION_NORMAL,          /**< The normal password layout @since 1.12 */
   ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD_VARIATION_NUMBERONLY       /**< The password layout to allow only number @since 1.12 */
} Ecore_IMF_Input_Panel_Layout_Password_Variation;

/**
 * @typedef Ecore_IMF_BiDi_Direction
 * @brief Enumeration for defining the types of Ecore_IMF bidirectionality.
 * @since 1.12
 */
typedef enum
{
   ECORE_IMF_BIDI_DIRECTION_NEUTRAL,    /**< The Neutral mode @since 1.12 */
   ECORE_IMF_BIDI_DIRECTION_LTR,        /**< The Left to Right mode @since 1.12 */
   ECORE_IMF_BIDI_DIRECTION_RTL         /**< The Right to Left mode @since 1.12 */
} Ecore_IMF_BiDi_Direction;

/**
 * @enum _Ecore_IMF_Device_Class
 * @brief Enumeration for defining the types of Ecore_IMF_Device_Class
 * @since 1.14
 */
typedef enum _Ecore_IMF_Device_Class
{
   ECORE_IMF_DEVICE_CLASS_NONE, /**< Not a device @since 1.14 */
   ECORE_IMF_DEVICE_CLASS_SEAT, /**< The user/seat (the user themselves) @since 1.14 */
   ECORE_IMF_DEVICE_CLASS_KEYBOARD, /**< A regular keyboard, numberpad or attached buttons @since 1.14 */
   ECORE_IMF_DEVICE_CLASS_MOUSE, /**< A mouse, trackball or touchpad relative motion device @since 1.14 */
   ECORE_IMF_DEVICE_CLASS_TOUCH, /**< A touchscreen with fingers or stylus @since 1.14 */
   ECORE_IMF_DEVICE_CLASS_PEN, /**< A special pen device @since 1.14 */
   ECORE_IMF_DEVICE_CLASS_POINTER, /**< A laser pointer, wii-style or "minority report" pointing device @since 1.14 */
   ECORE_IMF_DEVICE_CLASS_GAMEPAD /**<  A gamepad controller or joystick @since 1.14 */
} Ecore_IMF_Device_Class; /**< A general class of device @since 1.14 */

/**
 * @enum _Ecore_IMF_Device_Subclass
 * @brief Enumeration for defining the types of Ecore_IMF_Device_Subclass
 * @since 1.14
 */
typedef enum _Ecore_IMF_Device_Subclass
{
   ECORE_IMF_DEVICE_SUBCLASS_NONE, /**< Not a device @since 1.14 */
   ECORE_IMF_DEVICE_SUBCLASS_FINGER, /**< The normal flat of your finger @since 1.14 */
   ECORE_IMF_DEVICE_SUBCLASS_FINGERNAIL, /**< A fingernail @since 1.14 */
   ECORE_IMF_DEVICE_SUBCLASS_KNUCKLE, /**< A Knuckle @since 1.14 */
   ECORE_IMF_DEVICE_SUBCLASS_PALM, /**< The palm of a users hand @since 1.14 */
   ECORE_IMF_DEVICE_SUBCLASS_HAND_SIZE, /**< The side of your hand @since 1.14 */
   ECORE_IMF_DEVICE_SUBCLASS_HAND_FLAT, /**< The flat of your hand @since 1.14 */
   ECORE_IMF_DEVICE_SUBCLASS_PEN_TIP, /**< The tip of a pen @since 1.14 */
   ECORE_IMF_DEVICE_SUBCLASS_TRACKPAD, /**< A trackpad style mouse @since 1.14 */
   ECORE_IMF_DEVICE_SUBCLASS_TRACKPOINT, /**< A trackpoint style mouse @since 1.14 */
   ECORE_IMF_DEVICE_SUBCLASS_TRACKBALL, /**< A trackball style mouse @since 1.14 */
} Ecore_IMF_Device_Subclass; /**< A general subclass of device @since 1.14 */

/**
 * @struct _Ecore_IMF_Event_Preedit_Start
 * @brief The structure type used with the Preedit_Start Input Method event
 */
struct _Ecore_IMF_Event_Preedit_Start
{
   Ecore_IMF_Context *ctx;
};

/**
 * @struct _Ecore_IMF_Event_Preedit_End
 * @brief The structure type used with the Preedit_End Input Method event
 */
struct _Ecore_IMF_Event_Preedit_End
{
   Ecore_IMF_Context *ctx;
};

/**
 * @struct _Ecore_IMF_Event_Preedit_Changed
 * @brief The structure type used with the Preedit_Changed Input Method event
 */
struct _Ecore_IMF_Event_Preedit_Changed
{
   Ecore_IMF_Context *ctx;
};

/**
 * @struct _Ecore_IMF_Event_Commit
 * @brief The structure type used with the Commit Input Method event
 */
struct _Ecore_IMF_Event_Commit
{
   Ecore_IMF_Context *ctx;
   char              *str;
};

/**
 * @struct _Ecore_IMF_Event_Delete_Surrounding
 * @brief The structure type used with the Delete_Surrounding Input Method event
 */
struct _Ecore_IMF_Event_Delete_Surrounding
{
   Ecore_IMF_Context *ctx;
   int                offset;
   int                n_chars;
};

/**
 * @struct _Ecore_IMF_Event_Selection
 * @brief The structure type used with the Selection Input Method event
 */
struct _Ecore_IMF_Event_Selection
{
   Ecore_IMF_Context *ctx;
   int                start;
   int                end;
};

/**
 * @struct _Ecore_IMF_Event_Commit_Content
 * @brief The structure type used with the Commit_Content Input Method event
 * @since 1.20
 */
struct _Ecore_IMF_Event_Commit_Content
{
   Ecore_IMF_Context *ctx;                  /**< The associated Ecore IMF Context */
   const char        *content_uri;          /**< The content URI */
   const char        *description;          /**< The content description */
   const char        *mime_types;           /**< The content MIME types */
};

/**
 * @struct _Ecore_IMF_Event_Mouse_Down
 * @brief The structure type used with the Mouse_Down event
 */
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

/**
 * @struct _Ecore_IMF_Event_Mouse_Up
 * @brief The structure type used with the Mouse_Up event
 */
struct _Ecore_IMF_Event_Mouse_Up
{
   int button;                             /**< The button which has been released */
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

/**
 * @struct _Ecore_IMF_Event_Mouse_In
 * @brief The structure type used with the Mouse_In event
 */
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

/**
 * @struct _Ecore_IMF_Event_Mouse_Out
 * @brief The structure type used with the Mouse_Out event
 */
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

/**
 * @struct _Ecore_IMF_Event_Mouse_Move
 * @brief The structure type used with the Mouse_Move event
 */
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

/**
 * @struct _Ecore_IMF_Event_Mouse_Wheel
 * @brief The structure type used with the Mouse_Wheel event
 */
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

/**
 * @struct _Ecore_IMF_Event_Key_Down
 * @brief The structure type used with the Key_Down event
 */
struct _Ecore_IMF_Event_Key_Down
{
   const char                   *keyname;   /**< The string name of the key pressed */
   Ecore_IMF_Keyboard_Modifiers  modifiers; /**< The keyboard modifiers active when the event has been emitted */
   Ecore_IMF_Keyboard_Locks      locks;     /**< The keyboard locks active when the event has been emitted */
   const char                   *key;       /**< The logical key : (eg shift+1 == exclamation) */
   const char                   *string;    /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char                   *compose;   /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int                  timestamp; /**< The timestamp when the event occurred */
   const char                   *dev_name;  /**< The device name of the key pressed @since 1.14 */
   Ecore_IMF_Device_Class        dev_class; /**< The device class of the key pressed @since 1.14 */
   Ecore_IMF_Device_Subclass     dev_subclass; /**< The device subclass of the key pressed @since 1.14 */
};

/**
 * @struct _Ecore_IMF_Event_Key_Up
 * @brief The structure type used with the Key_Up event
 */
struct _Ecore_IMF_Event_Key_Up
{
   const char                   *keyname;   /**< The string name of the key released */
   Ecore_IMF_Keyboard_Modifiers  modifiers; /**< The keyboard modifiers active when the event has been emitted */
   Ecore_IMF_Keyboard_Locks      locks;     /**< The keyboard locks active when the event has been emitted */
   const char                   *key;       /**< The logical key : (eg shift+1 == exclamation) */
   const char                   *string;    /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char                   *compose;   /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int                  timestamp; /**< The timestamp when the event occurred */
   const char                   *dev_name;  /**< The device name of the key released @since 1.14 */
   Ecore_IMF_Device_Class        dev_class; /**< The device class of the key released @since 1.14 */
   Ecore_IMF_Device_Subclass     dev_subclass; /**< The device subclass of the key released @since 1.14 */
};

/**
 * @brief A union of IMF events.
 */
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

/**
 * @struct _Ecore_IMF_Preedit_Attr
 * @brief Structure that contains preedit attribute information.
 */
struct _Ecore_IMF_Preedit_Attr
{
   Ecore_IMF_Preedit_Type preedit_type; /**< preedit style type */
   unsigned int start_index;            /**< start index of the range (in bytes) */
   unsigned int end_index;              /**< end index of the range (in bytes) */
};

/**
 * @struct _Ecore_IMF_Context_Class
 * @brief Structure used when creating a new Input Method Context. This
 * structure is mainly used by modules implementing the Input Method Context
 * interface.
 *
 */
struct _Ecore_IMF_Context_Class
{
   void (*add)                 (Ecore_IMF_Context *ctx); /**< Create the Input Method Context */
   void (*del)                 (Ecore_IMF_Context *ctx); /**< Delete the Input Method Context */
   void (*client_window_set)   (Ecore_IMF_Context *ctx, void *window); /**< Set the client window for the Input Method Context */
   void (*client_canvas_set)   (Ecore_IMF_Context *ctx, void *canvas); /**< Set the client canvas for the Input Method Context */
   void (*show)                (Ecore_IMF_Context *ctx); /**< Show the Input Method Context */
   void (*hide)                (Ecore_IMF_Context *ctx); /**< Hide the Input Method Context */
   void (*preedit_string_get)  (Ecore_IMF_Context *ctx, char **str, int *cursor_pos); /**< Return current preedit string and cursor position */
   void (*focus_in)            (Ecore_IMF_Context *ctx); /**< Input Method context widget has gained focus */
   void (*focus_out)           (Ecore_IMF_Context *ctx); /**< Input Method context widget has lost focus */
   void (*reset)               (Ecore_IMF_Context *ctx); /**< A change has been made */
   void (*cursor_position_set) (Ecore_IMF_Context *ctx, int cursor_pos); /**< Cursor position changed */
   void (*use_preedit_set)     (Ecore_IMF_Context *ctx, Eina_Bool use_preedit); /**< Use preedit string to display feedback */
   void (*input_mode_set)      (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode); /**< Set the input mode */
   Eina_Bool (*filter_event)   (Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event); /**< Internally handle an event */
   void (*preedit_string_with_attributes_get) (Ecore_IMF_Context *ctx, char **str, Eina_List **attrs, int *cursor_pos); /**<  return current preedit string, attributes, and cursor position */
   void (*prediction_allow_set)(Ecore_IMF_Context *ctx, Eina_Bool prediction); /**< Allow text prediction */
   void (*autocapital_type_set)(Ecore_IMF_Context *ctx, Ecore_IMF_Autocapital_Type autocapital_type); /**< Set auto-capitalization type */
   void (*control_panel_show)   (Ecore_IMF_Context *ctx); /**< Show the control panel */
   void (*control_panel_hide)   (Ecore_IMF_Context *ctx); /**< Hide the control panel */
   void (*input_panel_layout_set) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Layout layout); /**< Set the layout of the input panel */
   Ecore_IMF_Input_Panel_Layout (*input_panel_layout_get) (Ecore_IMF_Context *ctx); /**< Return the current layout of the input panel */
   void (*input_panel_language_set) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Lang lang); /**< Set the language of the input panel */
   Ecore_IMF_Input_Panel_Lang (*input_panel_language_get) (Ecore_IMF_Context *ctx); /**< Get the current language of the input panel */
   void (*cursor_location_set) (Ecore_IMF_Context *ctx, int x, int y, int w, int h); /**< Set the cursor location */
   void (*input_panel_imdata_set)(Ecore_IMF_Context *ctx, const void* data, int len); /**< Set panel-specific data to the input panel */
   void (*input_panel_imdata_get)(Ecore_IMF_Context *ctx, void* data, int *len); /**< Get current panel-specific data from the input panel */
   void (*input_panel_return_key_type_set) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Return_Key_Type return_key_type); /**< Set the return key theme of the input panel based on return key type provided */
   void (*input_panel_return_key_disabled_set) (Ecore_IMF_Context *ctx, Eina_Bool disabled); /**< Disable return key of the input panel */
   void (*input_panel_caps_lock_mode_set) (Ecore_IMF_Context *ctx, Eina_Bool mode); /**< Set input panel caps lock mode */
   void (*input_panel_geometry_get)(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h); /**< Return input panel geometry */
   Ecore_IMF_Input_Panel_State (*input_panel_state_get) (Ecore_IMF_Context *ctx); /**< Return input panel state */
   void (*input_panel_event_callback_add) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, void (*func) (void *data, Ecore_IMF_Context *ctx, int value), void *data); /**< Add a callback on input panel state,language,mode change */
   void (*input_panel_event_callback_del) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, void (*func) (void *data, Ecore_IMF_Context *ctx, int value)); /**< Delete the input panel event callback */
   void (*input_panel_language_locale_get) (Ecore_IMF_Context *ctx, char **lang); /**< Return the current language locale */
   void (*candidate_panel_geometry_get)(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h); /**< Return the candidate panel geometry */
   void (*input_hint_set) (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Hints input_hints); /**< Sets input hint to fine-tune input methods behavior */
   void (*bidi_direction_set) (Ecore_IMF_Context *ctx, Ecore_IMF_BiDi_Direction direction); /**< Set bidirectionality at the cursor position */
   Ecore_IMF_Input_Panel_Keyboard_Mode (*keyboard_mode_get) (Ecore_IMF_Context *ctx); /**< Return the current keyboard mode of the input panel */
   void (*prediction_hint_set) (Ecore_IMF_Context *ctx, const char *prediction_hint); /**< Set the prediction hint to the input panel */
   void (*mime_type_accept_set) (Ecore_IMF_Context *ctx, const char *mime_type); /**< Set the MIME type to the input panel */
   void (*input_panel_position_set) (Ecore_IMF_Context *ctx, int x, int y); /**< Set the position of the input panel */
};

/**
 * @struct _Ecore_IMF_Context_Info
 * @brief A IMF structure containing context information.
 */
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

/**
 * @ingroup Ecore_IMF_Lib_Group
 * @brief Initialises the Ecore_IMF library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 */
EAPI int                           ecore_imf_init(void);

/**
 * @ingroup Ecore_IMF_Lib_Group
 * @brief Shuts down the Ecore_IMF library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 */
EAPI int                           ecore_imf_shutdown(void);

/**
 * @ingroup Ecore_IMF_Lib_Group
 * @brief Registers an Ecore_IMF module.
 *
 * @param info              An Ecore_IMF_Context_Info structure
 * @param imf_module_create A function to call at the creation
 * @param imf_module_exit   A function to call when exiting
 *
 */
EAPI void                          ecore_imf_module_register(const Ecore_IMF_Context_Info *info, Ecore_IMF_Context *(*imf_module_create)(void), Ecore_IMF_Context *(*imf_module_exit)(void));

/**
 * @ingroup Ecore_IMF_Lib_Group
 * @brief Hides the input panel.
 * @return  EINA_TRUE if the input panel will be hidden
            EINA_FALSE if the input panel is already in hidden state
 * @since 1.8.0
 */
EAPI Eina_Bool                     ecore_imf_input_panel_hide(void);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the list of the available Input Method Context ids.
 *
 * Note that the caller is responsible for freeing the Eina_List
 * when finished with it. There is no need to finish the list strings.
 *
 * @return Return an Eina_List of strings;
 *         on failure it returns NULL.
 */
EAPI Eina_List                    *ecore_imf_context_available_ids_get(void);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the list of the available Input Method Context ids by canvas type.
 *
 * Note that the caller is responsible for freeing the Eina_List
 * when finished with it. There is no need to finish the list strings.
 *
 * @param  canvas_type A string containing the canvas type.
 * @return Return an Eina_List of strings;
 *         on failure it returns NULL.
 */
EAPI Eina_List                    *ecore_imf_context_available_ids_by_canvas_type_get(const char *canvas_type);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the id of the default Input Method Context.
 * The id may to used to create a new instance of an Input Method
 * Context object.
 *
 * @return Return a string containing the id of the default Input
 *         Method Context; on failure it returns NULL.
 */
EAPI const char                   *ecore_imf_context_default_id_get(void);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the id of the default Input Method Context corresponding to a canvas
 * type.
 * The id may be used to create a new instance of an Input Method
 * Context object.
 *
 * @param  canvas_type A string containing the canvas type.
 * @return Return a string containing the id of the default Input
 *         Method Context; on failure it returns NULL.
 */
EAPI const char                   *ecore_imf_context_default_id_by_canvas_type_get(const char *canvas_type);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Retrieves the info for the Input Method Context with @p id.
 *
 * @param id The Input Method Context id to query for.
 * @return Return a #Ecore_IMF_Context_Info for the Input Method Context with @p id;
 *         on failure it returns NULL.
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
 * @ingroup Ecore_IMF_Context_Group
 * @brief Creates a new Input Method Context defined by the given id.
 *
 * @param id The Input Method Context id.
 * @return A newly allocated Input Method Context;
 *         on failure it returns NULL.
 */
EAPI Ecore_IMF_Context            *ecore_imf_context_add(const char *id);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Retrieves the info for the given Input Method Context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return Return a #Ecore_IMF_Context_Info for the given Input Method Context;
 *         on failure it returns NULL.
 */
EAPI const Ecore_IMF_Context_Info *ecore_imf_context_info_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Deletes the given Input Method Context and free its memory.
 *
 * @param ctx An #Ecore_IMF_Context.
 */
EAPI void                          ecore_imf_context_del(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the client window for the Input Method Context; this is the
 * Ecore_X_Window when using X11, Ecore_Win32_Window when using Win32, etc.
 * This window is used in order to correctly position status windows, and may
 * also be used for purposes internal to the Input Method Context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param window The client window. This may be @c NULL to indicate
 *               that the previous client window no longer exists.
 */
EAPI void                          ecore_imf_context_client_window_set(Ecore_IMF_Context *ctx, void *window);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the client window of the Input Method Context.
 *
 * See @ref ecore_imf_context_client_window_set for more details.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return Return the client window.
 * @since 1.1.0
 */
EAPI void                         *ecore_imf_context_client_window_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the client canvas for the Input Method Context; this is the
 * canvas in which the input appears.
 * The canvas type can be determined by using the context canvas type.
 * Actually only canvas with type "evas" (Evas *) is supported.
 * This canvas may be used in order to correctly position status windows, and may
 * also be used for purposes internal to the Input Method Context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param canvas The client canvas. This may be @c NULL to indicate
 *               that the previous client canvas no longer exists.
 */
EAPI void                          ecore_imf_context_client_canvas_set(Ecore_IMF_Context *ctx, void *canvas);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the client canvas of the Input Method Context.
 *
 * See @ref ecore_imf_context_client_canvas_set for more details.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return Return the client canvas.
 * @since 1.1.0
 */
EAPI void                         *ecore_imf_context_client_canvas_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Asks the Input Method Context to show itself.
 *
 * @param ctx An #Ecore_IMF_Context.
 */
EAPI void                          ecore_imf_context_show(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Asks the Input Method Context to hide itself.
 *
 * @param ctx An #Ecore_IMF_Context.
 */
EAPI void                          ecore_imf_context_hide(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Retrieves the current preedit string and cursor position
 * for the Input Method Context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param str Location to store the retrieved string. The
 *            string retrieved must be freed with free().
 * @param cursor_pos Location to store position of cursor (in characters)
 *                   within the preedit string.
 */
EAPI void                          ecore_imf_context_preedit_string_get(Ecore_IMF_Context *ctx, char **str, int *cursor_pos);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Retrieves the current preedit string, attributes and
 * cursor position for the Input Method Context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param str Location to store the retrieved string. The
 *            string retrieved must be freed with free().
 * @param attrs An Eina_List of attributes
 * @param cursor_pos Location to store position of cursor (in characters)
 *                   within the preedit string.
 *
 * Example
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
 * @since 1.1.0
 */
EAPI void                          ecore_imf_context_preedit_string_with_attributes_get(Ecore_IMF_Context *ctx, char **str, Eina_List **attrs, int *cursor_pos);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Notifies the Input Method Context that the widget to which its
 * correspond has gained focus.
 *
 * @param ctx An #Ecore_IMF_Context.
 *
 * Example
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
 * @ingroup Ecore_IMF_Context_Group
 * @brief Notifies the Input Method Context that the widget to which its
 * correspond has lost focus.
 *
 * @param ctx An #Ecore_IMF_Context.
 *
 * Example
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
 * @ingroup Ecore_IMF_Context_Group
 * @brief Notifies the Input Method Context that a change such as a
 * change in cursor position has been made. This will typically
 * cause the Input Method Context to clear the preedit state or commit the preedit string.
 *
 * The operation of ecore_imf_context_reset() depends on the specific characteristics of
 * each language. For example, the preedit string is cleared in the Chinese and Japanese Input Method Engine.
 * However, The preedit string is committed and then cleared in the Korean Input Method Engine.
 *
 * This function should be called in case of the focus-out and mouse down event callback function.
 * In addition, it should be called before inserting some text.
 *
 * @param ctx An #Ecore_IMF_Context.
 *
 * Example
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
 * @ingroup Ecore_IMF_Context_Group
 * @brief Notifies the Input Method Context that a change in the cursor
 * position has been made.
 *
 * This function should be called when cursor position is changed or mouse up event is generated.
 * Some input methods that do a heavy job using this event can give a critical performance latency problem.
 * For better typing performance, we suggest that the cursor position change events need to be occurred
 * only if the cursor position is on a confirmed status not on moving status.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param cursor_pos New cursor position in characters.
 */
EAPI void                          ecore_imf_context_cursor_position_set(Ecore_IMF_Context *ctx, int cursor_pos);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Notifies the Input Method Context that a change in the cursor
 * location has been made. The location is relative to the canvas.
 * The cursor location can be used to determine the position of
 * candidate word window in the immodule.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param x cursor x position.
 * @param y cursor y position.
 * @param w cursor width.
 * @param h cursor height.
 * @since 1.1.0
 */
EAPI void                          ecore_imf_context_cursor_location_set(Ecore_IMF_Context *ctx, int x, int y, int w, int h);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets whether the IM context should use the preedit string
 * to display feedback. If @c use_preedit is @c EINA_FALSE (default
 * is @c EINA_TRUE), then the IM context may use some other method to display
 * feedback, such as displaying it in a child of the root window.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param use_preedit Whether the IM context should use the preedit string.
 */
EAPI void                          ecore_imf_context_use_preedit_set(Ecore_IMF_Context *ctx, Eina_Bool use_preedit);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the callback to be used on surrounding_get request.
 *
 * This callback will be called when the Input Method Context
 * module requests the surrounding context.
 * Input methods typically want context in order to constrain input text based on existing text;
 * this is important for languages such as Thai where only some sequences of characters are allowed.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param func The callback to be called.
 * @param data The data pointer to be passed to @p func
 */
EAPI void                          ecore_imf_context_retrieve_surrounding_callback_set(Ecore_IMF_Context *ctx, Eina_Bool (*func)(void *data, Ecore_IMF_Context *ctx, char **text, int *cursor_pos), const void *data);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the callback to be used on selection_get request.
 *
 * This callback will be called when the Input Method Context
 * module requests the selection context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param func The callback to be called.
 * @param data The data pointer to be passed to @p func
 * @since 1.9.0
 */
EAPI void                          ecore_imf_context_retrieve_selection_callback_set(Ecore_IMF_Context *ctx, Eina_Bool (*func)(void *data, Ecore_IMF_Context *ctx, char **text), const void *data);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the input mode used by the Ecore Input Context.
 *
 * The input mode can be one of the input modes defined in
 * Ecore_IMF_Input_Mode. The default input mode is
 * ECORE_IMF_INPUT_MODE_FULL.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param input_mode The input mode to be used by @p ctx.
 */
EAPI void                          ecore_imf_context_input_mode_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the input mode being used by the Ecore Input Context.
 *
 * See @ref ecore_imf_context_input_mode_set for more details.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return The input mode being used by @p ctx.
 */
EAPI Ecore_IMF_Input_Mode          ecore_imf_context_input_mode_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Allows an Ecore Input Context to internally handle an event.
 * If this function returns @c EINA_TRUE, then no further processing
 * should be done for this event.
 *
 * Input methods must be able to accept all types of events (simply
 * returning @c EINA_FALSE if the event was not handled), but there is no
 * obligation of any events to be submitted to this function.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param type The type of event defined by #Ecore_IMF_Event_Type.
 * @param event The event itself.
 * @return @c EINA_TRUE if the event was handled; otherwise @c EINA_FALSE.
 *
 * Example
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

/* plugin specific functions */

/**
 * @defgroup Ecore_IMF_Context_Module_Group Ecore Input Method Context Module Functions
 * @ingroup Ecore_IMF_Lib_Group
 *
 * Functions that should be used by Ecore Input Method Context modules.
 */

/**
 * @ingroup Ecore_IMF_Context_Module_Group
 * @brief Creates a new Input Method Context with klass specified by @p ctxc.
 *
 * This method should be used by modules implementing the Input
 * Method Context interface.
 *
 * @param ctxc An #Ecore_IMF_Context_Class.
 * @return A new #Ecore_IMF_Context; on failure it returns NULL.
 */
EAPI Ecore_IMF_Context            *ecore_imf_context_new(const Ecore_IMF_Context_Class *ctxc);

/**
 * @ingroup Ecore_IMF_Context_Module_Group
 * Sets the Input Method Context specific data.
 *
 * Note that this method should be used by modules to set
 * the Input Method Context specific data and it's not meant to
 * be used by applications to store application specific data.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param data The Input Method Context specific data.
 * @return A new #Ecore_IMF_Context; on failure it returns NULL.
 */
EAPI void                          ecore_imf_context_data_set(Ecore_IMF_Context *ctx, void *data);

/**
 * @ingroup Ecore_IMF_Context_Module_Group
 * @brief Gets the Input Method Context specific data.
 *
 * See @ref ecore_imf_context_data_set for more details.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return The Input Method Context specific data.
 */
EAPI void                         *ecore_imf_context_data_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Module_Group
 * @brief Retrieves context around insertion point.
 * Input methods typically want context in order to constrain input text based on existing text;
 * this is important for languages such as Thai where only some sequences of characters are allowed.
 * In addition, the text around the insertion point can be used for supporting autocapital feature.
 *
 * This function is implemented by calling the
 * Ecore_IMF_Context::retrieve_surrounding_func (
 * set using #ecore_imf_context_retrieve_surrounding_callback_set).
 *
 * There is no obligation for a widget to respond to the
 * retrieve_surrounding_func, so input methods must be prepared
 * to function without context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param text Location to store a UTF-8 encoded string of text
 *             holding context around the insertion point.
 *             If the function returns @c EINA_TRUE, then you must free
 *             the result stored in this location with free().
 * @param cursor_pos Location to store the position in characters of
 *                   the insertion cursor within @p text.
 * @return @c EINA_TRUE if surrounding text was provided; otherwise
 * @c EINA_FALSE.
 */
EAPI Eina_Bool                     ecore_imf_context_surrounding_get(Ecore_IMF_Context *ctx, char **text, int *cursor_pos);

/**
 * @ingroup Ecore_IMF_Context_Module_Group
 * @brief Retrieves the selected text.
 *
 * This function is implemented by calling the
 * Ecore_IMF_Context::retrieve_selection_func (
 * set using #ecore_imf_context_retrieve_selection_callback_set).
 *
 * There is no obligation for a widget to respond to the
 * retrieve_surrounding_func, so input methods must be prepared
 * to function without context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param text Location to store a UTF-8 encoded string of the selected text.
 *             If the function returns @c EINA_TRUE, then you must free
 *             the result stored in this location with free().
 * @return @c EINA_TRUE if selected text was provided; otherwise
 * @c EINA_FALSE.
 * @since 1.9.0
 */
EAPI Eina_Bool                     ecore_imf_context_selection_get(Ecore_IMF_Context *ctx, char **text);

/**
 * @ingroup Ecore_IMF_Context_Module_Group
 * @brief Adds ECORE_IMF_EVENT_PREEDIT_START to the event queue.
 *
 * ECORE_IMF_EVENT_PREEDIT_START should be added when a new preedit sequence starts.
 * It's asynchronous method to put event to the event queue.
 * ecore_imf_context_event_callback_call() can be used as synchronous method.
 *
 * @param ctx An #Ecore_IMF_Context.
 */
EAPI void                          ecore_imf_context_preedit_start_event_add(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Module_Group
 * @brief Adds ECORE_IMF_EVENT_PREEDIT_END to the event queue.
 *
 * ECORE_IMF_EVENT_PREEDIT_END should be added when a new preedit sequence has been completed or canceled.
 * It's asynchronous method to put event to the event queue.
 * ecore_imf_context_event_callback_call() can be used as synchronous method.
 *
 * @param ctx An #Ecore_IMF_Context.
 */
EAPI void                          ecore_imf_context_preedit_end_event_add(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Module_Group
 * @brief Adds ECORE_IMF_EVENT_PREEDIT_CHANGED to the event queue.
 *
 * It's asynchronous method to put event to the event queue.
 * ecore_imf_context_event_callback_call() can be used as synchronous method.
 *
 * @param ctx An #Ecore_IMF_Context.
 */
EAPI void                          ecore_imf_context_preedit_changed_event_add(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Module_Group
 * @brief Adds ECORE_IMF_EVENT_COMMIT to the event queue.
 *
 * It's asynchronous method to put event to the event queue.
 * ecore_imf_context_event_callback_call() can be used as synchronous method.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param str The committed string.
 */
EAPI void                          ecore_imf_context_commit_event_add(Ecore_IMF_Context *ctx, const char *str);

/**
 * @ingroup Ecore_IMF_Context_Module_Group
 * @brief Adds ECORE_IMF_EVENT_DELETE_SURROUNDING to the event queue.
 *
 * Asks the widget that the input context is attached to to delete characters around the cursor position
 * by adding the ECORE_IMF_EVENT_DELETE_SURROUNDING to the event queue.
 * Note that offset and n_chars are in characters not in bytes.
 *
 * It's asynchronous method to put ECORE_IMF_EVENT_DELETE_SURROUNDING event to the event queue.
 * ecore_imf_context_event_callback_call() can be used as synchronous method.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param offset The start offset of surrounding to be deleted.
 * @param n_chars The number of characters to be deleted.
 */
EAPI void                          ecore_imf_context_delete_surrounding_event_add(Ecore_IMF_Context *ctx, int offset, int n_chars);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Adds (registers) a callback function to a given context event.
 *
 * This function adds a function callback to the context @p ctx when the
 * event of type @p type occurs on it. The function pointer is @p
 * func.
 *
 * The event type @p type to trigger the function may be one of
 * #ECORE_IMF_CALLBACK_PREEDIT_START, #ECORE_IMF_CALLBACK_PREEDIT_END,
 * #ECORE_IMF_CALLBACK_PREEDIT_CHANGED, #ECORE_IMF_CALLBACK_COMMIT,
 * #ECORE_IMF_CALLBACK_DELETE_SURROUNDING, #ECORE_IMF_CALLBACK_SELECTION_SET,
 * #ECORE_IMF_CALLBACK_PRIVATE_COMMAND_SEND, #ECORE_IMF_CALLBACK_COMMIT_CONTENT,
 * #ECORE_IMF_CALLBACK_TRANSACTION_START, and #ECORE_IMF_CALLBACK_TRANSACTION_END.
 *
 * @param ctx Ecore_IMF_Context to attach a callback to.
 * @param type The type of event that will trigger the callback
 * @param func The (callback) function to be called when the event is
 *        triggered
 * @param data The data pointer to be passed to @p func
 * @since 1.2.0
 *
 * Example
 * @code
 * #include <glib.h>
 *
 * // example for handling commit event from input framework
 * static void
 * _imf_event_commit_cb(void *data, Ecore_IMF_Context *ctx, void *event_info)
 * {
 *    char *commit_str = event_info;
 *    // something to do
 * }
 *
 * ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_COMMIT, _imf_event_commit_cb, data);
 *
 * // example for receiving media content URI from input framework
 * @code
 * #include <glib.h>
 *
 * static void
 * _imf_event_commit_content_cb(void *data, Ecore_IMF_Context *ctx, void *event_info)
 * {
 *    Ecore_IMF_Event_Commit_Content *commit_content = (Ecore_IMF_Event_Commit_Content *)event;
 *    if (!commit_content) return;
 *
 *    // convert URI to filename
 *    gchar *filepath = g_filename_from_uri(commit_content->content_uri, NULL, NULL);
 *    printf("filepath : %s, description : %s, mime types : %s\n", filepath, commit_content->description, commit_content->mime_types);
 *
 *    // do something to use filepath
 *
 *    if (filepath)
 *       g_free(filepath);
 * }
 *
 * ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_COMMIT_CONTENT, _imf_event_commit_content_cb, data);
 * @endcode
 */
EAPI void                          ecore_imf_context_event_callback_add(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, Ecore_IMF_Event_Cb func, const void *data);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Deletes (unregisters) a callback function registered to a given
 * context event.
 *
 * This function removes a function callback from the context @p ctx when the
 * event of type @p type occurs on it. The function pointer is @p
 * func.
 *
 * @see ecore_imf_context_event_callback_add() for more details
 *
 * @param ctx Ecore_IMF_Context to remove a callback from.
 * @param type The type of event that was triggering the callback
 * @param func The (callback) function that was to be called when the event was triggered
 * @return the data pointer
 * @since 1.2.0
 */
EAPI void                         *ecore_imf_context_event_callback_del(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, Ecore_IMF_Event_Cb func);

/**
 * @ingroup Ecore_IMF_Context_Module_Group
 * @brief Calls a given callback on the context @p ctx.
 *
 * ecore_imf_context_preedit_start_event_add(), ecore_imf_context_preedit_end_event_add(),
 * ecore_imf_context_preedit_changed_event_add(), ecore_imf_context_commit_event_add() and
 * ecore_imf_context_delete_surrounding_event_add() APIs are asynchronous
 * because those API adds each event to the event queue.
 *
 * This API provides the way to call each callback function immediately.
 *
 * @param ctx Ecore_IMF_Context.
 * @param type The type of event that will trigger the callback
 * @param event_info The pointer to event specific struct or information to
 *        pass to the callback functions registered on this event
 * @since 1.2.0
 */
EAPI void                          ecore_imf_context_event_callback_call(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, void *event_info);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets whether the IM context should allow to use the text prediction.
 * If @p prediction is @c EINA_FALSE (default is @c EINA_TRUE), then the IM
 * context will not display the text prediction window.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param prediction Whether the IM context should allow to use the text prediction.
 * @note Default value is EINA_TRUE.
 * @since 1.1.0
 */
EAPI void                          ecore_imf_context_prediction_allow_set(Ecore_IMF_Context *ctx, Eina_Bool prediction);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets whether the IM context should allow to use the text prediction.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return @c EINA_TRUE if it allows to use the text prediction, otherwise
 * @c EINA_FALSE.
 * @since 1.1.0
 */
EAPI Eina_Bool                     ecore_imf_context_prediction_allow_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the autocapitalization type on the immodule.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param autocapital_type the autocapitalization type.
 * @note Default type is ECORE_IMF_AUTOCAPITAL_TYPE_SENTENCE.
 * @since 1.1.0
 */
EAPI void                          ecore_imf_context_autocapital_type_set(Ecore_IMF_Context *ctx, Ecore_IMF_Autocapital_Type autocapital_type);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the autocapitalization type.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return The autocapital type being used by @p ctx.
 * @since 1.1.0
 */
EAPI Ecore_IMF_Autocapital_Type    ecore_imf_context_autocapital_type_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the input hint which allows input methods to fine-tune their behavior.
 *
 * @param ctx An #Ecore_IMF_Context
 * @param hints Input hint
 * @note The default input hint is @c ECORE_IMF_INPUT_HINT_AUTO_COMPLETE.
 * @since 1.12
 */
EAPI void                          ecore_imf_context_input_hint_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Hints hints);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the value of input hint.
 *
 * @param ctx An #Ecore_IMF_Context
 * @return The value of input hint
 * @since 1.12
 */
EAPI Ecore_IMF_Input_Hints         ecore_imf_context_input_hint_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Asks the Input Method Context to show the control panel of using Input Method.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @since 1.1.0
 */
EAPI void                          ecore_imf_context_control_panel_show(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Asks the Input Method Context to hide the control panel of using Input Method.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @since 1.1.0
 */
EAPI void                          ecore_imf_context_control_panel_hide(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Asks the Input Method Context to show the input panel (virtual keyboard).
 *
 * @param ctx An #Ecore_IMF_Context.
 * @since 1.1.0
 */
EAPI void                          ecore_imf_context_input_panel_show(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Asks the Input Method Context to hide the input panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @since 1.1.0
 */
EAPI void                          ecore_imf_context_input_panel_hide(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the layout of the input panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param layout see #Ecore_IMF_Input_Panel_Layout
 * @note Default layout type is ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL.
 * @since 1.1.0
 */
EAPI void                          ecore_imf_context_input_panel_layout_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Layout layout);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the layout of the current active input panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return layout see #Ecore_IMF_Input_Panel_Layout
 * @since 1.1.0
 */
EAPI Ecore_IMF_Input_Panel_Layout  ecore_imf_context_input_panel_layout_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the layout variation of the current active input panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param variation the layout variation
 * @note Default layout variation type is NORMAL.
 * @since 1.8.0
 */
EAPI void                          ecore_imf_context_input_panel_layout_variation_set(Ecore_IMF_Context *ctx, int variation);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the layout variation of the current active input panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return the layout variation
 * @since 1.8.0
 */
EAPI int                           ecore_imf_context_input_panel_layout_variation_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the language of the input panel.
 * This API can be used when you want to show the English keyboard.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param lang the language to be set to the input panel.
 * @since 1.1.0
 */
EAPI void                          ecore_imf_context_input_panel_language_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Lang lang);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the language of the input panel.
 *
 * See @ref ecore_imf_context_input_panel_language_set for more details.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return Ecore_IMF_Input_Panel_Lang
 * @since 1.1.0
 */
EAPI Ecore_IMF_Input_Panel_Lang    ecore_imf_context_input_panel_language_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets whether the Input Method Context should request to show the input panel automatically
 * when the widget has focus.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param enabled If true, the input panel will be shown when the widget is clicked or has focus.
 * @since 1.1.0
 */
EAPI void                          ecore_imf_context_input_panel_enabled_set(Ecore_IMF_Context *ctx, Eina_Bool enabled);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets whether the Input Method Context requests to show the input panel automatically.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return Return the attribute to show the input panel automatically
 * @since 1.1.0
 */
EAPI Eina_Bool                     ecore_imf_context_input_panel_enabled_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the input panel-specific data to deliver to the input panel.
 * This API is used by applications to deliver specific data to the input panel.
 * The data format MUST be negotiated by both application and the input panel.
 * The size and format of data are defined by the input panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param data The specific data to be set to the input panel.
 * @param len the length of data, in bytes, to send to the input panel
 * @since 1.2.0
 */
EAPI void                          ecore_imf_context_input_panel_imdata_set(Ecore_IMF_Context *ctx, const void *data, int len);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the specific data of the current active input panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param data The specific data to be got from the input panel
 * @param len The length of data
 * @since 1.2.0
 */
EAPI void                          ecore_imf_context_input_panel_imdata_get(Ecore_IMF_Context *ctx, void *data, int *len);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the "return" key type. This type is used to set string or icon on the "return" key of the input panel.
 *
 * An input panel displays the string or icon associated with this type.@n
 * Regardless of return key type, return key event will be generated when pressing return key.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param return_key_type The type of "return" key on the input panel
 * @note Default type is ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT.
 * @since 1.2.0
 */
EAPI void                          ecore_imf_context_input_panel_return_key_type_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Return_Key_Type return_key_type);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the "return" key type.
 *
 * @see ecore_imf_context_input_panel_return_key_type_set() for more details
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return The type of "return" key on the input panel
 * @since 1.2.0
 */
EAPI Ecore_IMF_Input_Panel_Return_Key_Type ecore_imf_context_input_panel_return_key_type_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the return key on the input panel to be disabled.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param disabled The state
 * @since 1.2.0
 */
EAPI void                          ecore_imf_context_input_panel_return_key_disabled_set(Ecore_IMF_Context *ctx, Eina_Bool disabled);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets whether the return key on the input panel should be disabled or not.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return @c EINA_TRUE if it should be disabled.
 * @since 1.2.0
 */
EAPI Eina_Bool                     ecore_imf_context_input_panel_return_key_disabled_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the caps lock mode on the input panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param mode Turn on caps lock on the input panel if @c EINA_TRUE.
 * @since 1.2.0
 */
EAPI void                          ecore_imf_context_input_panel_caps_lock_mode_set(Ecore_IMF_Context *ctx, Eina_Bool mode);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the caps lock mode on the input panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return @c EINA_TRUE if the caps lock is turned on.
 * @since 1.2.0
 */
EAPI Eina_Bool                     ecore_imf_context_input_panel_caps_lock_mode_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the position of the current active input panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param x top-left x co-ordinate of the input panel
 * @param y top-left y co-ordinate of the input panel
 * @param w width of the input panel
 * @param h height of the input panel
 * @since 1.3
 */
EAPI void                          ecore_imf_context_input_panel_geometry_get(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets state of current active input panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return The state of input panel.
 * @since 1.3
 */
EAPI Ecore_IMF_Input_Panel_State   ecore_imf_context_input_panel_state_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Registers a callback function which will be called if there is change in input panel state,language,mode etc.
 * In order to deregister the callback function
 * Use @ref ecore_imf_context_input_panel_event_callback_del.
 *
 * @param ctx An #Ecore_IMF_Context
 * @param type event type
 * @param func the callback function
 * @param data application-input panel specific data.
 * @since 1.3
 */
EAPI void                          ecore_imf_context_input_panel_event_callback_add(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, void (*func) (void *data, Ecore_IMF_Context *ctx, int value), const void *data);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Unregisters a callback function which will be called if there is change in input panel state, language, mode etc.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param type An #Ecore_IMF_Input_Panel_Event.
 * @param func the callback function
 * @since 1.3
 */
EAPI void                          ecore_imf_context_input_panel_event_callback_del(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, void (*func) (void *data, Ecore_IMF_Context *ctx, int value));

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Calls a given input panel callback on the context @p ctx.
 *
 * @param ctx Ecore_IMF_Context.
 * @param type The type of event that will trigger the callback
 * @param value the event value
 * @since 1.8.0
 */
EAPI void                          ecore_imf_context_input_panel_event_callback_call(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, int value);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Deletes all input panel callback on the context @p ctx.
 *
 * Deletes all input panel callback to be registered by ecore_imf_context_input_panel_event_callback_add()
 *
 * @param ctx Ecore_IMF_Context.
 * @since 1.8.0
 */
EAPI void                          ecore_imf_context_input_panel_event_callback_clear(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the current language locale of the input panel.
 *
 * ex) fr_FR
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param lang Location to store the retrieved language string. The
 *             string retrieved must be freed with free().
 * @since 1.3
 */
EAPI void                          ecore_imf_context_input_panel_language_locale_get(Ecore_IMF_Context *ctx, char **lang);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the geometry information of the candidate panel.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param x top-left x co-ordinate of the candidate panel
 * @param y top-left y co-ordinate of the candidate panel
 * @param w width of the candidate panel
 * @param h height of the candidate panel
 * @since 1.3
 */
EAPI void                          ecore_imf_context_candidate_panel_geometry_get(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets whether the Input Method Context should request to show the input panel in case of only an user's explicit Mouse Up event.
 * It doesn't request to show the input panel even though the Input Method Context has focus.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param ondemand If true, the input panel will be shown in case of only Mouse up event. (Focus event will be ignored.)
 * @since 1.8.0
 */
EAPI void                          ecore_imf_context_input_panel_show_on_demand_set(Ecore_IMF_Context *ctx, Eina_Bool ondemand);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets whether the Input Method Context should request to show the input panel in case of only an user's explicit Mouse Up event.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return @c EINA_TRUE if the input panel will be shown in case of only Mouse up event.
 * @since 1.8.0
 */
EAPI Eina_Bool                     ecore_imf_context_input_panel_show_on_demand_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the bidirectionality at the current cursor position.
 *
 * @since 1.12.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] direction The direction mode
 */
EAPI void                          ecore_imf_context_bidi_direction_set(Ecore_IMF_Context *ctx, Ecore_IMF_BiDi_Direction direction);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the bidirectionality at the current cursor position.
 *
 * @since 1.12.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The direction mode
 */
EAPI Ecore_IMF_BiDi_Direction      ecore_imf_context_bidi_direction_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Get the keyboard mode on the input panel.
 *
 * @since 1.20.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return the keyboard mode
 */
EAPI Ecore_IMF_Input_Panel_Keyboard_Mode ecore_imf_context_keyboard_mode_get(Ecore_IMF_Context *ctx);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Set the prediction hint string to deliver to the input panel.
 *
 * This API can be used when you want to set prediction hint to use intelligent reply suggestion service.
 * The intelligent reply suggestion service generates reply candidates for given prediction hint.
 * Example
 * prediction hint: How are you? -> result: I'm fine, Not bad, I'm all right.
 *
 * @since 1.20.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] prediction_hint The prediction hint string.
 */
EAPI void                          ecore_imf_context_prediction_hint_set(Ecore_IMF_Context *ctx, const char *prediction_hint);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the allowed MIME type to deliver to the input panel.
 *
 * @since 1.20.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param[in] mime_type The allowed MIME type in entry
 *
 * Example
 * @code
 * const char *mime_type = "text/plain,image/png,application/pdf";
 * ecore_imf_context_mime_type_accept_set(imf_context, mime_type);
 * @endcode
 */
EAPI void                         ecore_imf_context_mime_type_accept_set(Ecore_IMF_Context *ctx, const char *mime_type);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the x,y coordinates of the input panel.
 * @remarks This API can be used in floating mode.
 *
 * @since 1.21.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param x top-left x coordinate of the input panel
 * @param y top-left y coordinate of the input panel
 */
EAPI void                         ecore_imf_context_input_panel_position_set(Ecore_IMF_Context *ctx, int x, int y);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Sets the prediction hint data at the specified key
 *
 * @since 1.21.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param key The key of the prediction hint
 * @param data The data to replace
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * This function modifies the data of @p key with @p data in the hash associated @p
 * ctx. If no entry is found, @p data is added to the hash associated @p ctx with the
 * key @p key. On success this function returns EINA_TRUE,
 * otherwise it returns @c EINA_FALSE.
 */
EAPI Eina_Bool                    ecore_imf_context_prediction_hint_hash_set(Ecore_IMF_Context *ctx, const char *key, const char *value);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Removes the prediction hint data identified by a key
 *
 * @since 1.21.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @param key The key of the prediction hint
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * This function removes the entry identified by @p key from the hash associated @p ctx.
 */
EAPI Eina_Bool                    ecore_imf_context_prediction_hint_hash_del(Ecore_IMF_Context *ctx, const char *key);

/**
 * @ingroup Ecore_IMF_Context_Group
 * @brief Gets the hash table of prediction hint data
 *
 * @since 1.21.0
 *
 * @param[in] ctx An #Ecore_IMF_Context
 * @return The prediction hint hash table
 */
EAPI const Eina_Hash             *ecore_imf_context_prediction_hint_hash_get(Ecore_IMF_Context *ctx);

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

#undef EAPI
#define EAPI

#endif
