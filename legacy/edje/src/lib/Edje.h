#ifndef _EDJE_H
#define _EDJE_H

#include <stdint.h>
#include <math.h>

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

/** 
 * @file Edje.h
 * @brief Edje Graphical Design Library
 *
 * These routines are used for Edje.
 */


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

enum _Edje_Object_Table_Homogeneous_Mode
{
   EDJE_OBJECT_TABLE_HOMOGENEOUS_NONE = 0,
   EDJE_OBJECT_TABLE_HOMOGENEOUS_TABLE = 1,
   EDJE_OBJECT_TABLE_HOMOGENEOUS_ITEM = 2
};
typedef enum _Edje_Object_Table_Homogeneous_Mode Edje_Object_Table_Homogeneous_Mode;

typedef enum _Edje_Part_Type
{
   EDJE_PART_TYPE_NONE      = 0,
   EDJE_PART_TYPE_RECTANGLE = 1,
   EDJE_PART_TYPE_TEXT      = 2,
   EDJE_PART_TYPE_IMAGE     = 3,
   EDJE_PART_TYPE_SWALLOW   = 4,
   EDJE_PART_TYPE_TEXTBLOCK = 5,
   EDJE_PART_TYPE_GRADIENT  = 6,
   EDJE_PART_TYPE_GROUP     = 7,
   EDJE_PART_TYPE_BOX       = 8,
   EDJE_PART_TYPE_TABLE     = 9,
   EDJE_PART_TYPE_EXTERNAL  = 10,
   EDJE_PART_TYPE_LAST      = 11
} Edje_Part_Type;

typedef enum _Edje_Text_Effect
{
   EDJE_TEXT_EFFECT_NONE                = 0,
   EDJE_TEXT_EFFECT_PLAIN               = 1,
   EDJE_TEXT_EFFECT_OUTLINE             = 2,
   EDJE_TEXT_EFFECT_SOFT_OUTLINE        = 3,
   EDJE_TEXT_EFFECT_SHADOW              = 4,
   EDJE_TEXT_EFFECT_SOFT_SHADOW         = 5,
   EDJE_TEXT_EFFECT_OUTLINE_SHADOW      = 6,
   EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW = 7,
   EDJE_TEXT_EFFECT_FAR_SHADOW          = 8,
   EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW     = 9,
   EDJE_TEXT_EFFECT_GLOW                = 10,
   EDJE_TEXT_EFFECT_LAST                = 11
} Edje_Text_Effect;

typedef enum _Edje_Action_Type
{
   EDJE_ACTION_TYPE_NONE          = 0,
   EDJE_ACTION_TYPE_STATE_SET     = 1,
   EDJE_ACTION_TYPE_ACTION_STOP   = 2,
   EDJE_ACTION_TYPE_SIGNAL_EMIT   = 3,
   EDJE_ACTION_TYPE_DRAG_VAL_SET  = 4,
   EDJE_ACTION_TYPE_DRAG_VAL_STEP = 5,
   EDJE_ACTION_TYPE_DRAG_VAL_PAGE = 6,
   EDJE_ACTION_TYPE_SCRIPT        = 7,
   EDJE_ACTION_TYPE_FOCUS_SET     = 8,
   EDJE_ACTION_TYPE_LUA_SCRIPT    = 9,
   EDJE_ACTION_TYPE_LAST          = 10
} Edje_Action_Type;

typedef enum _Edje_Tween_Mode
{
   EDJE_TWEEN_MODE_NONE       = 0,
   EDJE_TWEEN_MODE_LINEAR     = 1,
   EDJE_TWEEN_MODE_SINUSOIDAL = 2,
   EDJE_TWEEN_MODE_ACCELERATE = 3,
   EDJE_TWEEN_MODE_DECELERATE = 4,
   EDJE_TWEEN_MODE_LAST       = 5
} Edje_Tween_Mode;

enum _Edje_Cursor
{
   EDJE_CURSOR_MAIN,
   EDJE_CURSOR_SELECTION_BEGIN,
   EDJE_CURSOR_SELECTION_END
   // more later
};
typedef enum _Edje_Cursor Edje_Cursor;

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

enum _Edje_External_Param_Type
{
  EDJE_EXTERNAL_PARAM_TYPE_INT,
  EDJE_EXTERNAL_PARAM_TYPE_DOUBLE,
  EDJE_EXTERNAL_PARAM_TYPE_STRING,
  EDJE_EXTERNAL_PARAM_TYPE_MAX
};
typedef enum _Edje_External_Param_Type Edje_External_Param_Type;

struct _Edje_External_Param
{
  const char *name;
  Edje_External_Param_Type type;
  // XXX these could be in a union, but eet doesn't support them (or does it?)
  int i;
  double d;
  const char *s;
};
typedef struct _Edje_External_Param Edje_External_Param;

#define EDJE_EXTERNAL_INT_UNSET INT32_MAX
#define EDJE_EXTERNAL_DOUBLE_UNSET NAN

struct _Edje_External_Param_Info
{
   const char *name;
   Edje_External_Param_Type type;
   union
   {
      struct
      {
	 int def, min, max, step;
      } i;
      struct
      {
	 double def, min, max, step;
      } d;
      struct
      {
	 const char *def;
	 const char *accept_fmt;
	 const char *deny_fmt;
      } s;
   } info;
};
typedef struct _Edje_External_Param_Info Edje_External_Param_Info;

#define EDJE_EXTERNAL_PARAM_INFO_INT_FULL(name, def, min, max, step) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_INT, {.i = {def, min, max, step}}}
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL(name, def, min, max, step) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_DOUBLE, {.d = {def, min, max, step}}}
#define EDJE_EXTERNAL_PARAM_INFO_STRING_FULL(name, def, accept, deny) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_STRING, {.s = {def, accept, deny}}}

#define EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_INT_FULL(name, def, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL(name, def, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_STRING_FULL(name, def, NULL, NULL)

#define EDJE_EXTERNAL_PARAM_INFO_INT(name) \
   EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT(name, 0)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE(name) \
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT(name, 0.0)
#define EDJE_EXTERNAL_PARAM_INFO_STRING(name) \
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT(name, NULL)

#define EDJE_EXTERNAL_PARAM_INFO_SENTINEL {NULL, 0, {.s = {NULL, NULL, NULL}}}

struct _Edje_External_Type
{
#define EDJE_EXTERNAL_TYPE_ABI_VERSION (1)
  unsigned int abi_version; /**< always use:
			     *  - #EDJE_EXTERNAL_TYPE_ABI_VERSION to declare.
			     *  - edje_external_type_abi_version_get() to check.
			     */

  const char *module;
  Evas_Object *(*add) (void *data, Evas *evas, Evas_Object *parent, const Eina_List *params);
  void (*state_set) (void *data, Evas_Object *obj, const void *from_params, const void *to_params, float pos);
  void (*signal_emit) (void *data, Evas_Object *obj, const char *emission, const char *source);
  void *(*params_parse) (void *data, Evas_Object *obj, const Eina_List *params);
  void (*params_free) (void *params);

  const char *(*label_get) (void *data);
  const char *(*description_get) (void *data);
  Evas_Object *(*icon_add) (void *data, Evas *e);
  Evas_Object *(*preview_add) (void *data, Evas *e);

  Edje_External_Param_Info *parameters_info;

  void *data;
};
typedef struct _Edje_External_Type Edje_External_Type;


struct _Edje_External_Type_Info
{
   const char *name;
   const Edje_External_Type *info;
};
typedef struct _Edje_External_Type_Info Edje_External_Type_Info;



typedef void (*Edje_Signal_Cb) (void *data, Evas_Object *obj, const char *emission, const char *source);
typedef void (*Edje_Text_Change_Cb) (void *data, Evas_Object *obj, const char *part);
typedef void (*Edje_Message_Handler_Cb) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg);

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
   EAPI void         edje_object_scale_set           (Evas_Object *obj, double scale);
   EAPI double       edje_object_scale_get           (const Evas_Object *obj);
       
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
   EAPI Eina_Bool    edje_color_class_get(const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);
   EAPI void         edje_color_class_del(const char *color_class);
   EAPI Eina_List *  edje_color_class_list(void);
   EAPI void         edje_text_class_set(const char *text_class, const char *font, Evas_Font_Size size);
   EAPI void         edje_text_class_del(const char *text_class);
   EAPI Eina_List *  edje_text_class_list(void);
   EAPI void         edje_extern_object_min_size_set (Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);
   EAPI void         edje_extern_object_max_size_set (Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh);
   EAPI void         edje_extern_object_aspect_set(Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah);
   EAPI void         edje_box_layout_register(const char *name, Evas_Object_Box_Layout func, void *(*layout_data_get)(void *), void (*layout_data_free)(void *), void (*free_data)(void *), void *data);

   /* edje_smart.c */
   EAPI Evas_Object *edje_object_add                 (Evas *evas);

   /* edje_util.c */
   EAPI const char  *edje_object_data_get            (const Evas_Object *obj, const char *key);

   /* edje_load.c */
   EAPI int          edje_object_file_set            (Evas_Object *obj, const char *file, const char *group);
   EAPI void         edje_object_file_get            (const Evas_Object *obj, const char **file, const char **group);
   EAPI int          edje_object_load_error_get      (const Evas_Object *obj);
   EAPI const char  *edje_load_error_str             (int error);

   /* edje_program.c */
   EAPI void         edje_object_signal_callback_add (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);
   EAPI void        *edje_object_signal_callback_del (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func);
   EAPI void         edje_object_signal_emit         (Evas_Object *obj, const char *emission, const char *source);
   EAPI void         edje_object_play_set            (Evas_Object *obj, int play);
   EAPI int          edje_object_play_get            (const Evas_Object *obj);
   EAPI void         edje_object_animation_set       (Evas_Object *obj, int on);
   EAPI int          edje_object_animation_get       (const Evas_Object *obj);

   /* edje_util.c */
   EAPI int          edje_object_freeze              (Evas_Object *obj);
   EAPI int          edje_object_thaw                (Evas_Object *obj);
   EAPI void         edje_object_color_class_set     (Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);
   EAPI Eina_Bool     edje_object_color_class_get    (const Evas_Object *o, const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);
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
   EAPI void         edje_object_text_change_cb_set  (Evas_Object *obj, Edje_Text_Change_Cb func, void *data);
   EAPI void         edje_object_part_text_set       (Evas_Object *obj, const char *part, const char *text);
   EAPI const char  *edje_object_part_text_get       (const Evas_Object *obj, const char *part);
   EAPI void         edje_object_part_text_unescaped_set(Evas_Object *obj, const char *part, const char *text_to_escape);
   EAPI char        *edje_object_part_text_unescaped_get(const Evas_Object *obj, const char *part);

   EAPI const char      *edje_object_part_text_selection_get           (const Evas_Object *obj, const char *part);
   EAPI void             edje_object_part_text_select_none             (const Evas_Object *obj, const char *part);
   EAPI void             edje_object_part_text_select_all              (const Evas_Object *obj, const char *part);
   EAPI void             edje_object_part_text_insert                  (Evas_Object *obj, const char *part, const char *text);
   EAPI const Eina_List *edje_object_part_text_anchor_list_get         (const Evas_Object *obj, const char *part);
   EAPI const Eina_List *edje_object_part_text_anchor_geometry_get     (const Evas_Object *obj, const char *part, const char *anchor);
   EAPI void             edje_object_part_text_cursor_geometry_get     (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
   EAPI void             edje_object_part_text_select_allow_set        (const Evas_Object *obj, const char *part, Eina_Bool allow);
   EAPI void             edje_object_part_text_select_abort            (const Evas_Object *obj, const char *part);
   
   EAPI Eina_Bool        edje_object_part_text_cursor_next(const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI Eina_Bool        edje_object_part_text_cursor_prev(const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI Eina_Bool        edje_object_part_text_cursor_up(const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI Eina_Bool        edje_object_part_text_cursor_down(const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI void             edje_object_part_text_cursor_begin_set(const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI void             edje_object_part_text_cursor_end_set(const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI void             edje_object_part_text_cursor_copy(const Evas_Object *obj, const char *part, Edje_Cursor src, Edje_Cursor dst);
   EAPI void             edje_object_part_text_cursor_line_begin_set(const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI void             edje_object_part_text_cursor_line_end_set(const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI Eina_Bool        edje_object_part_text_cursor_is_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI Eina_Bool        edje_object_part_text_cursor_is_visible_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI const char      *edje_object_part_text_cursor_content_get(const Evas_Object *obj, const char *part, Edje_Cursor cur);
       
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
   EAPI Eina_Bool    edje_object_part_box_append     (Evas_Object *obj, const char *part, Evas_Object *child);
   EAPI Eina_Bool    edje_object_part_box_prepend    (Evas_Object *obj, const char *part, Evas_Object *child);
   EAPI Eina_Bool    edje_object_part_box_insert_before (Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference);
   EAPI Eina_Bool    edje_object_part_box_insert_at  (Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos);
   EAPI Evas_Object *edje_object_part_box_remove     (Evas_Object *obj, const char *part, Evas_Object *child);
   EAPI Evas_Object *edje_object_part_box_remove_at  (Evas_Object *obj, const char *part, unsigned int pos);
   EAPI Eina_Bool    edje_object_part_box_remove_all (Evas_Object *obj, const char *part, Eina_Bool clear);
   EAPI Eina_Bool    edje_object_part_table_pack     (Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);
   EAPI Eina_Bool    edje_object_part_table_unpack   (Evas_Object *obj, const char *part, Evas_Object *child_obj);
   EAPI Eina_Bool    edje_object_part_table_col_row_size_get (const Evas_Object *obj, const char *part, int *cols, int *rows);
   EAPI Eina_Bool    edje_object_part_table_clear    (Evas_Object *obj, const char *part, Eina_Bool clear);

   /* edje_message_queue.c */
   EAPI void         edje_object_message_send           (Evas_Object *obj, Edje_Message_Type type, int id, void *msg);
   EAPI void         edje_object_message_handler_set    (Evas_Object *obj, Edje_Message_Handler_Cb func, void *data);
   EAPI void         edje_object_message_signal_process (Evas_Object *obj);

   EAPI void         edje_message_signal_process        (void);

   /* edje_external.c */
  EAPI Eina_Bool edje_external_type_register(const char *type_name, const Edje_External_Type *type_info);
  EAPI Eina_Bool edje_external_type_unregister(const char *type_name);

  EAPI void      edje_external_type_array_register(const Edje_External_Type_Info *array);
  EAPI void      edje_external_type_array_unregister(const Edje_External_Type_Info *array);

  EAPI unsigned int edje_external_type_abi_version_get(void) EINA_CONST;


  EAPI Eina_Iterator *edje_external_iterator_get(void);
  EAPI Edje_External_Param *edje_external_param_find(const Eina_List *params, const char *key);
  EAPI Eina_Bool edje_external_param_int_get(const Eina_List *params, const char *key, int *ret);
  EAPI Eina_Bool edje_external_param_double_get(const Eina_List *params, const char *key, double *ret);
  EAPI Eina_Bool edje_external_param_string_get(const Eina_List *params, const char *key, const char **ret);
  EAPI const Edje_External_Param_Info *edje_external_param_info_get(const char *type_name);

  /* edje_module.c */
  EAPI Eina_Bool edje_module_load(const char *module);
  EAPI const Eina_List *edje_available_modules_get(void);


#ifdef __cplusplus
}
#endif

#endif
