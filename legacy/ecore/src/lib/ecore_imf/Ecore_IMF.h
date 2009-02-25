/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _ECORE_IMF_H
#define _ECORE_IMF_H

#include <Ecore_Data.h>

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

   EAPI extern int ECORE_IMF_EVENT_PREEDIT_START;
   EAPI extern int ECORE_IMF_EVENT_PREEDIT_END;
   EAPI extern int ECORE_IMF_EVENT_PREEDIT_CHANGED;
   EAPI extern int ECORE_IMF_EVENT_COMMIT;
   EAPI extern int ECORE_IMF_EVENT_DELETE_SURROUNDING;

   typedef enum
     {
	ECORE_IMF_EVENT_MOUSE_DOWN,
	ECORE_IMF_EVENT_MOUSE_UP,
	ECORE_IMF_EVENT_MOUSE_IN,
	ECORE_IMF_EVENT_MOUSE_OUT,
	ECORE_IMF_EVENT_MOUSE_MOVE,
	ECORE_IMF_EVENT_MOUSE_WHEEL,
	ECORE_IMF_EVENT_KEY_DOWN,
	ECORE_IMF_EVENT_KEY_UP
     } Ecore_IMF_Event_Type;

   typedef enum
     {
	ECORE_IMF_KEYBOARD_MODIFIER_NONE  = 0,      /**< No active modifiers */
	ECORE_IMF_KEYBOARD_MODIFIER_CTRL  = 1 << 0, /**< "Control" is pressed */
	ECORE_IMF_KEYBOARD_MODIFIER_ALT   = 1 << 1, /**< "Alt" is pressed */
	ECORE_IMF_KEYBOARD_MODIFIER_SHIFT = 1 << 2, /**< "Shift" is pressed */
	ECORE_IMF_KEYBOARD_MODIFIER_WIN   = 1 << 3  /**< "Win" (between "Ctrl" and "Alt") is pressed */
     } Ecore_IMF_Keyboard_Modifiers;

   typedef enum
     {
	ECORE_IMF_KEYBOARD_LOCK_NONE      = 0,      /**< No locks are active */
	ECORE_IMF_KEYBOARD_LOCK_NUM       = 1 << 0, /**< "Num" lock is active */
	ECORE_IMF_KEYBOARD_LOCK_CAPS      = 1 << 1, /**< "Caps" lock is active */
	ECORE_IMF_KEYBOARD_LOCK_SCROLL    = 1 << 2  /**< "Scroll" lock is active */
     } Ecore_IMF_Keyboard_Locks;

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
	char *str;
     };

   struct _Ecore_IMF_Event_Delete_Surrounding
     {
	Ecore_IMF_Context *ctx;
	int offset;
	int n_chars;
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
	unsigned int                 timestamp; /**< The timestamp when the event occured */
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
	unsigned int                 timestamp; /**< The timestamp when the event occured */
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
	unsigned int                 timestamp; /**< The timestamp when the event occured */
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
	unsigned int                 timestamp; /**< The timestamp when the event occured */
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
	unsigned int                 timestamp; /**< The timestamp when the event occured */
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
	unsigned int                 timestamp; /**< The timestamp when the event occured */
     };

   struct _Ecore_IMF_Event_Key_Down
     {
	char                        *keyname;   /**< The string name of the key pressed */
	Ecore_IMF_Keyboard_Modifiers modifiers; /**< The keyboard modifiers active when the event has been emitted */
	Ecore_IMF_Keyboard_Locks     locks;     /**< The keyboard locks active when the event has been emitted */
	const char                  *key;       /**< The logical key : (eg shift+1 == exclamation) */
	const char                  *string;    /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
	const char                  *compose;   /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
	unsigned int                 timestamp; /**< The timestamp when the event occured */
   };

   struct _Ecore_IMF_Event_Key_Up
     {
	char                        *keyname;   /**< The string name of the key pressed */
	Ecore_IMF_Keyboard_Modifiers modifiers; /**< The keyboard modifiers active when the event has been emitted */
	Ecore_IMF_Keyboard_Locks     locks;     /**< The keyboard locks active when the event has been emitted */
	const char                  *key;       /**< The logical key : (eg shift+1 == exclamation) */
	const char                  *string;    /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
	const char                  *compose;   /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
	unsigned int                 timestamp; /**< The timestamp when the event occured */
     };

   union _Ecore_IMF_Event
     {
	Ecore_IMF_Event_Mouse_Down mouse_down;
	Ecore_IMF_Event_Mouse_Up mouse_up;
	Ecore_IMF_Event_Mouse_In mouse_in;
	Ecore_IMF_Event_Mouse_Out mouse_out;
	Ecore_IMF_Event_Mouse_Move mouse_move;
	Ecore_IMF_Event_Mouse_Wheel mouse_wheel;
	Ecore_IMF_Event_Key_Down key_down;
	Ecore_IMF_Event_Key_Up key_up;
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
	void (*use_preedit_set)     (Ecore_IMF_Context *ctx, int use_preedit);
	void (*input_mode_set)      (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode);
	int  (*filter_event)        (Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event);
     };

   struct _Ecore_IMF_Context_Info
     {
	const char *id;              /* ID */
	const char *description;     /* Human readable description */
	const char *default_locales; /* Languages for which this context is the default, separated by : */
	const char *canvas_type;     /* The canvas type used by the input method. Eg.: evas */
	int         canvas_required; /* Whether the canvas usage is required for this input method */
     };

   EAPI int                           ecore_imf_init(void);
   EAPI int                           ecore_imf_shutdown(void);

   EAPI Eina_List                    *ecore_imf_context_available_ids_get(void);
   EAPI Eina_List                    *ecore_imf_context_available_ids_by_canvas_type_get(const char *canvas_type);
   EAPI const char                   *ecore_imf_context_default_id_get(void);
   EAPI const char                   *ecore_imf_context_default_id_by_canvas_type_get(const char *canvas_type);
   EAPI const Ecore_IMF_Context_Info *ecore_imf_context_info_by_id_get(const char *id);

   EAPI Ecore_IMF_Context            *ecore_imf_context_add(const char *id);
   EAPI const Ecore_IMF_Context_Info *ecore_imf_context_info_get(Ecore_IMF_Context *ctx);
   EAPI void                          ecore_imf_context_del(Ecore_IMF_Context *ctx);
   EAPI void                          ecore_imf_context_client_window_set(Ecore_IMF_Context *ctx, void *window);
   EAPI void                          ecore_imf_context_client_canvas_set(Ecore_IMF_Context *ctx, void *canvas);
   EAPI void                          ecore_imf_context_show(Ecore_IMF_Context *ctx);
   EAPI void                          ecore_imf_context_hide(Ecore_IMF_Context *ctx);
   EAPI void                          ecore_imf_context_preedit_string_get(Ecore_IMF_Context *ctx, char **str, int *cursor_pos);
   EAPI void                          ecore_imf_context_focus_in(Ecore_IMF_Context *ctx);
   EAPI void                          ecore_imf_context_focus_out(Ecore_IMF_Context *ctx);
   EAPI void                          ecore_imf_context_reset(Ecore_IMF_Context *ctx);
   EAPI void                          ecore_imf_context_cursor_position_set(Ecore_IMF_Context *ctx, int cursor_pos);
   EAPI void                          ecore_imf_context_use_preedit_set(Ecore_IMF_Context *ctx, int use_preedit);
   EAPI void                          ecore_imf_context_retrieve_surrounding_callback_set(Ecore_IMF_Context *ctx, int (*func)(void *data, Ecore_IMF_Context *ctx, char **text, int *cursor_pos), const void *data);
   EAPI void                          ecore_imf_context_input_mode_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode);
   EAPI Ecore_IMF_Input_Mode          ecore_imf_context_input_mode_get(Ecore_IMF_Context *ctx);
   EAPI int                           ecore_imf_context_filter_event(Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event);

   /* plugin specific functions */
   EAPI Ecore_IMF_Context            *ecore_imf_context_new(const Ecore_IMF_Context_Class *ctxc);
   EAPI void                          ecore_imf_context_data_set(Ecore_IMF_Context *ctx, void *data);
   EAPI void                         *ecore_imf_context_data_get(Ecore_IMF_Context *ctx);
   EAPI int                           ecore_imf_context_surrounding_get(Ecore_IMF_Context *ctx, char **text, int *cursor_pos);
   EAPI void                          ecore_imf_context_preedit_start_event_add(Ecore_IMF_Context *ctx);
   EAPI void                          ecore_imf_context_preedit_end_event_add(Ecore_IMF_Context *ctx);
   EAPI void                          ecore_imf_context_preedit_changed_event_add(Ecore_IMF_Context *ctx);
   EAPI void                          ecore_imf_context_commit_event_add(Ecore_IMF_Context *ctx, const char *str);
   EAPI void                          ecore_imf_context_delete_surrounding_event_add(Ecore_IMF_Context *ctx, int offset, int n_chars);

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
