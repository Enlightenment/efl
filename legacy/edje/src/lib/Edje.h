#ifndef _EDJE_H
#define _EDJE_H

#ifndef _MSC_VER
# include <stdint.h>
#endif
#include <math.h>
#include <float.h>
#include <limits.h>

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

#ifdef __cplusplus
extern "C" {
#endif
   
#define EDJE_VERSION_MAJOR 1
#define EDJE_VERSION_MINOR 0
   
   typedef struct _Edje_Version
     {
        int major;
        int minor;
        int micro;
        int revision;
     } Edje_Version;
   
   EAPI extern Edje_Version *edje_version;
   
/**
 * @file Edje.h
 * @brief Edje Graphical Design Library
 *
 * These routines are used for Edje.
 */

typedef enum _Edje_Message_Type
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
} Edje_Message_Type;

typedef enum _Edje_Aspect_Control
{
   EDJE_ASPECT_CONTROL_NONE = 0,
   EDJE_ASPECT_CONTROL_NEITHER = 1,
   EDJE_ASPECT_CONTROL_HORIZONTAL = 2,
   EDJE_ASPECT_CONTROL_VERTICAL = 3,
   EDJE_ASPECT_CONTROL_BOTH = 4
} Edje_Aspect_Control;

typedef enum _Edje_Object_Table_Homogeneous_Mode
{
   EDJE_OBJECT_TABLE_HOMOGENEOUS_NONE = 0,
   EDJE_OBJECT_TABLE_HOMOGENEOUS_TABLE = 1,
   EDJE_OBJECT_TABLE_HOMOGENEOUS_ITEM = 2
} Edje_Object_Table_Homogeneous_Mode;

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
   EDJE_ACTION_TYPE_RESERVED00    = 9,
   EDJE_ACTION_TYPE_FOCUS_OBJECT  = 10,
   EDJE_ACTION_TYPE_PARAM_COPY    = 11,
   EDJE_ACTION_TYPE_PARAM_SET     = 12,
   EDJE_ACTION_TYPE_LAST          = 13
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

typedef enum _Edje_Cursor
{
   EDJE_CURSOR_MAIN,
   EDJE_CURSOR_SELECTION_BEGIN,
   EDJE_CURSOR_SELECTION_END,
   EDJE_CURSOR_PREEDIT_START,
   EDJE_CURSOR_PREEDIT_END,
   EDJE_CURSOR_USER,
   EDJE_CURSOR_USER_EXTRA,
   // more later
} Edje_Cursor;

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

typedef enum _Edje_Drag_Dir
{
   EDJE_DRAG_DIR_NONE = 0,
   EDJE_DRAG_DIR_X = 1,
   EDJE_DRAG_DIR_Y = 2,
   EDJE_DRAG_DIR_XY = 3
} Edje_Drag_Dir;

typedef enum _Edje_Load_Error
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
} Edje_Load_Error;

typedef enum _Edje_Text_Filter_Type
{
   EDJE_TEXT_FILTER_TEXT = 0,
   EDJE_TEXT_FILTER_FORMAT = 1,
   EDJE_TEXT_FILTER_MARKUP = 2
} Edje_Text_Filter_Type;
   
typedef enum _Edje_External_Param_Type
{
  EDJE_EXTERNAL_PARAM_TYPE_INT,
  EDJE_EXTERNAL_PARAM_TYPE_DOUBLE,
  EDJE_EXTERNAL_PARAM_TYPE_STRING,
  EDJE_EXTERNAL_PARAM_TYPE_BOOL,
  EDJE_EXTERNAL_PARAM_TYPE_CHOICE,
  EDJE_EXTERNAL_PARAM_TYPE_MAX
} Edje_External_Param_Type;

/**
 * @typedef Edje_External_Param_Flags flags that determines the
 * behavior of a parameter.
 *
 * @var EDJE_EXTERNAL_PARAM_FLAGS_NONE property is incapable of
 *      operations, this is used to catch bogus flags.
 * @var EDJE_EXTERNAL_PARAM_FLAGS_GET property can be read/get
 * @var EDJE_EXTERNAL_PARAM_FLAGS_SET property can be written/set.
 *      This only enables edje_object_part_external_param_set() and
 *      Embryo scripts. To enable parameter being set from state
 *      description whenever it changes state, use
 *      #EDJE_EXTERNAL_PARAM_FLAGS_STATE.
 * @var EDJE_EXTERNAL_PARAM_FLAGS_STATE property can be set from state
 *      description.
 * @var EDJE_EXTERNAL_PARAM_FLAGS_CONSTRUCTOR this property is only
 *      set once when object is constructed using its value from
 *      "default" 0.0 state description. Setting this overrides
 *      #EDJE_EXTERNAL_PARAM_FLAGS_STATE.
 * @var EDJE_EXTERNAL_PARAM_FLAGS_REGULAR convenience flag that sets
 *      property as GET, SET and STATE.
 */
typedef enum _Edje_External_Param_Flags
{
  EDJE_EXTERNAL_PARAM_FLAGS_NONE        = 0,
  EDJE_EXTERNAL_PARAM_FLAGS_GET         = (1 << 0),
  EDJE_EXTERNAL_PARAM_FLAGS_SET         = (1 << 1),
  EDJE_EXTERNAL_PARAM_FLAGS_STATE       = (1 << 2),
  EDJE_EXTERNAL_PARAM_FLAGS_CONSTRUCTOR = (1 << 3),
  EDJE_EXTERNAL_PARAM_FLAGS_REGULAR     = (EDJE_EXTERNAL_PARAM_FLAGS_GET |
                                           EDJE_EXTERNAL_PARAM_FLAGS_SET |
                                           EDJE_EXTERNAL_PARAM_FLAGS_STATE)
} Edje_External_Param_Flags;

EAPI const char *edje_external_param_type_str(Edje_External_Param_Type type) EINA_PURE;

struct _Edje_External_Param
{
   const char               *name;
   Edje_External_Param_Type  type;
   // XXX these could be in a union, but eet doesn't support them (or does it?)
   int                       i; /**< used by both integer and boolean */
   double                    d;
   const char               *s; /**< used by both string and choice */
};
typedef struct _Edje_External_Param Edje_External_Param;

#define EDJE_EXTERNAL_INT_UNSET INT_MAX
#define EDJE_EXTERNAL_DOUBLE_UNSET DBL_MAX

typedef struct _Edje_External_Param_Info Edje_External_Param_Info;
struct _Edje_External_Param_Info
{
   const char               *name;
   Edje_External_Param_Type  type;
   Edje_External_Param_Flags flags;
   union {
      struct {
         int                 def, min, max, step;
      } i;
      struct {
         double              def, min, max, step;
      } d;
      struct {
         const char         *def;
         const char         *accept_fmt;
         const char         *deny_fmt;
      } s;
      struct {
         int                 def;
         const char         *false_str;
         const char         *true_str;
      } b;
      struct {
         const char         *def;
         const char        **choices; /* NULL terminated array */
         char               *(*def_get)(void *data, const Edje_External_Param_Info *info); /* return malloc() memory with the default choice, should be used if def is NULL. First parameter is Edje_External_Type::data */
         char              **(*query)(void *data, const Edje_External_Param_Info *info); /* NULL terminated array of strings, memory is dynamically allocated and should be freed with free() for array and each element. First parameter is Edje_External_Type::data */
      } c;
   } info;
};

#define EDJE_EXTERNAL_PARAM_INFO_INT_FULL_FLAGS(name, def, min, max, step, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_INT, flags, {.i = {def, min, max, step}}}
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL_FLAGS(name, def, min, max, step, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_DOUBLE, flags, {.d = {def, min, max, step}}}
#define EDJE_EXTERNAL_PARAM_INFO_STRING_FULL_FLAGS(name, def, accept, deny, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_STRING, flags, {.s = {def, accept, deny}}}
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL_FLAGS(name, def, false_str, true_str, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_BOOL, flags, {.b = {def, false_str, true_str}}}
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL_FLAGS(name, def, choices, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_CHOICE, flags, {.c = {def, choices, NULL, NULL}}}
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_DYNAMIC_FULL_FLAGS(name, def_get, query, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_CHOICE, flags, {.c = {NULL, NULL, def_get, query}}}

#define EDJE_EXTERNAL_PARAM_INFO_INT_FULL(name, def, min, max, step) \
  EDJE_EXTERNAL_PARAM_INFO_INT_FULL_FLAGS(name, def, min, max, step, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL(name, def, min, max, step) \
  EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL_FLAGS(name, def, min, max, step, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_FULL(name, def, accept, deny) \
  EDJE_EXTERNAL_PARAM_INFO_STRING_FULL_FLAGS(name, def, accept, deny, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL(name, def, false_str, true_str) \
  EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL_FLAGS(name, def, false_str, true_str, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL(name, def, choices) \
  EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL_FLAGS(name, def, choices, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_DYNAMIC_FULL(name, def_get, query) \
  EDJE_EXTERNAL_PARAM_INFO_CHOICE_DYNAMIC_FULL_FLAGS(name, def_get, query, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)

#define EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_INT_FULL(name, def, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL(name, def, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_STRING_FULL(name, def, NULL, NULL)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL(name, def, "false", "true")

#define EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT_FLAGS(name, def, flags)    \
  EDJE_EXTERNAL_PARAM_INFO_INT_FULL_FLAGS(name, def, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET, flags)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT_FLAGS(name, def, flags) \
  EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL_FLAGS(name, def, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET, flags)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT_FLAGS(name, def, flags) \
  EDJE_EXTERNAL_PARAM_INFO_STRING_FULL_FLAGS(name, def, NULL, NULL, flags)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT_FLAGS(name, def, flags)   \
  EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL_FLAGS(name, def, "false", "true", flags)

#define EDJE_EXTERNAL_PARAM_INFO_INT(name) \
   EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT(name, 0)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE(name) \
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT(name, 0.0)
#define EDJE_EXTERNAL_PARAM_INFO_STRING(name) \
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT(name, NULL)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL(name) \
   EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT(name, 0)

#define EDJE_EXTERNAL_PARAM_INFO_INT_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT_FLAGS(name, 0, flags)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT_FLAGS(name, 0.0, flags)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT_FLAGS(name, NULL, flags)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT_FLAGS(name, 0, flags)

#define EDJE_EXTERNAL_PARAM_INFO_SENTINEL {NULL, 0, 0, {.s = {NULL, NULL, NULL}}}

/**
 * @struct _Edje_External_Type
 *
 * @brief Information about an external type to be used.
 *
 * This structure provides information on how to display and modify a
 * third party Evas_Object in Edje.
 *
 * Some function pointers are not really used by Edje, but provide
 * means for Edje users to better interact with such objects. For
 * instance, an editor may use label_get() and icon_get() to list all
 * registered external types.
 *
 * @note The function pointers provided in this structure must check
 *       for errors and invalid or out-of-range values as for
 *       performance reasons Edje will not enforce hints provided as
 *       #Edje_External_Param_Info in the member parameters_info.
 */
struct _Edje_External_Type
{
#define EDJE_EXTERNAL_TYPE_ABI_VERSION (3)
  unsigned int  abi_version; /**< always use:
                              *  - #EDJE_EXTERNAL_TYPE_ABI_VERSION to declare.
                              *  - edje_external_type_abi_version_get() to check.
                              */
  const char    *module;
  const char    *module_name;
  Evas_Object *(*add) (void *data, Evas *evas, Evas_Object *parent, const Eina_List *params, const char *part_name); /**< creates the object to be used by Edje as the part */
  void         (*state_set) (void *data, Evas_Object *obj, const void *from_params, const void *to_params, float pos); /**< called upon state changes, including the initial "default" 0.0 state. Parameters are the value returned by params_parse() */
  void         (*signal_emit) (void *data, Evas_Object *obj, const char *emission, const char *source); /**< Feed a signal emitted with emission originally set as part_name:signal to this object (without the "part_name:" prefix) */
  Eina_Bool    (*param_set) (void *data, Evas_Object *obj, const Edje_External_Param *param); /**< dynamically change a parameter of this external, called by scripts and user code. Returns @c EINA_TRUE on success */
  Eina_Bool    (*param_get) (void *data, const Evas_Object *obj, Edje_External_Param *param); /**< dynamically fetch a parameter of this external, called by scripts and user code. Returns @c EINA_TRUE on success. (Must check parameter name and type!) */
  Evas_Object *(*content_get) (void *data, const Evas_Object *obj, const char *content); /**< dynamically fetch a sub object of this external, called by scripts and user code. Returns @c Evas_Object * on success. (Must check parameter name and type!) */
  void        *(*params_parse) (void *data, Evas_Object *obj, const Eina_List *params); /**< parses the list of parameters, converting into a friendly representation. Used with state_set() */
  void         (*params_free) (void *params); /**< free parameters parsed with params_parse() */

  /* The following callbacks aren't used by Edje itself, but by UI design
     tools instead */
  const char  *(*label_get) (void *data);
  const char  *(*description_get) (void *data);
  Evas_Object *(*icon_add) (void *data, Evas *e);
  Evas_Object *(*preview_add) (void *data, Evas *e);
  const char  *(*translate) (void *data, const char *orig); /**< called to translate parameters_info name properties for use in user interfaces that support internationalization (i18n) */

  Edje_External_Param_Info *parameters_info;
  void                     *data;
};
typedef struct _Edje_External_Type Edje_External_Type;

struct _Edje_External_Type_Info
{
   const char               *name;
   const Edje_External_Type *info;
};
typedef struct _Edje_External_Type_Info Edje_External_Type_Info;

typedef void         (*Edje_Signal_Cb)          (void *data, Evas_Object *obj, const char *emission, const char *source);
typedef void         (*Edje_Text_Change_Cb)     (void *data, Evas_Object *obj, const char *part);
typedef void         (*Edje_Message_Handler_Cb) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg);
typedef void         (*Edje_Text_Filter_Cb)     (void *data, Evas_Object *obj, const char *part, Edje_Text_Filter_Type type, char **text);
typedef Evas_Object *(*Edje_Item_Provider_Cb)   (void *data, Evas_Object *obj, const char *part, const char *item);

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
   EAPI Eina_Bool    edje_object_scale_set           (Evas_Object *obj, double scale);
   EAPI double       edje_object_scale_get           (const Evas_Object *obj);
   EAPI void         edje_object_mirrored_set        (Evas_Object *obj, Eina_Bool rtl);
   EAPI Eina_Bool    edje_object_mirrored_get        (const Evas_Object *obj);

   /* edje_load.c */
   EAPI Eina_List   *edje_file_collection_list       (const char *file);
   EAPI void         edje_file_collection_list_free  (Eina_List *lst);
   EAPI Eina_Bool    edje_file_group_exists          (const char *file, const char *glob);
   EAPI char        *edje_file_data_get              (const char *file, const char *key);
   EAPI void         edje_file_cache_set             (int count);
   EAPI int          edje_file_cache_get             (void);
   EAPI void         edje_file_cache_flush           (void);
   EAPI void         edje_collection_cache_set       (int count);
   EAPI int          edje_collection_cache_get       (void);
   EAPI void         edje_collection_cache_flush     (void);

   /* edje_util.c */
   EAPI Eina_Bool    edje_color_class_set            (const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);
   EAPI Eina_Bool    edje_color_class_get            (const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);
   EAPI void         edje_color_class_del            (const char *color_class);
   EAPI Eina_List   *edje_color_class_list           (void);
   EAPI Eina_Bool    edje_text_class_set             (const char *text_class, const char *font, Evas_Font_Size size);
   EAPI void         edje_text_class_del             (const char *text_class);
   EAPI Eina_List   *edje_text_class_list            (void);
   EAPI void         edje_extern_object_min_size_set (Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);
   EAPI void         edje_extern_object_max_size_set (Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh);
   EAPI void         edje_extern_object_aspect_set   (Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah);
   EAPI void         edje_box_layout_register        (const char *name, Evas_Object_Box_Layout func, void *(*layout_data_get)(void *), void (*layout_data_free)(void *), void (*free_data)(void *), void *data);

   /* edje_smart.c */
   EAPI Evas_Object *edje_object_add                 (Evas *evas);

   /* edje_util.c */
   EAPI const char  *edje_object_data_get            (const Evas_Object *obj, const char *key);

   /* edje_load.c */
   EAPI Eina_Bool        edje_object_file_set        (Evas_Object *obj, const char *file, const char *group);
   EAPI void             edje_object_file_get        (const Evas_Object *obj, const char **file, const char **group);
   EAPI Edje_Load_Error  edje_object_load_error_get  (const Evas_Object *obj);
   EAPI const char      *edje_load_error_str         (Edje_Load_Error error);

   /* edje_util.c */
   EAPI Eina_Bool    edje_object_preload             (Evas_Object *obj, Eina_Bool cancel);

   /* edje_program.c */
   EAPI void         edje_object_signal_callback_add (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);
   EAPI void        *edje_object_signal_callback_del (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func);
   EAPI void        *edje_object_signal_callback_del_full(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);
   EAPI void         edje_object_signal_emit         (Evas_Object *obj, const char *emission, const char *source);
   EAPI void         edje_object_play_set            (Evas_Object *obj, Eina_Bool play);
   EAPI Eina_Bool    edje_object_play_get            (const Evas_Object *obj);
   EAPI void         edje_object_animation_set       (Evas_Object *obj, Eina_Bool on);
   EAPI Eina_Bool    edje_object_animation_get       (const Evas_Object *obj);

   /* edje_util.c */
   EAPI int          edje_object_freeze                  (Evas_Object *obj);
   EAPI int          edje_object_thaw                    (Evas_Object *obj);
   EAPI Eina_Bool    edje_object_color_class_set         (Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);
   EAPI Eina_Bool    edje_object_color_class_get         (const Evas_Object *o, const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);
   EAPI void         edje_object_color_class_del         (Evas_Object *obj, const char *color_class);
   EAPI Eina_Bool    edje_object_text_class_set          (Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size);
   EAPI void         edje_object_size_min_get            (const Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);
   EAPI void         edje_object_size_max_get            (const Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh);
   EAPI void         edje_object_calc_force              (Evas_Object *obj);
   EAPI void         edje_object_size_min_calc           (Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);
   EAPI Eina_Bool    edje_object_parts_extends_calc      (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
   EAPI void         edje_object_size_min_restricted_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh, Evas_Coord restrictedw, Evas_Coord restrictedh);
   EAPI Eina_Bool    edje_object_part_exists             (const Evas_Object *obj, const char *part);
   EAPI const Evas_Object *edje_object_part_object_get   (const Evas_Object *obj, const char *part);
   EAPI Eina_Bool    edje_object_part_geometry_get       (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
   
   EAPI void         edje_object_item_provider_set       (Evas_Object *obj, Edje_Item_Provider_Cb func, void *data);
   
   EAPI void         edje_object_text_change_cb_set      (Evas_Object *obj, Edje_Text_Change_Cb func, void *data);
   
   EAPI Eina_Bool    edje_object_part_text_set           (Evas_Object *obj, const char *part, const char *text);
   EAPI const char  *edje_object_part_text_get           (const Evas_Object *obj, const char *part);
   EAPI Eina_Bool    edje_object_part_text_unescaped_set (Evas_Object *obj, const char *part, const char *text_to_escape);
   EAPI char        *edje_object_part_text_unescaped_get (const Evas_Object *obj, const char *part);

   EAPI const char      *edje_object_part_text_selection_get           (const Evas_Object *obj, const char *part);
   EAPI void             edje_object_part_text_select_none             (const Evas_Object *obj, const char *part);
   EAPI void             edje_object_part_text_select_all              (const Evas_Object *obj, const char *part);
   EAPI void             edje_object_part_text_insert                  (Evas_Object *obj, const char *part, const char *text);
   EAPI const Eina_List *edje_object_part_text_anchor_list_get         (const Evas_Object *obj, const char *part);
   EAPI const Eina_List *edje_object_part_text_anchor_geometry_get     (const Evas_Object *obj, const char *part, const char *anchor);
   EAPI const Eina_List *edje_object_part_text_item_list_get           (const Evas_Object *obj, const char *part);
   EAPI Eina_Bool        edje_object_part_text_item_geometry_get       (const Evas_Object *obj, const char *part, const char *item, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);
   EAPI void             edje_object_part_text_cursor_geometry_get     (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
   EAPI void             edje_object_part_text_select_allow_set        (const Evas_Object *obj, const char *part, Eina_Bool allow);
   EAPI void             edje_object_part_text_select_abort            (const Evas_Object *obj, const char *part);
   EAPI void             edje_object_part_text_select_begin            (const Evas_Object *obj, const char *part);
   EAPI void             edje_object_part_text_select_extend           (const Evas_Object *obj, const char *part);
       
   EAPI Eina_Bool        edje_object_part_text_cursor_next                 (Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI Eina_Bool        edje_object_part_text_cursor_prev                 (Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI Eina_Bool        edje_object_part_text_cursor_up                   (Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI Eina_Bool        edje_object_part_text_cursor_down                 (Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI void             edje_object_part_text_cursor_begin_set            (Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI void             edje_object_part_text_cursor_end_set              (Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI void             edje_object_part_text_cursor_copy                 (Evas_Object *obj, const char *part, Edje_Cursor src, Edje_Cursor dst);
   EAPI void             edje_object_part_text_cursor_line_begin_set       (Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI void             edje_object_part_text_cursor_line_end_set         (Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI Eina_Bool        edje_object_part_text_cursor_coord_set            (Evas_Object *obj, const char *part, Edje_Cursor cur, Evas_Coord x, Evas_Coord y);
   EAPI Eina_Bool        edje_object_part_text_cursor_is_format_get        (const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI Eina_Bool        edje_object_part_text_cursor_is_visible_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI const char      *edje_object_part_text_cursor_content_get          (const Evas_Object *obj, const char *part, Edje_Cursor cur);
   EAPI void             edje_object_part_text_cursor_pos_set              (Evas_Object *obj, const char *part, Edje_Cursor cur, int pos);
   EAPI int              edje_object_part_text_cursor_pos_get              (const Evas_Object *obj, const char *part, Edje_Cursor cur);

   EAPI void             edje_object_text_insert_filter_callback_add       (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);
   EAPI void            *edje_object_text_insert_filter_callback_del       (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func);
   EAPI void            *edje_object_text_insert_filter_callback_del_full  (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);
   
   EAPI Eina_Bool        edje_object_part_swallow        (Evas_Object *obj, const char *part, Evas_Object *obj_swallow);
   EAPI void             edje_object_part_unswallow      (Evas_Object *obj, Evas_Object *obj_swallow);
   EAPI Evas_Object     *edje_object_part_swallow_get    (const Evas_Object *obj, const char *part);
   EAPI const char      *edje_object_part_state_get      (const Evas_Object *obj, const char *part, double *val_ret);
   EAPI Edje_Drag_Dir    edje_object_part_drag_dir_get   (const Evas_Object *obj, const char *part);
   EAPI Eina_Bool        edje_object_part_drag_value_set (Evas_Object *obj, const char *part, double dx, double dy);
   EAPI Eina_Bool        edje_object_part_drag_value_get (const Evas_Object *obj, const char *part, double *dx, double *dy);
   EAPI Eina_Bool        edje_object_part_drag_size_set  (Evas_Object *obj, const char *part, double dw, double dh);
   EAPI Eina_Bool        edje_object_part_drag_size_get  (const Evas_Object *obj, const char *part, double *dw, double *dh);
   EAPI Eina_Bool        edje_object_part_drag_step_set  (Evas_Object *obj, const char *part, double dx, double dy);
   EAPI Eina_Bool        edje_object_part_drag_step_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);
   EAPI Eina_Bool        edje_object_part_drag_page_set  (Evas_Object *obj, const char *part, double dx, double dy);
   EAPI Eina_Bool        edje_object_part_drag_page_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);
   EAPI Eina_Bool        edje_object_part_drag_step      (Evas_Object *obj, const char *part, double dx, double dy);
   EAPI Eina_Bool        edje_object_part_drag_page      (Evas_Object *obj, const char *part, double dx, double dy);

   EAPI Evas_Object              *edje_object_part_external_object_get     (const Evas_Object *obj, const char *part);
   EAPI Eina_Bool                 edje_object_part_external_param_set      (Evas_Object *obj, const char *part, const Edje_External_Param *param);
   EAPI Eina_Bool                 edje_object_part_external_param_get      (const Evas_Object *obj, const char *part, Edje_External_Param *param);
   EAPI Evas_Object              *edje_object_part_external_content_get    (const Evas_Object *obj, const char *part, const char *content);
   EAPI Edje_External_Param_Type  edje_object_part_external_param_type_get (const Evas_Object *obj, const char *part, const char *param);

   EAPI Eina_Bool    edje_object_part_box_append             (Evas_Object *obj, const char *part, Evas_Object *child);
   EAPI Eina_Bool    edje_object_part_box_prepend            (Evas_Object *obj, const char *part, Evas_Object *child);
   EAPI Eina_Bool    edje_object_part_box_insert_before      (Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference);
   EAPI Eina_Bool    edje_object_part_box_insert_at          (Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos);
   EAPI Evas_Object *edje_object_part_box_remove             (Evas_Object *obj, const char *part, Evas_Object *child);
   EAPI Evas_Object *edje_object_part_box_remove_at          (Evas_Object *obj, const char *part, unsigned int pos);
   EAPI Eina_Bool    edje_object_part_box_remove_all         (Evas_Object *obj, const char *part, Eina_Bool clear);
   EAPI Evas_Object *edje_object_part_table_child_get        (Evas_Object *obj, const char *part, unsigned int col, unsigned int row);
   EAPI Eina_Bool    edje_object_part_table_pack             (Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);
   EAPI Eina_Bool    edje_object_part_table_unpack           (Evas_Object *obj, const char *part, Evas_Object *child_obj);
   EAPI Eina_Bool    edje_object_part_table_col_row_size_get (const Evas_Object *obj, const char *part, int *cols, int *rows);
   EAPI Eina_Bool    edje_object_part_table_clear            (Evas_Object *obj, const char *part, Eina_Bool clear);

   /* edje_message_queue.c */
   EAPI void         edje_object_message_send                (Evas_Object *obj, Edje_Message_Type type, int id, void *msg);
   EAPI void         edje_object_message_handler_set         (Evas_Object *obj, Edje_Message_Handler_Cb func, void *data);
   EAPI void         edje_object_message_signal_process      (Evas_Object *obj);

   EAPI void         edje_message_signal_process             (void);

   /* edje_external.c */
   EAPI Eina_Bool    edje_external_type_register             (const char *type_name, const Edje_External_Type *type_info);
   EAPI Eina_Bool    edje_external_type_unregister           (const char *type_name);
   
   EAPI void         edje_external_type_array_register       (const Edje_External_Type_Info *array);
   EAPI void         edje_external_type_array_unregister     (const Edje_External_Type_Info *array);
   
   EAPI unsigned int edje_external_type_abi_version_get      (void) EINA_CONST;
   
   
   EAPI Eina_Iterator                  *edje_external_iterator_get     (void);
   EAPI Edje_External_Param            *edje_external_param_find       (const Eina_List *params, const char *key);
   EAPI Eina_Bool                       edje_external_param_int_get    (const Eina_List *params, const char *key, int *ret);
   EAPI Eina_Bool                       edje_external_param_double_get (const Eina_List *params, const char *key, double *ret);
   EAPI Eina_Bool                       edje_external_param_string_get (const Eina_List *params, const char *key, const char **ret);
   EAPI Eina_Bool                       edje_external_param_bool_get   (const Eina_List *params, const char *key, Eina_Bool *ret);
   EAPI Eina_Bool                       edje_external_param_choice_get (const Eina_List *params, const char *key, const char **ret);
   EAPI const Edje_External_Param_Info *edje_external_param_info_get   (const char *type_name);
   EAPI const Edje_External_Type       *edje_external_type_get         (const char *type_name);

   /* edje_module.c */
   EAPI Eina_Bool               edje_module_load                (const char *module);
   EAPI const Eina_List        *edje_available_modules_get      (void);

   /* perspective info for maps inside edje objects */
   typedef struct _Edje_Perspective Edje_Perspective;
   
   EAPI Edje_Perspective       *edje_perspective_new            (Evas *e);
   EAPI void                    edje_perspective_free           (Edje_Perspective *ps);
   EAPI void                    edje_perspective_set            (Edje_Perspective *ps, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc);
   EAPI void                    edje_perspective_global_set     (Edje_Perspective *ps, Eina_Bool global);
   EAPI Eina_Bool               edje_perspective_global_get     (const Edje_Perspective *ps);
   EAPI const Edje_Perspective *edje_evas_global_perspective_get(const Evas *e);
   EAPI void                    edje_object_perspective_set     (Evas_Object *obj, Edje_Perspective *ps);
   EAPI const Edje_Perspective *edje_object_perspective_get     (const Evas_Object *obj);
   
#ifdef __cplusplus
}
#endif

#endif
