/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef _ECORE_IMF_H
#define _ECORE_IMF_H

#ifdef EAPI
#undef EAPI
#endif
#ifdef _MSC_VER
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
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

#include <Ecore_Data.h>
#include <Evas.h>

#ifdef __cplusplus
extern "C" {
#endif

   typedef struct _Ecore_IMF_Event_Preedit_Start      Ecore_IMF_Event_Preedit_Start;
   typedef struct _Ecore_IMF_Event_Preedit_End        Ecore_IMF_Event_Preedit_End;
   typedef struct _Ecore_IMF_Event_Preedit_Changed    Ecore_IMF_Event_Preedit_Changed;
   typedef struct _Ecore_IMF_Event_Commit             Ecore_IMF_Event_Commit;
   typedef struct _Ecore_IMF_Event_Delete_Surrounding Ecore_IMF_Event_Delete_Surrounding;

   typedef struct _Ecore_IMF_Context                  Ecore_IMF_Context;                  /**< An Input Method Context */
   typedef struct _Ecore_IMF_Context_Class            Ecore_IMF_Context_Class;            /**< An Input Method Context class */
   typedef struct _Ecore_IMF_Context_Info             Ecore_IMF_Context_Info;             /**< An Input Method Context info */

   EAPI extern int ECORE_IMF_EVENT_PREEDIT_START;
   EAPI extern int ECORE_IMF_EVENT_PREEDIT_END;
   EAPI extern int ECORE_IMF_EVENT_PREEDIT_CHANGED;
   EAPI extern int ECORE_IMF_EVENT_COMMIT;
   EAPI extern int ECORE_IMF_EVENT_DELETE_SURROUNDIND;

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

   struct _Ecore_IMF_Context_Class
     {
	void (*add)                 (Ecore_IMF_Context *ctx);
	void (*del)                 (Ecore_IMF_Context *ctx);
	void (*client_window_set)   (Ecore_IMF_Context *ctx, void *window);
	void (*show)                (Ecore_IMF_Context *ctx);
	void (*hide)                (Ecore_IMF_Context *ctx);
	void (*preedit_string_get)  (Ecore_IMF_Context *ctx, char **str, int *cursor_pos);
	void (*focus_in)            (Ecore_IMF_Context *ctx);
	void (*focus_out)           (Ecore_IMF_Context *ctx);
	void (*reset)               (Ecore_IMF_Context *ctx);
	void (*cursor_position_set) (Ecore_IMF_Context *ctx, int cursor_pos);
	void (*use_preedit_set)     (Ecore_IMF_Context *ctx, int use_preedit);
	void (*input_mode_set)      (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode);
	int  (*filter_event)        (Ecore_IMF_Context *ctx, Evas_Callback_Type type, void *event_info);
     };

   struct _Ecore_IMF_Context_Info
     {
	const char *id;              /* ID */
	const char *description;     /* Human readable description */
	const char *default_locales; /* Languages for which this context is the default, separated by : */
     };

   EAPI int                           ecore_imf_init(void);
   EAPI int                           ecore_imf_shutdown(void);

   EAPI Ecore_List                   *ecore_imf_context_available_ids_get(void);
   EAPI const char                   *ecore_imf_context_default_id_get(void);
   EAPI const Ecore_IMF_Context_Info *ecore_imf_context_info_by_id_get(const char *id);

   EAPI Ecore_IMF_Context            *ecore_imf_context_add(const char *id);
   EAPI void                          ecore_imf_context_del(Ecore_IMF_Context *ctx);
   EAPI void                          ecore_imf_context_client_window_set(Ecore_IMF_Context *ctx, void *window);
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
   EAPI int                           ecore_imf_context_filter_event(Ecore_IMF_Context *ctx, Evas_Callback_Type type, void *event_info);

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
