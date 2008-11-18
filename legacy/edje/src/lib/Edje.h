#ifndef _EDJE_H
#define _EDJE_H

#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EDJE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EDJE_BUILD */
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

/* FIXDOC: Define these? */
enum _Edje_Message_Type
{
   EDJE_MESSAGE_NONE = 0,

     EDJE_MESSAGE_SIGNAL = 1, /* DONT USE THIS */

     EDJE_MESSAGE_STRING = 2,
     EDJE_MESSAGE_INT = 3,
     EDJE_MESSAGE_FLOAT = 4,

     EDJE_MESSAGE_STRING_SET = 5,
     EDJE_MESSAGE_INT_SET = 6,
     EDJE_MESSAGE_FLOAT_SET = 7,

     EDJE_MESSAGE_STRING_INT = 8,
     EDJE_MESSAGE_STRING_FLOAT = 9,

     EDJE_MESSAGE_STRING_INT_SET = 10,
     EDJE_MESSAGE_STRING_FLOAT_SET = 11
};
typedef enum _Edje_Message_Type Edje_Message_Type;

enum _Edje_Aspect_Control
{
   EDJE_ASPECT_CONTROL_NONE = 0,
   EDJE_ASPECT_CONTROL_NEITHER = 1,
   EDJE_ASPECT_CONTROL_HORIZONTAL = 2,
   EDJE_ASPECT_CONTROL_VERTICAL = 3,
   EDJE_ASPECT_CONTROL_BOTH = 4
};
typedef enum _Edje_Aspect_Control Edje_Aspect_Control;

typedef struct _Edje_Message_String           Edje_Message_String;
typedef struct _Edje_Message_Int              Edje_Message_Int;
typedef struct _Edje_Message_Float            Edje_Message_Float;
typedef struct _Edje_Message_String_Set       Edje_Message_String_Set;
typedef struct _Edje_Message_Int_Set          Edje_Message_Int_Set;
typedef struct _Edje_Message_Float_Set        Edje_Message_Float_Set;
typedef struct _Edje_Message_String_Int       Edje_Message_String_Int;
typedef struct _Edje_Message_String_Float     Edje_Message_String_Float;
typedef struct _Edje_Message_String_Int_Set   Edje_Message_String_Int_Set;
typedef struct _Edje_Message_String_Float_Set Edje_Message_String_Float_Set;

struct _Edje_Message_String
{
   char *str;
};

struct _Edje_Message_Int
{
   int val;
};

struct _Edje_Message_Float
{
   double val;
};

struct _Edje_Message_String_Set
{
   int count;
   char *str[1];
};

struct _Edje_Message_Int_Set
{
   int count;
   int val[1];
};

struct _Edje_Message_Float_Set
{
   int count;
   double val[1];
};

struct _Edje_Message_String_Int
{
   char *str;
   int val;
};

struct _Edje_Message_String_Float
{
   char *str;
   double val;
};

struct _Edje_Message_String_Int_Set
{
   char *str;
   int count;
   int val[1];
};

struct _Edje_Message_String_Float_Set
{
   char *str;
   int count;
   double val[1];
};

enum
{
   EDJE_DRAG_DIR_NONE = 0,
     EDJE_DRAG_DIR_X = 1,
     EDJE_DRAG_DIR_Y = 2,
     EDJE_DRAG_DIR_XY = 3
};

enum
{
   EDJE_LOAD_ERROR_NONE = 0,
     EDJE_LOAD_ERROR_GENERIC = 1,
     EDJE_LOAD_ERROR_DOES_NOT_EXIST = 2,
     EDJE_LOAD_ERROR_PERMISSION_DENIED = 3,
     EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = 4,
     EDJE_LOAD_ERROR_CORRUPT_FILE = 5,
     EDJE_LOAD_ERROR_UNKNOWN_FORMAT = 6,
     EDJE_LOAD_ERROR_INCOMPATIBLE_FILE = 7,
     EDJE_LOAD_ERROR_UNKNOWN_COLLECTION = 8,
     EDJE_LOAD_ERROR_RECURSIVE_REFERENCE = 9
};

#ifdef __cplusplus
extern "C" {
#endif

   /* edje_main.c */
   EAPI int          edje_init                       (void);
   EAPI int          edje_shutdown                   (void);

   /* edje_program.c */
   EAPI void         edje_frametime_set              (double t);
   EAPI double       edje_frametime_get              (void);

   /* edje_util.c */
   EAPI void         edje_freeze                     (void);
   EAPI void         edje_thaw                       (void);
   EAPI void         edje_fontset_append_set         (const char *fonts);
   EAPI const char  *edje_fontset_append_get         (void);
   EAPI void         edje_scale_set                  (double scale);
   EAPI double       edje_scale_get                  (void);

   /* edje_load.c */
   EAPI Eina_List   *edje_file_collection_list       (const char *file);
   EAPI void         edje_file_collection_list_free  (Eina_List *lst);
   EAPI int          edje_file_group_exists          (const char *file, const char *glob);
   EAPI char        *edje_file_data_get              (const char *file, const char *key);
   EAPI void         edje_file_cache_set             (int count);
   EAPI int          edje_file_cache_get             (void);
   EAPI void         edje_file_cache_flush           (void);
   EAPI void         edje_collection_cache_set       (int count);
   EAPI int          edje_collection_cache_get       (void);
   EAPI void         edje_collection_cache_flush     (void);


   /* edje_util.c */
   EAPI void         edje_color_class_set(const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);
   EAPI void         edje_color_class_del(const char *color_class);
   EAPI Eina_List *  edje_color_class_list(void);
   EAPI void         edje_text_class_set(const char *text_class, const char *font, Evas_Font_Size size);
   EAPI void         edje_text_class_del(const char *text_class);
   EAPI Eina_List *  edje_text_class_list(void);
   EAPI void         edje_extern_object_min_size_set (Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);
   EAPI void         edje_extern_object_max_size_set (Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh);
   EAPI void         edje_extern_object_aspect_set(Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah);

   /* edje_smart.c */
   EAPI Evas_Object *edje_object_add                 (Evas *evas);

   /* edje_util.c */
   EAPI const char  *edje_object_data_get            (const Evas_Object *obj, const char *key);

   /* edje_load.c */
   EAPI int          edje_object_file_set            (Evas_Object *obj, const char *file, const char *group);
   EAPI void         edje_object_file_get            (const Evas_Object *obj, const char **file, const char **group);
   EAPI int          edje_object_load_error_get      (const Evas_Object *obj);

   /* edje_program.c */
   EAPI void         edje_object_signal_callback_add (Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *obj, const char *emission, const char *source), void *data);
   EAPI void        *edje_object_signal_callback_del (Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *obj, const char *emission, const char *source));
   EAPI void         edje_object_signal_emit         (Evas_Object *obj, const char *emission, const char *source);
   EAPI void         edje_object_play_set            (Evas_Object *obj, int play);
   EAPI int          edje_object_play_get            (const Evas_Object *obj);
   EAPI void         edje_object_animation_set       (Evas_Object *obj, int on);
   EAPI int          edje_object_animation_get       (const Evas_Object *obj);

   /* edje_util.c */
   EAPI int          edje_object_freeze              (Evas_Object *obj);
   EAPI int          edje_object_thaw                (Evas_Object *obj);
   EAPI void         edje_object_color_class_set     (Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);
   EAPI void         edje_object_color_class_del     (Evas_Object *obj, const char *color_class);
   EAPI void         edje_object_text_class_set      (Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size);
   EAPI void         edje_object_size_min_get        (const Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);
   EAPI void         edje_object_size_max_get        (const Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh);
   EAPI void         edje_object_calc_force          (Evas_Object *obj);
   EAPI void         edje_object_size_min_calc       (Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);
   EAPI void         edje_object_size_min_restricted_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh, Evas_Coord restrictedw, Evas_Coord restrictedh);
   EAPI int          edje_object_part_exists         (const Evas_Object *obj, const char *part);
   EAPI const Evas_Object *edje_object_part_object_get     (const Evas_Object *obj, const char *part);
   EAPI void         edje_object_part_geometry_get   (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
   EAPI void         edje_object_text_change_cb_set  (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, const char *part), void *data);
   EAPI void         edje_object_part_text_set       (Evas_Object *obj, const char *part, const char *text);
   EAPI const char  *edje_object_part_text_get       (const Evas_Object *obj, const char *part);
   EAPI const char  *edje_object_part_text_selection_get(const Evas_Object *obj, const char *part);
   EAPI void         edje_object_part_text_select_none(const Evas_Object *obj, const char *part);
   EAPI void         edje_object_part_text_select_all(const Evas_Object *obj, const char *part);
       
   EAPI void         edje_object_part_text_insert    (Evas_Object *obj, const char *part, const char *text);
   EAPI Eina_List   *edje_object_part_text_anchor_list_get(const Evas_Object *obj, const char *part);
   EAPI Eina_List   *edje_object_part_text_anchor_geometry_get(const Evas_Object *obj, const char *part, const char *anchor);
   EAPI void         edje_object_part_text_cursor_geometry_get(const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
       
   EAPI void         edje_object_part_swallow        (Evas_Object *obj, const char *part, Evas_Object *obj_swallow);
   EAPI void         edje_object_part_unswallow      (Evas_Object *obj, Evas_Object *obj_swallow);
   EAPI Evas_Object *edje_object_part_swallow_get    (const Evas_Object *obj, const char *part);
   EAPI const char  *edje_object_part_state_get      (const Evas_Object *obj, const char *part, double *val_ret);
   EAPI int          edje_object_part_drag_dir_get   (const Evas_Object *obj, const char *part);
   EAPI void         edje_object_part_drag_value_set (Evas_Object *obj, const char *part, double dx, double dy);
   EAPI void         edje_object_part_drag_value_get (const Evas_Object *obj, const char *part, double *dx, double *dy);
   EAPI void         edje_object_part_drag_size_set  (Evas_Object *obj, const char *part, double dw, double dh);
   EAPI void         edje_object_part_drag_size_get  (const Evas_Object *obj, const char *part, double *dw, double *dh);
   EAPI void         edje_object_part_drag_step_set  (Evas_Object *obj, const char *part, double dx, double dy);
   EAPI void         edje_object_part_drag_step_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);
   EAPI void         edje_object_part_drag_page_set  (Evas_Object *obj, const char *part, double dx, double dy);
   EAPI void         edje_object_part_drag_page_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);
   EAPI void         edje_object_part_drag_step      (Evas_Object *obj, const char *part, double dx, double dy);
   EAPI void         edje_object_part_drag_page      (Evas_Object *obj, const char *part, double dx, double dy);

   /* edje_message_queue.c */
   EAPI void         edje_object_message_send           (Evas_Object *obj, Edje_Message_Type type, int id, void *msg);
   EAPI void         edje_object_message_handler_set    (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg), void *data);
   EAPI void         edje_object_message_signal_process (Evas_Object *obj);

   EAPI void         edje_message_signal_process        (void);

#ifdef __cplusplus
}
#endif

#endif
