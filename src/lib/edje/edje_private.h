#ifndef _EDJE_PRIVATE_H
#define _EDJE_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <locale.h>
#include <libgen.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

#include <fcntl.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <setjmp.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Eina.h>
#include <Eet.h>
#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Ecore_Input.h>
#ifdef HAVE_ECORE_IMF
# include <Ecore_IMF.h>
# include <Ecore_IMF_Evas.h>
#endif
#include <Embryo.h>

#ifdef HAVE_EIO
# include <Eio.h>
#endif

#ifdef HAVE_EPHYSICS
# include <EPhysics.h>
#endif

#include "Edje.h"

EAPI extern int _edje_default_log_dom ;

#ifdef EDJE_DEFAULT_LOG_COLOR
# undef EDJE_DEFAULT_LOG_COLOR
#endif
#define EDJE_DEFAULT_LOG_COLOR EINA_COLOR_CYAN
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_edje_default_log_dom, __VA_ARGS__)
#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_edje_default_log_dom, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_edje_default_log_dom, __VA_ARGS__)
#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_edje_default_log_dom, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_edje_default_log_dom, __VA_ARGS__)
#ifdef __GNUC__
# if __GNUC__ >= 4
// BROKEN in gcc 4 on amd64
//#  pragma GCC visibility push(hidden)
# endif
#endif

#ifndef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#ifndef CLAMP
#define CLAMP(x, min, max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif


#ifdef BUILD_EDJE_FP

#define FLOAT_T Eina_F32p32
#define EDJE_T_FLOAT EET_T_F32P32
#define MUL(a, b) eina_f32p32_mul(a, b)
#define SCALE(a, b) eina_f32p32_scale(a, b)
#define DIV(a, b) eina_f32p32_div(a, b)
#define DIV2(a) ((a) >> 1)
#define ADD(a, b) eina_f32p32_add(a, b)
#define SUB(a, b) eina_f32p32_sub(a, b)
#define SQRT(a) eina_f32p32_sqrt(a)
#define TO_DOUBLE(a) eina_f32p32_double_to(a)
#define FROM_DOUBLE(a) eina_f32p32_double_from(a)
#define FROM_INT(a) eina_f32p32_int_from(a)
#define TO_INT(a) eina_f32p32_int_to(a)
#define ZERO 0
#define COS(a) eina_f32p32_cos(a)
#define SIN(a) eina_f32p32_sin(a)
#define PI EINA_F32P32_PI

#else

#define FLOAT_T double
#define EDJE_T_FLOAT EET_T_DOUBLE
#define MUL(a, b) ((a) * (b))
#define SCALE(a, b) ((a) * (double)(b))
#define DIV(a, b) ((a) / (b))
#define DIV2(a) ((a) / 2.0)
#define ADD(a, b) ((a) + (b))
#define SUB(a, b) ((a) - (b))
#define SQRT(a) sqrt(a)
#define TO_DOUBLE(a) (double)(a)
#define FROM_DOUBLE(a) (a)
#define FROM_INT(a) (double)(a)
#define TO_INT(a) (int)(a)
#define ZERO 0.0
#define COS(a) cos(a)
#define SIN(a) sin(a)
#define PI 3.14159265358979323846

#endif

/* Inheritable Edje Smart API. For now private so only Edje Edit makes
 * use of this, but who knows what will be possible in the future */
#define EDJE_SMART_API_VERSION 1

/* Basic macro to init the Edje Smart API */
#define EDJE_SMART_API_INIT(smart_class_init) {smart_class_init, EDJE_SMART_API_VERSION, NULL}

#define EDJE_SMART_API_INIT_NULL EDJE_SMART_API_INIT(EVAS_SMART_CLASS_INIT_NULL)
#define EDJE_SMART_API_INIT_VERSION EDJE_SMART_API_INIT(EVAS_SMART_CLASS_INIT_VERSION)
#define EDJE_SMART_API_INIT_NAME_VERSION(name) EDJE_SMART_API_INIT(EVAS_SMART_CLASS_INIT_NAME_VERSION(name))

/* increment this when the EET data descriptors have changed and old
 * EETs cannot be loaded/used correctly anymore.
 */
#define EDJE_FILE_VERSION 3
/* increment this when you add new feature to edje file format without
 * breaking backward compatibility.
 */
#define EDJE_FILE_MINOR 5

/* FIXME:
 *
 * More example Edje files
 *
 * ? programs can do multiple actions from one signal
 * ? add containering (hbox, vbox, table, wrapping multi-line hbox & vbox)
 * ? text entry widget (single line only)
 *
 * ? recursions, unsafe callbacks outside Edje etc. with freeze, ref/unref and block/unblock and break_programs needs to be redesigned & fixed
 * ? all unsafe calls that may result in callbacks must be marked and dealt with
 */

typedef enum
{
   EDJE_ASPECT_PREFER_NONE,
   EDJE_ASPECT_PREFER_VERTICAL,
   EDJE_ASPECT_PREFER_HORIZONTAL,
   EDJE_ASPECT_PREFER_BOTH,
   EDJE_ASPECT_PREFER_SOURCE
} Edje_Internal_Aspect;

struct _Edje_Perspective
{
   Evas_Object *obj;
   Evas        *e;
   Evas_Coord   px, py, z0, foc;
   Eina_List   *users;
   Eina_Bool    global : 1;
};

struct _Edje_Position_Scale
{
   FLOAT_T x, y;
};

struct _Edje_Position
{
   int x, y;
};

struct _Edje_Size
{
   int w, h;
   Eina_Bool limit; /* should we limit ourself to the size of the source */
};

struct _Edje_Rectangle
{
   int x, y, w, h;
};

struct _Edje_Color
{
   unsigned char  r, g, b, a;
};

struct _Edje_Map_Color
{
   int idx;
   unsigned char r;
   unsigned char g;
   unsigned char b;
   unsigned char a;
};

struct _Edje_Aspect_Prefer
{
   FLOAT_T min, max;
   char prefer;
};

struct _Edje_Aspect
{
   int w, h;
   Edje_Aspect_Control mode;
};

struct _Edje_String
{
   const char *str;
   unsigned int id;
};

typedef struct _Edje_Position_Scale                  Edje_Alignment;
typedef struct _Edje_Position_Scale                  Edje_Position_Scale;
typedef struct _Edje_Position                        Edje_Position;
typedef struct _Edje_Size                            Edje_Size;
typedef struct _Edje_Rectangle                       Edje_Rectangle;
typedef struct _Edje_Color                           Edje_Color;
typedef struct _Edje_Map_Color                       Edje_Map_Color;
typedef struct _Edje_Aspect_Prefer                   Edje_Aspect_Prefer;
typedef struct _Edje_Aspect                          Edje_Aspect;
typedef struct _Edje_String                          Edje_String;

typedef struct _Edje_File                            Edje_File;
typedef struct _Edje_Style                           Edje_Style;
typedef struct _Edje_Style_Tag                       Edje_Style_Tag;
typedef struct _Edje_External_Directory              Edje_External_Directory;
typedef struct _Edje_External_Directory_Entry        Edje_External_Directory_Entry;
typedef struct _Edje_Font_Directory_Entry            Edje_Font_Directory_Entry;
typedef struct _Edje_Image_Directory                 Edje_Image_Directory;
typedef struct _Edje_Image_Directory_Entry           Edje_Image_Directory_Entry;
typedef struct _Edje_Image_Directory_Set             Edje_Image_Directory_Set;
typedef struct _Edje_Image_Directory_Set_Entry       Edje_Image_Directory_Set_Entry;
typedef struct _Edje_Limit                           Edje_Limit;
typedef struct _Edje_Sound_Sample                    Edje_Sound_Sample;
typedef struct _Edje_Sound_Tone                      Edje_Sound_Tone;
typedef struct _Edje_Sound_Directory                 Edje_Sound_Directory;
typedef struct _Edje_Program                         Edje_Program;
typedef struct _Edje_Program_Target                  Edje_Program_Target;
typedef struct _Edje_Program_After                   Edje_Program_After;
typedef struct _Edje_Part_Collection_Directory_Entry Edje_Part_Collection_Directory_Entry;
typedef struct _Edje_Pack_Element                    Edje_Pack_Element;
typedef struct _Edje_Part_Collection                 Edje_Part_Collection;
typedef struct _Edje_Part                            Edje_Part;
typedef struct _Edje_Part_Api                        Edje_Part_Api;
typedef struct _Edje_Part_Dragable		     Edje_Part_Dragable;
typedef struct _Edje_Part_Image_Id                   Edje_Part_Image_Id;
typedef struct _Edje_Part_Description_Image          Edje_Part_Description_Image;
typedef struct _Edje_Part_Description_Proxy          Edje_Part_Description_Proxy;
typedef struct _Edje_Part_Description_Text           Edje_Part_Description_Text;
typedef struct _Edje_Part_Description_Box            Edje_Part_Description_Box;
typedef struct _Edje_Part_Description_Table          Edje_Part_Description_Table;
typedef struct _Edje_Part_Description_External       Edje_Part_Description_External;
typedef struct _Edje_Part_Description_Common         Edje_Part_Description_Common;
typedef struct _Edje_Part_Description_Spec_Fill      Edje_Part_Description_Spec_Fill;
typedef struct _Edje_Part_Description_Spec_Border    Edje_Part_Description_Spec_Border;
typedef struct _Edje_Part_Description_Spec_Image     Edje_Part_Description_Spec_Image;
typedef struct _Edje_Part_Description_Spec_Proxy     Edje_Part_Description_Spec_Proxy;
typedef struct _Edje_Part_Description_Spec_Text      Edje_Part_Description_Spec_Text;
typedef struct _Edje_Part_Description_Spec_Box       Edje_Part_Description_Spec_Box;
typedef struct _Edje_Part_Description_Spec_Table     Edje_Part_Description_Spec_Table;
typedef struct _Edje_Physics_Face                    Edje_Physics_Face;
typedef struct _Edje_Patterns                        Edje_Patterns;
typedef struct _Edje_Part_Box_Animation              Edje_Part_Box_Animation;
typedef struct _Edje_Part_Limit                      Edje_Part_Limit;

typedef struct _Edje Edje;
typedef struct _Edje_Real_Part_Text Edje_Real_Part_Text;
typedef struct _Edje_Real_Part_Swallow Edje_Real_Part_Swallow;
typedef struct _Edje_Real_Part_Container Edje_Real_Part_Container;
typedef struct _Edje_Real_Part_State Edje_Real_Part_State;
typedef struct _Edje_Real_Part_Drag Edje_Real_Part_Drag;
typedef struct _Edje_Real_Part_Set Edje_Real_Part_Set;
typedef struct _Edje_Real_Part Edje_Real_Part;
typedef struct _Edje_Running_Program Edje_Running_Program;
typedef struct _Edje_Signal_Callback Edje_Signal_Callback;
typedef struct _Edje_Calc_Params Edje_Calc_Params;
typedef struct _Edje_Calc_Params_Map Edje_Calc_Params_Map;
typedef struct _Edje_Calc_Params_Physics Edje_Calc_Params_Physics;
typedef struct _Edje_Pending_Program Edje_Pending_Program;
typedef struct _Edje_Text_Style Edje_Text_Style;
typedef struct _Edje_Color_Class Edje_Color_Class;
typedef struct _Edje_Text_Class Edje_Text_Class;
typedef struct _Edje_Var Edje_Var;
typedef struct _Edje_Var_Int Edje_Var_Int;
typedef struct _Edje_Var_Float Edje_Var_Float;
typedef struct _Edje_Var_String Edje_Var_String;
typedef struct _Edje_Var_List Edje_Var_List;
typedef struct _Edje_Var_Hash Edje_Var_Hash;
typedef struct _Edje_Var_Animator Edje_Var_Animator;
typedef struct _Edje_Var_Timer Edje_Var_Timer;
typedef struct _Edje_Var_Pool Edje_Var_Pool;
typedef struct _Edje_Signal_Source_Char Edje_Signal_Source_Char;
typedef struct _Edje_Text_Insert_Filter_Callback Edje_Text_Insert_Filter_Callback;
typedef struct _Edje_Markup_Filter_Callback Edje_Markup_Filter_Callback;
typedef struct _Edje_Signals_Sources_Patterns Edje_Signals_Sources_Patterns;
typedef struct _Edje_Signal_Callback_Flags Edje_Signal_Callback_Flags;
typedef struct _Edje_Signal_Callback_Group Edje_Signal_Callback_Group;
typedef struct _Edje_Signal_Callback_Match Edje_Signal_Callback_Match;
typedef struct _Edje_Signal_Callback_Matches Edje_Signal_Callback_Matches;
typedef struct _Edje_Signal_Callback_Custom Edje_Signal_Callback_Custom;

#define EDJE_INF_MAX_W 100000
#define EDJE_INF_MAX_H 100000

#define EDJE_IMAGE_SOURCE_TYPE_NONE           0
#define EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT 1
#define EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY   2
#define EDJE_IMAGE_SOURCE_TYPE_EXTERNAL       3
#define EDJE_IMAGE_SOURCE_TYPE_LAST           4

#define EDJE_SOUND_SOURCE_TYPE_NONE           0
#define EDJE_SOUND_SOURCE_TYPE_INLINE_RAW     1
#define EDJE_SOUND_SOURCE_TYPE_INLINE_COMP    2
#define EDJE_SOUND_SOURCE_TYPE_INLINE_LOSSY   3
#define EDJE_SOUND_SOURCE_TYPE_INLINE_AS_IS   4

#define EDJE_VAR_NONE   0
#define EDJE_VAR_INT    1
#define EDJE_VAR_FLOAT  2
#define EDJE_VAR_STRING 3
#define EDJE_VAR_LIST   4
#define EDJE_VAR_HASH   5

#define EDJE_VAR_MAGIC_BASE 0x12fe84ba

#define EDJE_STATE_PARAM_NONE            0
#define EDJE_STATE_PARAM_ALIGNMENT       1
#define EDJE_STATE_PARAM_MIN             2
#define EDJE_STATE_PARAM_MAX             3
#define EDJE_STATE_PARAM_STEP            4
#define EDJE_STATE_PARAM_ASPECT          5
#define EDJE_STATE_PARAM_ASPECT_PREF     6
#define EDJE_STATE_PARAM_COLOR           7
#define EDJE_STATE_PARAM_COLOR2          8
#define EDJE_STATE_PARAM_COLOR3          9
#define EDJE_STATE_PARAM_COLOR_CLASS    10
#define EDJE_STATE_PARAM_REL1           11
#define EDJE_STATE_PARAM_REL1_TO        12
#define EDJE_STATE_PARAM_REL1_OFFSET    13
#define EDJE_STATE_PARAM_REL2           14
#define EDJE_STATE_PARAM_REL2_TO        15
#define EDJE_STATE_PARAM_REL2_OFFSET    16
#define EDJE_STATE_PARAM_IMAGE          17
#define EDJE_STATE_PARAM_BORDER         18
#define EDJE_STATE_PARAM_FILL_SMOOTH    19
#define EDJE_STATE_PARAM_FILL_POS       20
#define EDJE_STATE_PARAM_FILL_SIZE      21
#define EDJE_STATE_PARAM_TEXT           22
#define EDJE_STATE_PARAM_TEXT_CLASS     23
#define EDJE_STATE_PARAM_TEXT_FONT      24
#define EDJE_STATE_PARAM_TEXT_STYLE     25
#define EDJE_STATE_PARAM_TEXT_SIZE      26
#define EDJE_STATE_PARAM_TEXT_FIT       27
#define EDJE_STATE_PARAM_TEXT_MIN       28
#define EDJE_STATE_PARAM_TEXT_MAX       29
#define EDJE_STATE_PARAM_TEXT_ALIGN     30
#define EDJE_STATE_PARAM_VISIBLE        31
#define EDJE_STATE_PARAM_MAP_ON         32
#define EDJE_STATE_PARAM_MAP_PERSP      33
#define EDJE_STATE_PARAM_MAP_LIGHT      34
#define EDJE_STATE_PARAM_MAP_ROT_CENTER 35
#define EDJE_STATE_PARAM_MAP_ROT_X      36
#define EDJE_STATE_PARAM_MAP_ROT_Y      37
#define EDJE_STATE_PARAM_MAP_ROT_Z      38
#define EDJE_STATE_PARAM_MAP_BACK_CULL  39
#define EDJE_STATE_PARAM_MAP_PERSP_ON   40
#define EDJE_STATE_PARAM_PERSP_ZPLANE   41
#define EDJE_STATE_PARAM_PERSP_FOCAL    42
#define EDJE_STATE_PARAM_PHYSICS_MASS   43
#define EDJE_STATE_PARAM_PHYSICS_FRICTION 44
#define EDJE_STATE_PARAM_PHYSICS_RESTITUTION 45
#define EDJE_STATE_PARAM_PHYSICS_DAMPING 46
#define EDJE_STATE_PARAM_PHYSICS_SLEEP  47
#define EDJE_STATE_PARAM_PHYSICS_MATERIAL 48
#define EDJE_STATE_PARAM_PHYSICS_DENSITY 49
#define EDJE_STATE_PARAM_PHYSICS_IGNORE_PART_POS 50
#define EDJE_STATE_PARAM_PHYSICS_LIGHT_ON 51
#define EDJE_STATE_PARAM_PHYSICS_HARDNESS 52
#define EDJE_STATE_PARAM_PHYSICS_MOV_FREEDOM_LIN 53
#define EDJE_STATE_PARAM_PHYSICS_MOV_FREEDOM_ANG 54
#define EDJE_STATE_PARAM_PHYSICS_BACK_CULL 55
#define EDJE_STATE_PARAM_PHYSICS_Z      56
#define EDJE_STATE_PARAM_PHYSICS_DEPTH  57
#define EDJE_STATE_PARAM_PROXY_SRC_VISIBLE 58
#define EDJE_STATE_PARAM_PROXY_SRC_CLIP 59
#define EDJE_STATE_PARAM_LAST           60

#define EDJE_ENTRY_EDIT_MODE_NONE 0
#define EDJE_ENTRY_EDIT_MODE_SELECTABLE 1
#define EDJE_ENTRY_EDIT_MODE_EDITABLE 2
#define EDJE_ENTRY_EDIT_MODE_PASSWORD 3

#define EDJE_ENTRY_SELECTION_MODE_DEFAULT 0
#define EDJE_ENTRY_SELECTION_MODE_EXPLICIT 1

#define EDJE_ENTRY_CURSOR_MODE_UNDER 0
#define EDJE_ENTRY_CURSOR_MODE_BEFORE 1

#define EDJE_ORIENTATION_AUTO  0
#define EDJE_ORIENTATION_LTR   1
#define EDJE_ORIENTATION_RTL   2

#define EDJE_PART_PATH_SEPARATOR ':'
#define EDJE_PART_PATH_SEPARATOR_STRING ":"
#define EDJE_PART_PATH_SEPARATOR_INDEXL '['
#define EDJE_PART_PATH_SEPARATOR_INDEXR ']'

#define FLAG_NONE 0
#define FLAG_X    0x01
#define FLAG_Y    0x02
#define FLAG_XY   (FLAG_X | FLAG_Y)

/*----------*/

struct _Edje_File
{
   const char                     *path;
   time_t                          mtime;

   Edje_External_Directory        *external_dir;
   Edje_Image_Directory           *image_dir;
   Edje_Sound_Directory           *sound_dir;

   Eina_List                      *styles;

   Eina_List                      *color_classes;
   Eina_Hash                      *color_hash;

   int                             references;
   const char                     *compiler;
   int                             version;
   int				   minor;
   int                             feature_ver;

   Eina_Hash                      *data;
   Eina_Hash			  *fonts;

   Eina_Hash			  *collection;
   Eina_List			  *collection_cache;

   Edje_Patterns		  *collection_patterns;

   Eet_File                       *ef;
   Eina_File                      *f;

#ifdef HAVE_EIO
   Eio_Monitor                    *monitor;
   Eina_List                      *edjes;
   Eina_List                      *handlers;
   Ecore_Timer                    *timeout;
#endif

   unsigned char                   free_strings : 1;
   unsigned char                   dangling : 1;
   unsigned char		   warning : 1;
};

struct _Edje_Style
{
   char                           *name;
   Eina_List                      *tags;
   Evas_Textblock_Style           *style;

   Eina_Bool                       cache;
};

struct _Edje_Style_Tag
{
   const char                     *key;
   const char                     *value;
   const char			  *font;
   double			   font_size;
   const char			  *text_class;
};

/*----------*/


struct _Edje_Font_Directory_Entry
{
   const char *entry; /* the name of the font */
   const char *file; /* the name of the file */
};

/*----------*/

struct _Edje_External_Directory
{
   Edje_External_Directory_Entry *entries; /* a list of Edje_External_Directory_Entry */
   unsigned int entries_count;
};

struct _Edje_External_Directory_Entry
{
   const char *entry; /* the name of the external */
};


/*----------*/



/*----------*/

struct _Edje_Image_Directory
{
   Edje_Image_Directory_Entry *entries; /* an array of Edje_Image_Directory_Entry */
   unsigned int entries_count;

   Edje_Image_Directory_Set *sets; /* an array of Edje_Image_Directory_Set */
   unsigned int sets_count;
};

struct _Edje_Image_Directory_Entry
{
   const char *entry; /* the nominal name of the image - if any */
   int   source_type; /* alternate source mode. 0 = none */
   int   source_param; /* extra params on encoding */
   int   id; /* the id no. of the image */
};

struct _Edje_Image_Directory_Set
{
   char *name;
   Eina_List *entries;

   int id;
};

struct _Edje_Image_Directory_Set_Entry
{
   const char *name;
   int id;

   struct {
     struct {
       int w;
       int h;
     } min, max;
   } size;

   struct {
      int l, r, t, b;
      FLOAT_T scale_by;
   } border;
};

struct _Edje_Sound_Sample /*Sound Sample*/
{
   const char *name; /* the nominal name of the sound */
   const char *snd_src;  /* Sound source Wav file */
   int   compression;  /* Compression - RAW, LOSSLESS COMP ,  LOSSY ) */
   int   mode; /* alternate source mode. 0 = none */
   double quality;
   int   id; /* the id no. of the sound */
};

struct _Edje_Sound_Tone /*Sound Sample*/
{
   const char *name; /* the nominal name of the sound - if any */
   int   value; /* alternate source mode. 0 = none */
   int   id; /* the id no. of the sound */
};

struct _Edje_Sound_Directory
{

   Edje_Sound_Sample *samples;  /* an array of Edje_Sound_Sample entries */
   unsigned int samples_count;

   Edje_Sound_Tone *tones;  /* an array of Edje_Sound_Tone entries */
   unsigned int tones_count;
};

/*----------*/

struct _Edje_Program /* a conditional program to be run */
{
   int         id; /* id of program */
   const char *name; /* name of the action */

   const char *signal; /* if signal emission name matches the glob here... */
   const char *source; /* if part that emitted this (name) matches this glob */
   const char *sample_name;
   const char *tone_name;
   double duration;
   double speed;

   struct {
      const char *part;
      const char *state; /* if state is not set, we will try with source */
   } filter; /* the part filter.part should be in state filter.state for signal to be accepted */

   struct {
      double   from;
      double   range;
   } in;

   int         action; /* type - set state, stop action, set drag pos etc. */
   const char *state; /* what state of alternates to apply, NULL = default */
   const char *state2; /* what other state to use - for signal emit action */
   double      value; /* value of state to apply (if multiple names match) */
   double      value2; /* other value for drag actions */

   struct {
      int      mode; /* how to tween - linear, sinusoidal etc. */
      FLOAT_T  time; /* time to graduate between current and new state */
      FLOAT_T  v1; /* other value for drag actions */
      FLOAT_T  v2; /* other value for drag actions */
   } tween;

   Eina_List  *targets; /* list of target parts to apply the state to */

   Eina_List  *after; /* list of actions to run at the end of this, for looping */

   struct {
      const char *name;
      const char *description;
   } api;

   /* used for PARAM_COPY (param names in state and state2 above!) */
   struct {
      int src; /* part where parameter is being retrieved */
      int dst; /* part where parameter is being stored */
   } param;

#ifdef HAVE_EPHYSICS
   /* used for actions related to physics, as physics impulses and forces */
   struct {
      double w;
      double x;
      double y;
      double z;
   } physics;
#endif

   Eina_Bool exec : 1;
};

struct _Edje_Program_Target /* the target of an action */
{
   int id; /* just the part id no, or action id no */
};

struct _Edje_Program_After /* the action to run after another action */
{
   int id;
};

/*----------*/
struct _Edje_Limit
{
   const char *name;
   int value;
};

/*----------*/
#define PART_TYPE_FIELDS(TYPE)    \
      TYPE      RECTANGLE;        \
      TYPE      TEXT;             \
      TYPE      IMAGE;            \
      TYPE      PROXY;            \
      TYPE      SWALLOW;          \
      TYPE      TEXTBLOCK;        \
      TYPE      GROUP;            \
      TYPE      BOX;              \
      TYPE      TABLE;            \
      TYPE      SPACER;	  \
      TYPE      EXTERNAL;

struct _Edje_Part_Collection_Directory_Entry
{
   const char *entry; /* the nominal name of the part collection */
   int         id; /* the id of this named part collection */

   struct
   {
      PART_TYPE_FIELDS(int)
      int      part;
   } count;

   struct
   {
      PART_TYPE_FIELDS(Eina_Mempool *)
      Eina_Mempool *part;
   } mp;

   struct
   {
      PART_TYPE_FIELDS(Eina_Mempool *)
   } mp_rtl; /* For Right To Left interface */

   Edje_Part_Collection *ref;
};

/*----------*/

/*----------*/

struct _Edje_Pack_Element
{
   unsigned char    type; /* only GROUP supported for now */
   Edje_Real_Part  *parent; /* pointer to the table/box that hold it, set at runtime */
   const char      *name; /* if != NULL, will be set with evas_object_name_set */
   const char      *source; /* group name to use as source for this element */
   Edje_Size        min, prefer, max;
   struct {
      int l, r, t, b;
   } padding;
   struct {
      int w, h;
   } spread;
   Edje_Alignment   align;
   Edje_Alignment   weight;
   Edje_Aspect      aspect;
   const char      *options; /* extra options for custom objects */
   /* table specific follows */
   int              col, row;
   unsigned short   colspan, rowspan;
};

struct _Edje_Physics_Face
{
   unsigned char    type; /* BOX_FRONT BOX_BACK BOX_RIGHT CYLINDER_BACK ... */
   const char      *source; /* group name to use as source for this element */
};

typedef enum {
  EDJE_PART_LIMIT_UNKNOWN = 0,
  EDJE_PART_LIMIT_BELOW,
  EDJE_PART_LIMIT_ZERO,
  EDJE_PART_LIMIT_OVER
} Edje_Part_Limit_State;

#ifdef HAVE_EPHYSICS
typedef enum {
  EDJE_PART_PHYSICS_BODY_NONE= 0,
  EDJE_PART_PHYSICS_BODY_RIGID_BOX,
  EDJE_PART_PHYSICS_BODY_RIGID_SPHERE,
  EDJE_PART_PHYSICS_BODY_RIGID_CYLINDER,
  EDJE_PART_PHYSICS_BODY_SOFT_BOX,
  EDJE_PART_PHYSICS_BODY_SOFT_SPHERE,
  EDJE_PART_PHYSICS_BODY_SOFT_CYLINDER,
  EDJE_PART_PHYSICS_BODY_CLOTH,
  EDJE_PART_PHYSICS_BODY_BOUNDARY_TOP,
  EDJE_PART_PHYSICS_BODY_BOUNDARY_BOTTOM,
  EDJE_PART_PHYSICS_BODY_BOUNDARY_RIGHT,
  EDJE_PART_PHYSICS_BODY_BOUNDARY_LEFT,
  EDJE_PART_PHYSICS_BODY_BOUNDARY_FRONT,
  EDJE_PART_PHYSICS_BODY_BOUNDARY_BACK
} Edje_Part_Physics_Body;
#endif

struct _Edje_Part_Limit
{
   int part;

   Edje_Part_Limit_State width; /* -1, 0 or 1 */
   Edje_Part_Limit_State height; /* -1, 0, or 1 */
};

struct _Edje_Signals_Sources_Patterns
{
   EINA_REFCOUNT;
   Edje_Patterns *signals_patterns;
   Edje_Patterns *sources_patterns;

   Eina_Rbtree   *exact_match;

   union {
      struct {
	 Edje_Program **globing;
	 unsigned int  count;
      } programs;
      struct {
         Eina_Inarray   globing;
      } callbacks;
   } u;
};

struct _Edje_Signal_Callback_Match
{
   const char     *signal;
   const char     *source;
   Edje_Signal_Cb  func;
};

struct _Edje_Signal_Callback_Matches
{
   Edje_Signal_Callback_Match *matches;

   Edje_Signals_Sources_Patterns *patterns;

   unsigned int matches_count;
   EINA_REFCOUNT;

   Eina_Bool hashed : 1;
};

struct _Edje_Signal_Callback_Flags
{
   Eina_Bool delete_me:1;
   Eina_Bool just_added:1;
   Eina_Bool propagate:1;
};

struct _Edje_Signal_Callback_Group
{
   const Edje_Signal_Callback_Matches *matches;

   void **custom_data;

   Edje_Signal_Callback_Flags *flags;
};

/*----------*/

struct _Edje_Part_Collection
{
   struct { /* list of Edje_Program */
      Edje_Program **fnmatch; /* complex match with "*?[\" */
      unsigned int fnmatch_count;

      Edje_Program **strcmp; /* No special caractere, plain strcmp does the work */
      unsigned int strcmp_count;

      Edje_Program **strncmp; /* Finish by * or ?, plain strncmp does the work */
      unsigned int strncmp_count;

      Edje_Program **strrncmp; /* Start with * or ?, reverse strncmp will do the job */
      unsigned int strrncmp_count;

      Edje_Program **nocmp; /* Empty signal/source that will never match */
      unsigned int nocmp_count;
   } programs;

   struct { /* list of limit that need to be monitored */
      Edje_Limit **vertical;
      unsigned int vertical_count;

      Edje_Limit **horizontal;
      unsigned int horizontal_count;

      Edje_Part_Limit *parts;
      unsigned int parts_count;
   } limits;

   Edje_Part **parts; /* an array of Edje_Part */
   unsigned int parts_count;

   Eina_Hash *data;

   int        id; /* the collection id */

   Eina_Hash *alias; /* aliasing part */
   Eina_Hash *aliased; /* invert match of alias */

   struct {
      Edje_Size min, max;
      unsigned char orientation;
   } prop;

   int        references;

#ifdef EDJE_PROGRAM_CACHE
   struct {
      Eina_Hash                   *no_matches;
      Eina_Hash                   *matches;
   } prog_cache;
#endif

#ifdef HAVE_EPHYSICS
   struct {
      struct {
         double rate;
         Evas_Coord z;
         Evas_Coord depth;
         struct {
              Evas_Coord x;
              Evas_Coord y;
              Evas_Coord z;
         } gravity;
      } world;
   } physics;
#endif

   Embryo_Program   *script; /* all the embryo script code for this group */
   const char       *part;

   /* *** generated at runtime *** */
   struct {
      Edje_Signals_Sources_Patterns programs;

      Edje_Program **table_programs;
      int            table_programs_size;
   } patterns;
   /* *** *** */

   unsigned char    script_only;

   unsigned char    lua_script_only;

   unsigned char    broadcast_signal;

   unsigned char    physics_enabled; /* will be 1 if a body is declared */

   unsigned char    checked : 1;
};

struct _Edje_Part_Dragable
{
   int                 step_x; /* drag jumps n pixels (0 = no limit) */
   int                 step_y; /* drag jumps n pixels (0 = no limit) */

   int                 count_x; /* drag area divided by n (0 = no limit) */
   int                 count_y; /* drag area divided by n (0 = no limit) */

   int                 confine_id; /* dragging within this bit, -1 = no */
   int                 threshold_id; /* dragging outside this bit, -1 = no */

   /* davinchi */
   int		  event_id; /* If it is used as scrollbar */

   signed char         x; /* can u click & drag this bit in x dir */
   signed char         y; /* can u click & drag this bit in y dir */
};

struct _Edje_Part_Api
{
   const char         *name;
   const char         *description;
};

typedef struct _Edje_Part_Description_List Edje_Part_Description_List;
struct _Edje_Part_Description_List
{
   Edje_Part_Description_Common **desc;
   Edje_Part_Description_Common **desc_rtl; /* desc for Right To Left interface */
   unsigned int desc_count;
};

struct _Edje_Part
{
   const char                   *name; /* the name if any of the part */
   Edje_Part_Description_Common *default_desc; /* the part descriptor for default */
   Edje_Part_Description_Common *default_desc_rtl; /* default desc for Right To Left interface */

   Edje_Part_Description_List    other; /* other possible descriptors */

   const char           *source, *source2, *source3, *source4, *source5, *source6;
   int                    id; /* its id number */
   int                    clip_to_id; /* the part id to clip this one to */
   Edje_Part_Dragable     dragable;
   Edje_Pack_Element    **items; /* packed items for box and table */
   unsigned int           items_count;
   unsigned char          type; /* what type (image, rect, text) */
#ifdef HAVE_EPHYSICS
   unsigned char          physics_body; /* body (none, rigid box, soft circle, ...) */
#endif
   unsigned char          effect; /* 0 = plain... */
   unsigned char          mouse_events; /* it will affect/respond to mouse events */
   unsigned char          repeat_events; /* it will repeat events to objects below */
   Evas_Event_Flags       ignore_flags;
   unsigned char          scale; /* should certain properties scale with edje scale factor? */
   unsigned char          precise_is_inside;
   unsigned char          use_alternate_font_metrics;
   unsigned char          pointer_mode;
   unsigned char          entry_mode;
   unsigned char          select_mode;
   unsigned char          cursor_mode;
   unsigned char          multiline;
   unsigned char          access; /* it will be used accessibility feature */
   Edje_Part_Api          api;
   unsigned char          nested_children_count;
};

struct _Edje_Part_Image_Id
{
   int id;
   Eina_Bool set;
};

struct _Edje_Part_Description_Common
{
   struct {
      double         value; /* the value of the state (for ranges) */
      const char    *name; /* the named state if any */
   } state;

   Edje_Alignment align; /* 0 <-> 1.0 alignment within allocated space */

   struct {
      unsigned char  w, h; /* width or height is fixed in side (cannot expand with Edje object size) */
   } fixed;

   struct { // only during recalc
      unsigned char have;
      FLOAT_T w, h;
   } minmul;

   Edje_Size min, max;
   Edje_Position step; /* size stepping by n pixels, 0 = none */
   Edje_Aspect_Prefer aspect;

   char      *color_class; /* how to modify the color */
   Edje_Color color;
   Edje_Color color2;

   struct {
      FLOAT_T        relative_x;
      FLOAT_T        relative_y;
      int            offset_x;
      int            offset_y;
      int            id_x; /* -1 = whole part collection, or part ID */
      int            id_y; /* -1 = whole part collection, or part ID */
   } rel1, rel2;

   struct {
      int id_persp;
      int id_light;
      struct {
         int id_center;
         FLOAT_T x, y, z;
      } rot;
      Edje_Map_Color **colors;    /* List of the Edje_Map_Color */
      unsigned int colors_count;
      Eina_Bool backcull;
      Eina_Bool on;
      Eina_Bool persp_on;
      Eina_Bool smooth;
      Eina_Bool alpha;
   } map;

   struct {
      int zplane;
      int focal;
   } persp;

#ifdef HAVE_EPHYSICS
   struct {
      double mass;
      double restitution;
      double friction;
      double density;
      double hardness;
      struct {
         double linear;
         double angular;
      } damping;
      struct {
         double linear;
         double angular;
      } sleep;
      int z;
      int depth;
      Eina_List *faces; /* faces for 3d effects */
      struct {
         struct {
            unsigned char x;
            unsigned char y;
            unsigned char z;
         } lin;
         struct {
            unsigned char x;
            unsigned char y;
            unsigned char z;
         } ang;
      } mov_freedom;
      unsigned char backcull;
      unsigned char material; /* (custom, iron, wood, ...) */
      unsigned char light_on;
      unsigned char ignore_part_pos;
   } physics;
#endif

   unsigned char     visible; /* is it shown */
   unsigned char     limit; /* 0 == no, 1 = width, 2 = height, 3 = both */
};

struct _Edje_Part_Description_Spec_Fill
{
   FLOAT_T        pos_rel_x; /* fill offset x relative to area */
   FLOAT_T        rel_x; /* relative size compared to area */
   FLOAT_T        pos_rel_y; /* fill offset y relative to area */
   FLOAT_T        rel_y; /* relative size compared to area */
   int            pos_abs_x; /* fill offset x added to fill offset */
   int            abs_x; /* size of fill added to relative fill */
   int            pos_abs_y; /* fill offset y added to fill offset */
   int            abs_y; /* size of fill added to relative fill */
   int            angle; /* angle of fill -- currently only used by grads */
   int            spread; /* spread of fill -- currently only used by grads */
   char           smooth; /* fill with smooth scaling or not */
   unsigned char  type; /* fill coordinate from container (SCALE) or from source image (TILE) */
};

struct _Edje_Part_Description_Spec_Border
{
   int            l, r, t, b; /* border scaling on image fill */
   unsigned char  no_fill; /* do we fill the center of the image if bordered? 1 == NO!!!! */
   unsigned char  scale; /* scale image border by same as scale factor */
   FLOAT_T        scale_by; /* when border scale above is enabled, border width OUTPUT is scaled by the object or global scale factor. this value adds another multiplier that the global scale is multiplued by first. if <= 0.0 it is not used, and if 1.0 it i s "ineffective" */
};

struct _Edje_Part_Description_Spec_Image
{
   Edje_Part_Description_Spec_Fill   fill;

   Edje_Part_Image_Id **tweens; /* list of Edje_Part_Image_Id */
   unsigned int         tweens_count; /* number of tweens */

   int            id; /* the image id to use */
   int            scale_hint; /* evas scale hint */
   Eina_Bool      set; /* if image condition it's content */

   Edje_Part_Description_Spec_Border border;
};

struct _Edje_Part_Description_Spec_Proxy
{
   Edje_Part_Description_Spec_Fill   fill;

   int id; /* the part id to use as a source for this state */
   Eina_Bool source_visible;  /* source object visibility */
   Eina_Bool source_clip;     /* source object clip */
};

struct _Edje_Part_Description_Spec_Text
{
   Edje_String    text; /* if "" or NULL, then leave text unchanged */
   char          *text_class; /* how to apply/modify the font */
   Edje_String    style; /* the text style if a textblock */
   Edje_String    font; /* if a specific font is asked for */
   Edje_String    repch; /* replacement char for password mode entry */

   Edje_Alignment align; /* text alignment within bounds */
   Edje_Color     color3;

   double         elipsis; /* 0.0 - 1.0 defining where the elipsis align */
   int            size; /* 0 = use user set size */
   int            id_source; /* -1 if none */
   int            id_text_source; /* -1 if none */

   unsigned char  fit_x; /* resize font size down to fit in x dir */
   unsigned char  fit_y; /* resize font size down to fit in y dir */
   unsigned char  min_x; /* if text size should be part min size */
   unsigned char  min_y; /* if text size should be part min size */
   unsigned char  max_x; /* if text size should be part max size */
   unsigned char  max_y; /* if text size should be part max size */
   int            size_range_min;
   int            size_range_max; /* -1 means, no bound. */
};

struct _Edje_Part_Description_Spec_Box
{
   char          *layout, *alt_layout;
   Edje_Alignment align;
   struct {
      int x, y;
   } padding;
   struct {
      unsigned char h, v;
   } min;
};

struct _Edje_Part_Description_Spec_Table
{
   unsigned char  homogeneous;
   Edje_Alignment align;
   struct {
      int x, y;
   } padding;
   struct {
      unsigned char h, v;
   } min;
};

struct _Edje_Part_Description_Image
{
   Edje_Part_Description_Common common;
   Edje_Part_Description_Spec_Image image;
};

struct _Edje_Part_Description_Proxy
{
   Edje_Part_Description_Common common;
   Edje_Part_Description_Spec_Proxy proxy;
};

struct _Edje_Part_Description_Text
{
   Edje_Part_Description_Common common;
   Edje_Part_Description_Spec_Text text;
};

struct _Edje_Part_Description_Box
{
   Edje_Part_Description_Common common;
   Edje_Part_Description_Spec_Box box;
};

struct _Edje_Part_Description_Table
{
   Edje_Part_Description_Common common;
   Edje_Part_Description_Spec_Table table;
};

struct _Edje_Part_Description_External
{
   Edje_Part_Description_Common common;
   Eina_List *external_params; /* parameters for external objects */
};

/*----------*/

struct _Edje_Signal_Source_Char
{
   EINA_RBTREE;

   const char *signal;
   const char *source;

   Eina_Inarray  list;
};

struct _Edje
{
   Evas_Object_Smart_Clipped_Data *base;
   /* This contains (or should):
	Evas_Object          *clipper; // a big rect to clip this Edje to
	Evas                 *evas; // the Evas this Edje belongs to
   */
   const char           *path;
   const char           *group;
   const char           *parent;

   Evas_Object          *obj; /* the smart object */
   Edje_File            *file; /* the file the data comes form */
   Edje_Part_Collection *collection; /* the description being used */
   Eina_List            *actions; /* currently running actions */
   Eina_List            *pending_actions;
   Eina_Hash            *color_classes;
   Eina_List            *text_classes;
   /* variable pool for Edje Embryo scripts */
   Edje_Var_Pool        *var_pool;
   /* for faster lookups to avoid nth list walks */
   Edje_Real_Part      **table_parts;
   Edje_Real_Part       *focused_part;
   Eina_List            *subobjs;
   Eina_List            *text_insert_filter_callbacks;
   Eina_List            *markup_filter_callbacks;
   void                 *script_only_data;

   Eina_List            *groups;

   struct {
      Eina_Hash         *text_class;
      Eina_Hash         *color_class;
   } members;

   Edje_Perspective     *persp;

   const Edje_Signal_Callback_Group *callbacks;

   struct {
      Edje_Text_Change_Cb  func;
      void                *data;
   } text_change;

   struct {
      Edje_Message_Handler_Cb  func;
      void                    *data;
      int                      num;
   } message;

   struct {
      Edje_Item_Provider_Cb  func;
      void                  *data;
   } item_provider;

#ifdef HAVE_EPHYSICS
   EPhysics_World       *world;
#endif

   Eina_List            *user_defined;

   lua_State            *L;
   Eina_Inlist          *lua_objs;
   int                   lua_ref;

   int                   processing_messages;
   int                   state;
   int			 preload_count;

   unsigned int          table_parts_size;

   int                   walking_callbacks;

   int                   references;
   int                   block;
   int                   load_error;
   int                   freeze;

   Evas_Coord            x, y, w, h;
   Edje_Size             min;
   double                paused_at;
   FLOAT_T		 scale;

   Eina_Bool          is_rtl : 1;
   Eina_Bool          dirty : 1;
   Eina_Bool          recalc : 1;
   Eina_Bool          delete_callbacks : 1;
   Eina_Bool          just_added_callbacks : 1;
   Eina_Bool          have_objects : 1;
   Eina_Bool          paused : 1;
   Eina_Bool          no_anim : 1;
   Eina_Bool          calc_only : 1;
   Eina_Bool          walking_actions : 1;
   Eina_Bool          block_break : 1;
   Eina_Bool          delete_me : 1;
   Eina_Bool          postponed : 1;
   Eina_Bool          freeze_calc : 1;
   Eina_Bool          has_entries : 1;
   Eina_Bool          entries_inited : 1;
#ifdef EDJE_CALC_CACHE
   Eina_Bool          text_part_change : 1;
   Eina_Bool          all_part_change : 1;
#endif
   Eina_Bool          have_mapped_part : 1;
   Eina_Bool          recalc_call : 1;
   Eina_Bool          update_hints : 1;
   Eina_Bool          recalc_hints : 1;
   Eina_Bool          need_map_update : 1;
};

struct _Edje_Calc_Params_Map
{
   struct {
      int x, y, z;
   } center; // 12
   struct {
      FLOAT_T x, y, z;
   } rotation; // 24
   struct {
      int x, y, z;
      int r, g, b;
      int ar, ag, ab;
   } light; // 36
   struct {
      int x, y, z;
      int focal;
   } persp; // 16
   Edje_Map_Color **colors;
   unsigned int colors_count;
};

struct _Edje_Calc_Params_Physics
{
   double mass; // 8
   double restitution; // 8
   double friction; // 8
   double density; // 8
   double hardness; // 8
   struct {
      double linear; //8
      double angular; //8
   } damping; // 16
   struct {
      double linear; //8
      double angular; //8
   } sleep; // 16
   int z; // 4
   int depth; // 4
   struct {
      struct {
         unsigned char x;
         unsigned char y;
         unsigned char z;
      } lin; // 3
      struct {
         unsigned char x;
         unsigned char y;
         unsigned char z;
      } ang; // 3
   } mov_freedom; // 6
   unsigned char backcull;
   unsigned char material; // 1
   unsigned char light_on; // 1
   unsigned char ignore_part_pos; //1
};

struct _Edje_Calc_Params
{
   //   int              x, y, w, h; // 16
   struct {
      FLOAT_T       x, y, w, h; // 32
   } eval;
   Edje_Rectangle   final; // 16
   Edje_Rectangle   req; // 16
   Edje_Rectangle   req_drag; // 16
   Edje_Color       color; // 4
   union {
      struct {
	 struct {
	    int           x, y, w, h; // 16
	    int           angle; // 4
	    int           spread; // 4
	 } fill; // 24

	 union {
	    struct {
	       int           l, r, t, b; // 16
               FLOAT_T       border_scale_by;
	    } image; // 16
	 } spec; // 16
      } common; // 40
      struct {
	 Edje_Alignment align; /* text alignment within bounds */ // 16
	 double         elipsis; // 8
	 int            size; // 4
	 Edje_Color     color2, color3; // 8
      } text; // 36
   } type; // 40
   const Edje_Calc_Params_Map *map; // 88
#ifdef HAVE_EPHYSICS
   const Edje_Calc_Params_Physics *physics; // 90
#endif
   unsigned char    persp_on : 1;
   unsigned char    lighted : 1;
   unsigned char    mapped : 1;
   unsigned char    visible : 1;
   unsigned char    smooth : 1; // 1
}; // 271

struct _Edje_Real_Part_Set
{
  Edje_Image_Directory_Set_Entry *entry; // 4
  Edje_Image_Directory_Set       *set; // 4

  int                             id; // 4
};

struct _Edje_Real_Part_State
{
   Edje_Part_Description_Common *description; // 4
   Edje_Part_Description_Common *description_rtl; // 4
#ifdef EDJE_CALC_CACHE
   int                    state; // 4
   Edje_Calc_Params       p; // 271
#endif
   void                  *external_params; // 4
   Edje_Real_Part_Set    *set; // 4
}; // 32
// WITH EDJE_CALC_CACHE 307

struct _Edje_Real_Part_Drag
{
   FLOAT_T		 x, y; // 16
   Edje_Position_Scale	 val, size, step, page; // 64
   struct {
      unsigned int	 count; // 4
      int		 x, y; // 8
   } down;
   struct {
      int		 x, y; // 8
   } tmp;
   Edje_Real_Part       *confine_to; // 4
   Edje_Real_Part       *threshold; // 4
   Eina_Bool             need_reset : 1; // 4
   Eina_Bool             threshold_started_x : 1;
   Eina_Bool             threshold_started_y : 1;
   Eina_Bool             threshold_x : 1;
   Eina_Bool             threshold_y : 1;
   Eina_Bool             started : 1;
}; // 104

#define EDJE_RP_TYPE_NONE 0
#define EDJE_RP_TYPE_TEXT 1
#define EDJE_RP_TYPE_CONTAINER 2
#define EDJE_RP_TYPE_SWALLOW 3

struct _Edje_Real_Part_Text
{
   void                  *entry_data; // 4
   Edje_Real_Part        *source; // 4
   Edje_Real_Part        *text_source; // 4
   const char            *text; // 4
   const char            *font; // 4
   const char            *style; // 4
   Edje_Position          offset; // 8
   short                  size; // 2
   struct {
      unsigned char       fit_x, fit_y; // 2
      short               in_size; // 2
      short               out_size; // 2
      float               elipsis; // 4
      Evas_Coord          in_w, in_h; // 8
      const char         *in_str; // 4
      const char         *out_str; // 4
      const char         *in_font; // 4
      FLOAT_T             align_x, align_y; // 16
   } cache;
}; // 88
// FIXME make text a potiner to struct and alloc at end
// if part type is TEXT move common members textblock +
// text to front and have smaller struct for textblock

struct _Edje_Real_Part_Container
{
   Eina_List                *items; // 4 //FIXME: only if table/box
   Edje_Part_Box_Animation  *anim; // 4 //FIXME: Used only if box
};

struct _Edje_Real_Part_Swallow
{
   Evas_Object        *swallowed_object; // 4 // FIXME: move with swallow_params data
   struct {
      Edje_Size min, max; // 16
      Edje_Aspect aspect; // 12
   } swallow_params; // 28 // FIXME: only if type SWALLOW
};

struct _Edje_Real_Part
{
   Edje_Real_Part_State      param1; // 32
   Edje_Real_Part_State     *param2, *custom; // 8
   Edje_Part                *part; // 4
   Evas_Object              *object; // 4
   Evas_Object              *nested_smart; // 4
   Edje_Real_Part_Drag      *drag; // 4
   Edje_Part_Description_Common *chosen_description; // 4
   // WITH EDJE_CALC_CACHE: 307
   Edje_Running_Program     *program; // 4
   Edje_Calc_Params         *current; // 4
#ifdef HAVE_EPHYSICS
   Edje_Part_Description_Common *prev_description; // 4
   EPhysics_Body            *body; // 4
   Eina_List                *body_faces; // 4
#endif
   union {
      Edje_Real_Part_Text      *text;
      Edje_Real_Part_Container *container;
      Edje_Real_Part_Swallow   *swallow;
   } typedata; // 4
   FLOAT_T                   description_pos; // 8
   Edje_Rectangle            req; // 16
   int                       x, y, w, h; // 16
   int                       clicked_button; // 4
#ifdef EDJE_CALC_CACHE
   int                       state; // 4
#endif
   unsigned char             type; // 1
   unsigned char             calculated : 2; // 1
   unsigned char             calculating : 2; // 0
   Eina_Bool                 still_in   : 1; // 0
#ifdef EDJE_CALC_CACHE
   Eina_Bool                 invalidate : 1; // 0
#endif
}; // 128
// WITH EDJE_CALC_CACHE: 407

struct _Edje_Running_Program
{
   Edje           *edje;
   Edje_Program   *program;
   double          start_time;
   Eina_Bool       delete_me : 1;
};

struct _Edje_Signal_Callback
{
   const char	  *signal;
   const char	  *source;
   Edje_Signal_Cb  func;
   void           *data;
   Eina_Bool       just_added : 1;
   Eina_Bool       delete_me : 1;
   Eina_Bool       propagate : 1;
};

struct _Edje_Text_Insert_Filter_Callback
{
   const char  *part;
   Edje_Text_Filter_Cb func;
   void        *data;
};

struct _Edje_Markup_Filter_Callback
{
   const char  *part;
   Edje_Markup_Filter_Cb func;
   void        *data;
};

struct _Edje_Pending_Program
{
   Edje         *edje;
   Edje_Program *program;
   Ecore_Timer  *timer;
};

struct _Edje_Text_Style
{
   struct {
      unsigned char x, y;
   } offset;
   struct {
      unsigned char l, r, t, b;
   } pad;
   int num;
   struct {
      unsigned char color; /* 0 = color, 1, 2 = color2, color3 */
      signed   char x, y; /* offset */
      unsigned char alpha;
   } members[32];
};

struct _Edje_Color_Class
{
   const char    *name;
   unsigned char  r, g, b, a;
   unsigned char  r2, g2, b2, a2;
   unsigned char  r3, g3, b3, a3;
};

struct _Edje_Text_Class
{
   const char     *name;
   const char     *font;
   Evas_Font_Size  size;
};

struct _Edje_Var_Int
{
   int      v;
};

struct _Edje_Var_Float
{
   double   v;
};

struct _Edje_Var_String
{
   char    *v;
};

struct _Edje_Var_List
{
   Eina_List *v;
};

struct _Edje_Var_Hash
{
   Eina_Hash *v;
};

struct _Edje_Var_Timer
{
   Edje           *edje;
   int             id;
   Embryo_Function func;
   int             val;
   Ecore_Timer    *timer;
};

struct _Edje_Var_Animator
{
   Edje           *edje;
   int             id;
   Embryo_Function func;
   int             val;
   double          start, len;
   char            delete_me;
};

struct _Edje_Var_Pool
{
   int          id_count;
   Eina_List   *timers;
   Eina_List   *animators;
   int          size;
   Edje_Var    *vars;
   int          walking_list;
};

struct _Edje_Var
{
   union {
      Edje_Var_Int    i;
      Edje_Var_Float  f;
      Edje_Var_String s;
      Edje_Var_List   l;
      Edje_Var_Hash   h;
   } data;
   unsigned char type;
};

typedef enum _Edje_Queue
{
   EDJE_QUEUE_APP,
     EDJE_QUEUE_SCRIPT
} Edje_Queue;

typedef struct _Edje_Message_Signal Edje_Message_Signal;
typedef struct _Edje_Message        Edje_Message;

typedef struct _Edje_Message_Signal_Data Edje_Message_Signal_Data;
struct _Edje_Message_Signal_Data
{
   int ref;
   void *data;
   void (*free_func)(void *);
};

struct _Edje_Message_Signal
{
   const char *sig;
   const char *src;
   Edje_Message_Signal_Data *data;
};

struct _Edje_Message
{
   Edje              *edje;
   Edje_Queue         queue;
   Edje_Message_Type  type;
   int                id;
   unsigned char     *msg;
   Eina_Bool          propagated : 1;
};

typedef enum _Edje_Fill
{
   EDJE_FILL_TYPE_SCALE = 0,
     EDJE_FILL_TYPE_TILE
} Edje_Fill;

typedef enum _Edje_Match_Error
{
   EDJE_MATCH_OK,
     EDJE_MATCH_ALLOC_ERROR,
     EDJE_MATCH_SYNTAX_ERROR

} Edje_Match_Error;

typedef struct _Edje_States     Edje_States;
struct _Edje_Patterns
{
   const char    **patterns;

   Edje_States    *states;

   int             ref;
   Eina_Bool       delete_me : 1;
   
   size_t          patterns_size;
   size_t          max_length;
   unsigned int    finals[];
};

typedef enum _Edje_User_Defined_Type 
{
   EDJE_USER_SWALLOW,
   EDJE_USER_BOX_PACK,
   EDJE_USER_TABLE_PACK,
   EDJE_USER_STRING,
   EDJE_USER_DRAG_STEP,
   EDJE_USER_DRAG_PAGE,
   EDJE_USER_DRAG_VALUE,
   EDJE_USER_DRAG_SIZE
} Edje_User_Defined_Type;

typedef struct _Edje_User_Defined Edje_User_Defined;
struct _Edje_User_Defined
{
   Edje_User_Defined_Type type;
   const char *part;
   Edje *ed;

   union {
      struct {
         const char *text;
      } string;
      struct {
         Evas_Object *child;
      } swallow;
      struct {
         Evas_Object *child;
         int index;
      } box;
      struct {
         Evas_Object *child;
         unsigned short col;
         unsigned short row;
         unsigned short colspan;
         unsigned short rowspan;
      } table;
      struct {
         double x, y;
      } drag_position;
      struct {
         double w, h;
      } drag_size;
   } u;
};

typedef struct _Edje_Font_List             Edje_Font_List;
typedef struct _Edje_Font                  Edje_Font;

struct _Edje_Font_List
{
   Eina_List *list;
};

struct _Edje_Font
{
   char *name;
   char *file;
};

Edje_Patterns   *edje_match_collection_dir_init(const Eina_List *lst);
Edje_Patterns   *edje_match_programs_signal_init(Edje_Program * const *array,
						 unsigned int count);
Edje_Patterns   *edje_match_programs_source_init(Edje_Program * const *array,
						 unsigned int count);
Edje_Patterns   *edje_match_callback_signal_init(const Eina_Inarray *lst,
                                                 const Edje_Signal_Callback_Match *matches);
Edje_Patterns   *edje_match_callback_source_init(const Eina_Inarray *lst,
                                                 const Edje_Signal_Callback_Match *matches);

Eina_Bool        edje_match_collection_dir_exec(const Edje_Patterns      *ppat,
						const char               *string);
Eina_Bool        edje_match_programs_exec(const Edje_Patterns    *ppat_signal,
					  const Edje_Patterns    *ppat_source,
					  const char             *signal,
					  const char             *source,
					  Edje_Program          **programs,
					  Eina_Bool (*func)(Edje_Program *pr, void *data),
					  void                   *data,
                                          Eina_Bool               prop);
int edje_match_callback_exec(const Edje_Signals_Sources_Patterns *ssp,
			     const Edje_Signal_Callback_Match *matches,
			     const char *sig,
			     const char *source,
			     Edje *ed,
			     Eina_Bool prop);

void             edje_match_patterns_free(Edje_Patterns *ppat);

Eina_List *edje_match_program_hash_build(Edje_Program * const * programs,
					 unsigned int count,
					 Eina_Rbtree **tree);
void edje_match_callback_hash_build(const Edje_Signal_Callback_Match *callback,
				    int callbacks_count,
				    Eina_Rbtree **tree,
				    Eina_Inarray *result);
const Eina_Inarray *edje_match_signal_source_hash_get(const char *signal,
						      const char *source,
						      const Eina_Rbtree *tree);
void edje_match_signal_source_free(Edje_Signal_Source_Char *key, void *data);
void _edje_signal_callback_matches_unref(Edje_Signal_Callback_Matches *m);

// FIXME remove below 3 eapi decls when edje_convert goes
EAPI void _edje_edd_init(void);
EAPI void _edje_data_font_list_desc_make(Eet_Data_Descriptor **_font_list_edd, Eet_Data_Descriptor **_font_edd);
EAPI void _edje_edd_shutdown(void);

EAPI extern Eet_Data_Descriptor *_edje_edd_edje_file;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_part_collection;

extern int              _edje_anim_count;
extern Ecore_Animator  *_edje_timer;
extern Eina_List       *_edje_animators;
extern Eina_List       *_edje_edjes;

extern char            *_edje_fontset_append;
extern FLOAT_T          _edje_scale;
extern int              _edje_freeze_val;
extern int              _edje_freeze_calc_count;
extern Eina_List       *_edje_freeze_calc_list;

extern Eina_Bool        _edje_password_show_last;
extern FLOAT_T          _edje_password_show_last_timeout;

extern Eina_Mempool *_edje_real_part_mp;
extern Eina_Mempool *_edje_real_part_state_mp;

extern Eina_Cow *_edje_calc_params_map_cow;
extern Eina_Cow *_edje_calc_params_physics_cow;

EAPI extern Eina_Mempool *_emp_RECTANGLE;
EAPI extern Eina_Mempool *_emp_TEXT;
EAPI extern Eina_Mempool *_emp_IMAGE;
EAPI extern Eina_Mempool *_emp_PROXY;
EAPI extern Eina_Mempool *_emp_SWALLOW;
EAPI extern Eina_Mempool *_emp_TEXTBLOCK;
EAPI extern Eina_Mempool *_emp_GROUP;
EAPI extern Eina_Mempool *_emp_BOX;
EAPI extern Eina_Mempool *_emp_TABLE;
EAPI extern Eina_Mempool *_emp_EXTERNAL;
EAPI extern Eina_Mempool *_emp_SPACER;
EAPI extern Eina_Mempool *_emp_part;

void  _edje_part_pos_set(Edje *ed, Edje_Real_Part *ep, int mode, FLOAT_T pos, FLOAT_T v1, FLOAT_T v2);
Edje_Part_Description_Common *_edje_part_description_find(Edje *ed,
							  Edje_Real_Part *rp,
							  const char *name, double val);
void  _edje_part_description_apply(Edje *ed, Edje_Real_Part *ep, const char  *d1, double v1, const char *d2, double v2);
void  _edje_recalc(Edje *ed);
void  _edje_recalc_do(Edje *ed);
int   _edje_part_dragable_calc(Edje *ed, Edje_Real_Part *ep, FLOAT_T *x, FLOAT_T *y);
void  _edje_dragable_pos_set(Edje *ed, Edje_Real_Part *ep, FLOAT_T x, FLOAT_T y);

Eina_Bool _edje_timer_cb(void *data);
Eina_Bool _edje_pending_timer_cb(void *data);
void  _edje_callbacks_add(Evas_Object *obj, Edje *ed, Edje_Real_Part *rp);
void  _edje_callbacks_focus_add(Evas_Object *obj, Edje *ed, Edje_Real_Part *rp);
void  _edje_callbacks_del(Evas_Object *obj, Edje *ed);
void  _edje_callbacks_focus_del(Evas_Object *obj, Edje *ed);

const Edje_Signal_Callback_Group *_edje_signal_callback_alloc(void);
void _edje_signal_callback_free(const Edje_Signal_Callback_Group *cgp);
void _edje_signal_callback_push(const Edje_Signal_Callback_Group *cgp,
                                const char *signal, const char *source,
                                Edje_Signal_Cb func, void *data,
                                Eina_Bool propagate);
void *_edje_signal_callback_disable(const Edje_Signal_Callback_Group *cgp,
                                    const char *signal, const char *source,
                                    Edje_Signal_Cb func, void *data);

EAPI void _edje_edd_init(void);
EAPI void _edje_edd_shutdown(void);

int _edje_object_file_set_internal(Evas_Object *obj, const Eina_File *file, const char *group, const char *parent, Eina_List *group_path, Eina_Array *nested);

void  _edje_file_del(Edje *ed);
void  _edje_file_free(Edje_File *edf);
void  _edje_file_cache_shutdown(void);
void  _edje_collection_free(Edje_File *edf,
			    Edje_Part_Collection *ec,
			    Edje_Part_Collection_Directory_Entry *ce);
void  _edje_collection_free_part_description_clean(int type,
						   Edje_Part_Description_Common *desc,
						   Eina_Bool free_strings);
void _edje_collection_free_part_description_free(int type,
						 Edje_Part_Description_Common *desc,
						 Edje_Part_Collection_Directory_Entry *ce,
						 Eina_Bool free_strings);

void  _edje_del(Edje *ed);
void  _edje_ref(Edje *ed);
void  _edje_unref(Edje *ed);
void  _edje_clean_objects(Edje *ed);
void  _edje_ref(Edje *ed);
void  _edje_unref(Edje *ed);

Eina_Bool _edje_program_run_iterate(Edje_Running_Program *runp, double tim);
void  _edje_program_end(Edje *ed, Edje_Running_Program *runp);
void  _edje_program_run(Edje *ed, Edje_Program *pr, Eina_Bool force, const char *ssig, const char *ssrc);
void _edje_programs_patterns_clean(Edje_Part_Collection *ed);
void _edje_programs_patterns_init(Edje_Part_Collection *ed);
void  _edje_emit(Edje *ed, const char *sig, const char *src);
void _edje_emit_full(Edje *ed, const char *sig, const char *src, void *data, void (*free_func)(void *));
void _edje_emit_handle(Edje *ed, const char *sig, const char *src, Edje_Message_Signal_Data *data, Eina_Bool prop);
void  _edje_signals_sources_patterns_clean(Edje_Signals_Sources_Patterns *ssp);

const Edje_Signals_Sources_Patterns *_edje_signal_callback_patterns_ref(const Edje_Signal_Callback_Group *gp);
void _edje_signal_callback_patterns_unref(const Edje_Signals_Sources_Patterns *essp);
void _edje_signal_callback_reset(Edje_Signal_Callback_Flags *flags, unsigned int length);

void _edje_signal_callback_free(const Edje_Signal_Callback_Group *gp);

void           _edje_text_init(void);
void           _edje_text_part_on_add(Edje *ed, Edje_Real_Part *ep);
void           _edje_text_part_on_del(Edje *ed, Edje_Part *ep);
void           _edje_text_recalc_apply(Edje *ed,
                                       Edje_Real_Part *ep,
                                       Edje_Calc_Params *params,
                                       Edje_Part_Description_Text *chosen_desc,
				       Eina_Bool calc_only);
Evas_Font_Size _edje_text_size_calc(Evas_Font_Size size, Edje_Text_Class *tc);
const char *   _edje_text_class_font_get(Edje *ed,
                                         Edje_Part_Description_Text *chosen_desc,
                                         int *size, char **free_later);
const char *   _edje_text_font_get(const char *base, const char *new,
                                   char **free_later);



Edje_Real_Part   *_edje_real_part_get(const Edje *ed, const char *part);
Edje_Real_Part   *_edje_real_part_recursive_get(Edje **ed, const char *part);
Edje_Color_Class *_edje_color_class_find(const Edje *ed, const char *color_class);
void              _edje_color_class_member_direct_del(const char *color_class, void *lookup);
void              _edje_color_class_member_add(Edje *ed, const char *color_class);
void              _edje_color_class_member_del(Edje *ed, const char *color_class);
void              _edje_color_class_on_del(Edje *ed, Edje_Part *ep);
void              _edje_color_class_members_free(void);
void              _edje_color_class_hash_free(void);

Edje_Text_Class  *_edje_text_class_find(Edje *ed, const char *text_class);
void              _edje_text_class_member_add(Edje *ed, const char *text_class);
void              _edje_text_class_member_del(Edje *ed, const char *text_class);
void              _edje_text_class_member_direct_del(const char *text_class, void *lookup);
void              _edje_text_class_members_free(void);
void              _edje_text_class_hash_free(void);

Edje             *_edje_fetch(const Evas_Object *obj) EINA_PURE;
int               _edje_freeze(Edje *ed);
int               _edje_thaw(Edje *ed);
int               _edje_block(Edje *ed);
int               _edje_unblock(Edje *ed);
int               _edje_block_break(Edje *ed);
void              _edje_block_violate(Edje *ed);
void              _edje_object_part_swallow_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
void              _edje_object_part_swallow_changed_hints_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
void              _edje_real_part_swallow(Edje *ed, Edje_Real_Part *rp, Evas_Object *obj_swallow, Eina_Bool hints_update);
void              _edje_real_part_swallow_clear(Edje *ed, Edje_Real_Part *rp);
void              _edje_box_init(void);
void              _edje_box_shutdown(void);
Eina_Bool         _edje_box_layout_find(const char *name, Evas_Object_Box_Layout *cb, void **data, void (**free_data)(void *data));
void              _edje_box_recalc_apply(Edje *ed EINA_UNUSED, Edje_Real_Part *ep, Edje_Calc_Params *p3, Edje_Part_Description_Box *chosen_desc);
Eina_Bool         _edje_box_layout_add_child(Edje_Real_Part *rp, Evas_Object *child_obj);
void              _edje_box_layout_remove_child(Edje_Real_Part *rp, Evas_Object *child_obj);
Edje_Part_Box_Animation * _edje_box_layout_anim_new(Evas_Object *box);
void              _edje_box_layout_free_data(void *data);

Eina_Bool         _edje_real_part_box_append(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj);
Eina_Bool         _edje_real_part_box_prepend(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj);
Eina_Bool         _edje_real_part_box_insert_before(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj, const Evas_Object *ref);
Eina_Bool         _edje_real_part_box_insert_at(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj, unsigned int pos);
Evas_Object      *_edje_real_part_box_remove(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj);
Evas_Object      *_edje_real_part_box_remove_at(Edje *ed, Edje_Real_Part *rp, unsigned int pos);
Eina_Bool         _edje_real_part_box_remove_all(Edje *ed, Edje_Real_Part *rp, Eina_Bool clear);
Eina_Bool         _edje_real_part_table_pack(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);
Eina_Bool         _edje_real_part_table_unpack(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj);
void              _edje_real_part_table_clear(Edje *ed, Edje_Real_Part *rp, Eina_Bool clear);
Evas_Object      *_edje_children_get(Edje_Real_Part *rp, const char *partid);

Eina_Bool         _edje_object_part_text_raw_set(Edje *ed, Evas_Object *obj, Edje_Real_Part *rp, const char *part, const char *text);
char             *_edje_text_escape(const char *text);
char             *_edje_text_unescape(const char *text);

void          _edje_embryo_script_init      (Edje_Part_Collection *edc);
void          _edje_embryo_script_shutdown  (Edje_Part_Collection *edc);
void          _edje_embryo_script_reset     (Edje *ed);
void          _edje_embryo_test_run         (Edje *ed, const char *fname, const char *sig, const char *src);
Edje_Var     *_edje_var_new                 (void);
void          _edje_var_free                (Edje_Var *var);
void          _edje_var_init                (Edje *ed);
void          _edje_var_shutdown            (Edje *ed);
int           _edje_var_string_id_get       (Edje *ed, const char *string);
int           _edje_var_var_int_get         (Edje *ed, Edje_Var *var);
void          _edje_var_var_int_set         (Edje *ed, Edje_Var *var, int v);
double        _edje_var_var_float_get       (Edje *ed, Edje_Var *var);
void          _edje_var_var_float_set       (Edje *ed, Edje_Var *var, double v);
const char   *_edje_var_var_str_get         (Edje *ed, Edje_Var *var);
void          _edje_var_var_str_set         (Edje *ed, Edje_Var *var, const char *str);
int           _edje_var_int_get             (Edje *ed, int id);
void          _edje_var_int_set             (Edje *ed, int id, int v);
double        _edje_var_float_get           (Edje *ed, int id);
void          _edje_var_float_set           (Edje *ed, int id, double v);
const char   *_edje_var_str_get             (Edje *ed, int id);
void          _edje_var_str_set             (Edje *ed, int id, const char *str);

void          _edje_var_list_var_append(Edje *ed, int id, Edje_Var *var);
void          _edje_var_list_var_prepend(Edje *ed, int id, Edje_Var *var);
void          _edje_var_list_var_append_relative(Edje *ed, int id, Edje_Var *var, Edje_Var *relative);
void          _edje_var_list_var_prepend_relative(Edje *ed, int id, Edje_Var *var, Edje_Var *relative);
Edje_Var     *_edje_var_list_nth(Edje *ed, int id, int n);

int           _edje_var_list_count_get(Edje *ed, int id);
void          _edje_var_list_remove_nth(Edje *ed, int id, int n);

int           _edje_var_list_nth_int_get(Edje *ed, int id, int n);
void          _edje_var_list_nth_int_set(Edje *ed, int id, int n, int v);
void          _edje_var_list_int_append(Edje *ed, int id, int v);
void          _edje_var_list_int_prepend(Edje *ed, int id, int v);
void          _edje_var_list_int_insert(Edje *ed, int id, int n, int v);

double        _edje_var_list_nth_float_get(Edje *ed, int id, int n);
void          _edje_var_list_nth_float_set(Edje *ed, int id, int n, double v);
void          _edje_var_list_float_append(Edje *ed, int id, double v);
void          _edje_var_list_float_prepend(Edje *ed, int id, double v);
void          _edje_var_list_float_insert(Edje *ed, int id, int n, double v);

const char   *_edje_var_list_nth_str_get(Edje *ed, int id, int n);
void          _edje_var_list_nth_str_set(Edje *ed, int id, int n, const char *v);
void          _edje_var_list_str_append(Edje *ed, int id, const char *v);
void          _edje_var_list_str_prepend(Edje *ed, int id, const char *v);
void          _edje_var_list_str_insert(Edje *ed, int id, int n, const char *v);

int           _edje_var_timer_add           (Edje *ed, double in, const char *fname, int val);
void          _edje_var_timer_del           (Edje *ed, int id);

int           _edje_var_anim_add            (Edje *ed, double len, const char *fname, int val);
void          _edje_var_anim_del            (Edje *ed, int id);

void          _edje_message_init            (void);
void          _edje_message_shutdown        (void);
void          _edje_message_cb_set          (Edje *ed, void (*func) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg), void *data);
Edje_Message *_edje_message_new             (Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id);
void          _edje_message_free            (Edje_Message *em);
void          _edje_message_propornot_send  (Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id, void *emsg, Eina_Bool prop);
void          _edje_message_send            (Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id, void *emsg);
void          _edje_message_parameters_push (Edje_Message *em);
void          _edje_message_process         (Edje_Message *em);
void          _edje_message_queue_process   (void);
void          _edje_message_queue_clear     (void);
void          _edje_message_del             (Edje *ed);

void _edje_textblock_styles_add(Edje *ed);
void _edje_textblock_styles_del(Edje *ed);
void _edje_textblock_style_all_update(Edje *ed);
void _edje_textblock_style_parse_and_fix(Edje_File *edf);
void _edje_textblock_style_cleanup(Edje_File *edf);
Edje_File *_edje_cache_file_coll_open(const Eina_File *file, const char *coll, int *error_ret, Edje_Part_Collection **edc_ret, Edje *ed);
void _edje_cache_coll_clean(Edje_File *edf);
void _edje_cache_coll_flush(Edje_File *edf);
void _edje_cache_coll_unref(Edje_File *edf, Edje_Part_Collection *edc);
EAPI void edje_cache_emp_alloc(Edje_Part_Collection_Directory_Entry *ce);
EAPI void edje_cache_emp_free(Edje_Part_Collection_Directory_Entry *ce);
EAPI void _edje_cache_file_unref(Edje_File *edf);

void _edje_embryo_globals_init(Edje *ed);

#define CHKPARAM(n) if (params[0] != (sizeof(Embryo_Cell) * (n))) return -1;
#define HASNPARAMS(n) (params[0] == (sizeof(Embryo_Cell) * (n)))
#define GETSTR(str, par) { \
   Embryo_Cell *___cptr; \
   int ___l; \
   str = NULL; \
   if ((___cptr = embryo_data_address_get(ep, (par)))) { \
      ___l = embryo_data_string_length_get(ep, ___cptr); \
      if (((str) = alloca(___l + 1))) \
	embryo_data_string_get(ep, ___cptr, (str)); } }
#define GETSTREVAS(str, par) { \
   if ((str)) { \
      if ((par) && (!strcmp((par), (str)))) return 0; \
      if ((par)) eina_stringshare_del((par)); \
      (par) = (char *)eina_stringshare_add((str)); } \
   else (par) = NULL; }
#define GETFLOAT(val, par) { \
   float *___cptr; \
   if ((___cptr = (float *)embryo_data_address_get(ep, (par)))) { \
      val = *___cptr; } }

#define GETFLOAT_T(val, par)						\
  {									\
     float *___cptr;							\
     if ((___cptr = (float *)embryo_data_address_get(ep, (par))))	\
       {								\
	  val = FROM_DOUBLE(*___cptr);					\
       }								\
  }

#define GETINT(val, par) {			\
   int *___cptr; \
   if ((___cptr = (int *)embryo_data_address_get(ep, (par)))) { \
      val = *___cptr; } }
#define SETSTR(str, par) { \
   Embryo_Cell *___cptr; \
   if ((___cptr = embryo_data_address_get(ep, (par)))) { \
      embryo_data_string_set(ep, str, ___cptr); } }
#define SETSTRALLOCATE(s)			\
  {						\
     if (s) {					\
	if ((int) strlen((s)) < params[4]) {	\
	   SETSTR((s), params[3]); }		\
	else {					\
	   char *ss;				\
	   ss = alloca(strlen((s)) + 1);	\
	   strcpy(ss, (s));			\
	   ss[params[4] - 2] = 0;		\
	   SETSTR(ss, params[3]); } }		\
     else					\
       SETSTR("", params[3]);			\
  }
#define SETFLOAT(val, par) { \
   float *___cptr; \
   if ((___cptr = (float *)embryo_data_address_get(ep, (par)))) { \
      *___cptr = (float)val; } }
#define SETFLOAT_T(val, par)						\
  {									\
     float *___cptr;							\
     if ((___cptr = (float *)embryo_data_address_get(ep, (par))))	\
       {								\
	  *___cptr = (float) TO_DOUBLE(val);				\
       }								\
  }
#define SETINT(val, par) { \
   int *___cptr; \
   if ((___cptr = (int *)embryo_data_address_get(ep, (par)))) { \
      *___cptr = (int)val; } }

Eina_Bool _edje_script_only(Edje *ed);
void _edje_script_only_init(Edje *ed);
void _edje_script_only_shutdown(Edje *ed);
void _edje_script_only_show(Edje *ed);
void _edje_script_only_hide(Edje *ed);
void _edje_script_only_move(Edje *ed);
void _edje_script_only_resize(Edje *ed);
void _edje_script_only_message(Edje *ed, Edje_Message *em);

extern jmp_buf _edje_lua_panic_jmp;
#define _edje_lua_panic_here() setjmp(_edje_lua_panic_jmp)

lua_State *_edje_lua_state_get(void);
lua_State *_edje_lua_new_thread(Edje *ed, lua_State *L);
void _edje_lua_free_thread(Edje *ed, lua_State *L);
void _edje_lua_new_reg(lua_State *L, int index, void *ptr);
void _edje_lua_get_reg(lua_State *L, void *ptr);
void _edje_lua_free_reg(lua_State *L, void *ptr);
void _edje_lua_script_fn_new(Edje *ed);
void _edje_lua_group_fn_new(Edje *ed);
void _edje_lua_init(void);
void _edje_lua_shutdown(void);

void __edje_lua_error(const char *file, const char *fnc, int line, lua_State *L, int err_code);
#define _edje_lua_error(L, err_code)					\
  __edje_lua_error(__FILE__, __FUNCTION__, __LINE__, L, err_code)

Eina_Bool  _edje_lua_script_only(Edje *ed);
void _edje_lua_script_only_init(Edje *ed);
void _edje_lua_script_only_shutdown(Edje *ed);
void _edje_lua_script_only_show(Edje *ed);
void _edje_lua_script_only_hide(Edje *ed);
void _edje_lua_script_only_move(Edje *ed);
void _edje_lua_script_only_resize(Edje *ed);
void _edje_lua_script_only_message(Edje *ed, Edje_Message *em);

void _edje_entry_init(Edje *ed);
void _edje_entry_shutdown(Edje *ed);
void _edje_entry_real_part_init(Edje *ed, Edje_Real_Part *rp);
void _edje_entry_real_part_shutdown(Edje *ed, Edje_Real_Part *rp);
void _edje_entry_real_part_configure(Edje *ed, Edje_Real_Part *rp);
const char *_edje_entry_selection_get(Edje_Real_Part *rp);
const char *_edje_entry_text_get(Edje_Real_Part *rp);
void _edje_entry_text_markup_set(Edje_Real_Part *rp, const char *text);
void _edje_entry_text_markup_insert(Edje_Real_Part *rp, const char *text);
void _edje_entry_text_markup_append(Edje_Real_Part *rp, const char *text);
void _edje_entry_set_cursor_start(Edje_Real_Part *rp);
void _edje_entry_set_cursor_end(Edje_Real_Part *rp);
void _edje_entry_cursor_copy(Edje_Real_Part *rp, Edje_Cursor cur, Edje_Cursor dst);
void _edje_entry_select_none(Edje_Real_Part *rp);
void _edje_entry_select_all(Edje_Real_Part *rp);
void _edje_entry_select_begin(Edje_Real_Part *rp);
void _edje_entry_select_extend(Edje_Real_Part *rp);
const Eina_List *_edje_entry_anchor_geometry_get(Edje_Real_Part *rp, const char *anchor);
const Eina_List *_edje_entry_anchors_list(Edje_Real_Part *rp);
Eina_Bool _edje_entry_item_geometry_get(Edje_Real_Part *rp, const char *item, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);
const Eina_List *_edje_entry_items_list(Edje_Real_Part *rp);
void _edje_entry_cursor_geometry_get(Edje_Real_Part *rp, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);
void _edje_entry_user_insert(Edje_Real_Part *rp, const char *text);
void _edje_entry_select_allow_set(Edje_Real_Part *rp, Eina_Bool allow);
Eina_Bool _edje_entry_select_allow_get(const Edje_Real_Part *rp);
void _edje_entry_select_abort(Edje_Real_Part *rp);
void *_edje_entry_imf_context_get(Edje_Real_Part *rp);
Eina_Bool _edje_entry_cursor_next(Edje_Real_Part *rp, Edje_Cursor cur);
Eina_Bool _edje_entry_cursor_prev(Edje_Real_Part *rp, Edje_Cursor cur);
Eina_Bool _edje_entry_cursor_up(Edje_Real_Part *rp, Edje_Cursor cur);
Eina_Bool _edje_entry_cursor_down(Edje_Real_Part *rp, Edje_Cursor cur);
void _edje_entry_cursor_begin(Edje_Real_Part *rp, Edje_Cursor cur);
void _edje_entry_cursor_end(Edje_Real_Part *rp, Edje_Cursor cur);
void _edje_entry_cursor_line_begin(Edje_Real_Part *rp, Edje_Cursor cur);
void _edje_entry_cursor_line_end(Edje_Real_Part *rp, Edje_Cursor cur);
Eina_Bool _edje_entry_cursor_coord_set(Edje_Real_Part *rp, Edje_Cursor cur, int x, int y);
Eina_Bool _edje_entry_cursor_is_format_get(Edje_Real_Part *rp, Edje_Cursor cur);
Eina_Bool _edje_entry_cursor_is_visible_format_get(Edje_Real_Part *rp, Edje_Cursor cur);
char *_edje_entry_cursor_content_get(Edje_Real_Part *rp, Edje_Cursor cur);
void _edje_entry_cursor_pos_set(Edje_Real_Part *rp, Edje_Cursor cur, int pos);
int _edje_entry_cursor_pos_get(Edje_Real_Part *rp, Edje_Cursor cur);
void _edje_entry_imf_context_reset(Edje_Real_Part *rp);
void _edje_entry_input_panel_layout_set(Edje_Real_Part *rp, Edje_Input_Panel_Layout layout);
Edje_Input_Panel_Layout _edje_entry_input_panel_layout_get(Edje_Real_Part *rp);
void _edje_entry_input_panel_layout_variation_set(Edje_Real_Part *rp, int variation);
int _edje_entry_input_panel_layout_variation_get(Edje_Real_Part *rp);
void _edje_entry_autocapital_type_set(Edje_Real_Part *rp, Edje_Text_Autocapital_Type autocapital_type);
Edje_Text_Autocapital_Type _edje_entry_autocapital_type_get(Edje_Real_Part *rp);
void _edje_entry_prediction_allow_set(Edje_Real_Part *rp, Eina_Bool prediction);
Eina_Bool _edje_entry_prediction_allow_get(Edje_Real_Part *rp);
void _edje_entry_input_panel_enabled_set(Edje_Real_Part *rp, Eina_Bool enabled);
Eina_Bool _edje_entry_input_panel_enabled_get(Edje_Real_Part *rp);
void _edje_entry_input_panel_show(Edje_Real_Part *rp);
void _edje_entry_input_panel_hide(Edje_Real_Part *rp);
void _edje_entry_input_panel_language_set(Edje_Real_Part *rp, Edje_Input_Panel_Lang lang);
Edje_Input_Panel_Lang _edje_entry_input_panel_language_get(Edje_Real_Part *rp);
void _edje_entry_input_panel_imdata_set(Edje_Real_Part *rp, const void *data, int len);
void _edje_entry_input_panel_imdata_get(Edje_Real_Part *rp, void *data, int *len);
void _edje_entry_input_panel_return_key_type_set(Edje_Real_Part *rp, Edje_Input_Panel_Return_Key_Type return_key_type);
Edje_Input_Panel_Return_Key_Type _edje_entry_input_panel_return_key_type_get(Edje_Real_Part *rp);
void _edje_entry_input_panel_return_key_disabled_set(Edje_Real_Part *rp, Eina_Bool disabled);
Eina_Bool _edje_entry_input_panel_return_key_disabled_get(Edje_Real_Part *rp);

void _edje_external_init(void);
void _edje_external_shutdown(void);
Evas_Object *_edje_external_type_add(const char *type_name, Evas *evas, Evas_Object *parent, const Eina_List *params, const char *part_name);
void _edje_external_signal_emit(Evas_Object *obj, const char *emission, const char *source);
Eina_Bool _edje_external_param_set(Evas_Object *obj, Edje_Real_Part *rp, const Edje_External_Param *param) EINA_ARG_NONNULL(2);
Eina_Bool _edje_external_param_get(const Evas_Object *obj, Edje_Real_Part *rp, Edje_External_Param *param) EINA_ARG_NONNULL(2);
Evas_Object *_edje_external_content_get(const Evas_Object *obj, const char *content) EINA_ARG_NONNULL(1, 2);
void _edje_external_params_free(Eina_List *params, Eina_Bool free_strings);
void _edje_external_recalc_apply(Edje *ed, Edje_Real_Part *ep,
				 Edje_Calc_Params *params,
				 Edje_Part_Description_Common *chosen_desc);
void *_edje_external_params_parse(Evas_Object *obj, const Eina_List *params);
void _edje_external_parsed_params_free(Evas_Object *obj, void *params);

Eina_Module *_edje_module_handle_load(const char *module);
void _edje_module_init(void);
void _edje_module_shutdown(void);

void _edje_need_imf(void);

static inline Eina_Bool
edje_program_is_strncmp(const char *str)
{
   size_t length;

   length = strlen(str);

   if (strpbrk(str, "*?[\\") != str + length)
     return EINA_FALSE;
   if (str[length] == '['
       || str[length] == '\\')
     return EINA_FALSE;
   return EINA_TRUE;
}

static inline Eina_Bool
edje_program_is_strrncmp(const char *str)
{
   if (*str != '*' && *str != '?')
     return EINA_FALSE;
   if (strpbrk(str + 1, "*?[\\"))
     return EINA_FALSE;
   return EINA_TRUE;
}
void edje_object_propagate_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source), void *data);


/* used by edje_cc - private still */
EAPI void _edje_program_insert(Edje_Part_Collection *ed, Edje_Program *p);
EAPI void _edje_program_remove(Edje_Part_Collection *ed, Edje_Program *p);

void _edje_lua2_error_full(const char *file, const char *fnc, int line, lua_State *L, int err_code);
#define _edje_lua2_error(L, err_code) _edje_lua2_error_full(__FILE__, __FUNCTION__, __LINE__, L, err_code)
void _edje_lua2_script_init(Edje *ed);
void _edje_lua2_script_shutdown(Edje *ed);
void _edje_lua2_script_load(Edje_Part_Collection *edc, void *data, int size);
void _edje_lua2_script_unload(Edje_Part_Collection *edc);

void _edje_lua2_script_func_shutdown(Edje *ed);
void _edje_lua2_script_func_show(Edje *ed);
void _edje_lua2_script_func_hide(Edje *ed);
void _edje_lua2_script_func_move(Edje *ed);
void _edje_lua2_script_func_resize(Edje *ed);
void _edje_lua2_script_func_message(Edje *ed, Edje_Message *em);
void _edje_lua2_script_func_signal(Edje *ed, const char *sig, const char *src);

const char *edje_string_get(const Edje_String *es);
const char *edje_string_id_get(const Edje_String *es);

void _edje_object_orientation_inform(Evas_Object *obj);

void _edje_lib_ref(void);
void _edje_lib_unref(void);

void _edje_subobj_register(Edje *ed, Evas_Object *ob);
void _edje_subobj_unregister(Edje *ed, Evas_Object *ob);

void _edje_multisense_init(void);
void _edje_multisense_shutdown(void);
Eina_Bool _edje_multisense_internal_sound_sample_play(Edje *ed, const char *sample_name, const double speed);
Eina_Bool _edje_multisense_internal_sound_tone_play(Edje *ed, const char *tone_name, const double duration);

void _edje_part_recalc(Edje *ed, Edje_Real_Part *ep, int flags, Edje_Calc_Params *state);

void _edje_user_definition_remove(Edje_User_Defined *eud, Evas_Object *child);
void _edje_user_definition_free(Edje_User_Defined *eud);

void _scale_set(Eo *obj, void *_pd, va_list *list);
void _scale_get(Eo *obj, void *_pd, va_list *list);
void _mirrored_get(Eo *obj, void *_pd, va_list *list);
void _mirrored_set(Eo *obj, void *_pd, va_list *list);
void _data_get(Eo *obj, void *_pd, va_list *list);
void _freeze(Eo *obj, void *_pd, va_list *list);
void _thaw(Eo *obj, void *_pd, va_list *list);
void _color_class_set(Eo *obj, void *_pd, va_list *list);
void _color_class_get(Eo *obj, void *_pd, va_list *list);
void _text_class_set(Eo *obj, void *_pd, va_list *list);
void _part_exists(Eo *obj, void *_pd, va_list *list);
void _part_object_get(Eo *obj, void *_pd, va_list *list);
void _part_geometry_get(Eo *obj, void *_pd, va_list *list);
void _item_provider_set(Eo *obj, void *_pd, va_list *list);
void _text_change_cb_set(Eo *obj, void *_pd, va_list *list);
void _part_text_style_user_push(Eo *obj, void *_pd, va_list *list);
void _part_text_style_user_pop(Eo *obj, void *_pd, va_list *list);
void _part_text_style_user_peek(Eo *obj, void *_pd, va_list *list);
void _part_text_set(Eo *obj, void *_pd, va_list *list);
void _part_text_get(Eo *obj, void *_pd, va_list *list);
void _part_text_escaped_set(Eo *obj, void *_pd, va_list *list);
void _part_text_unescaped_set(Eo *obj, void *_pd, va_list *list);
void _part_text_unescaped_get(Eo *obj, void *_pd, va_list *list);
void _part_text_selection_get(Eo *obj, void *_pd, va_list *list);
void _part_text_select_none(Eo *obj, void *_pd, va_list *list);
void _part_text_select_all(Eo *obj, void *_pd, va_list *list);
void _part_text_insert(Eo *obj, void *_pd, va_list *list);
void _part_text_append(Eo *obj, void *_pd, va_list *list);
void _part_text_anchor_list_get(Eo *obj, void *_pd, va_list *list);
void _part_text_anchor_geometry_get(Eo *obj, void *_pd, va_list *list);
void _part_text_item_list_get(Eo *obj, void *_pd, va_list *list);
void _part_text_item_geometry_get(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_geometry_get(Eo *obj, void *_pd, va_list *list);
void _part_text_user_insert(Eo *obj, void *_pd, va_list *list);
void _part_text_select_allow_set(Eo *obj, void *_pd, va_list *list);
void _part_text_select_abort(Eo *obj, void *_pd, va_list *list);
void _part_text_select_begin(Eo *obj, void *_pd, va_list *list);
void _part_text_select_extend(Eo *obj, void *_pd, va_list *list);
void _part_text_imf_context_get(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_next(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_prev(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_up(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_down(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_begin_set(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_end_set(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_copy(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_line_begin_set(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_line_end_set(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_coord_set(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_is_format_get(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_is_visible_format_get(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_content_get(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_pos_set(Eo *obj, void *_pd, va_list *list);
void _part_text_cursor_pos_get(Eo *obj, void *_pd, va_list *list);
void _part_text_imf_context_reset(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_layout_set(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_layout_get(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_layout_variation_set(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_layout_variation_get(Eo *obj, void *_pd, va_list *list);
void _part_text_autocapital_type_set(Eo *obj, void *_pd, va_list *list);
void _part_text_autocapital_type_get(Eo *obj, void *_pd, va_list *list);
void _part_text_prediction_allow_set(Eo *obj, void *_pd, va_list *list);
void _part_text_prediction_allow_get(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_enabled_set(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_enabled_get(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_show(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_hide(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_language_set(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_language_get(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_imdata_set(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_imdata_get(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_return_key_type_set(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_return_key_type_get(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_return_key_disabled_set(Eo *obj, void *_pd, va_list *list);
void _part_text_input_panel_return_key_disabled_get(Eo *obj, void *_pd, va_list *list);
void _text_insert_filter_callback_add(Eo *obj, void *_pd, va_list *list);
void _text_insert_filter_callback_del(Eo *obj, void *_pd, va_list *list);
void _text_insert_filter_callback_del_full(Eo *obj, void *_pd, va_list *list);
void _text_markup_filter_callback_add(Eo *obj, void *_pd, va_list *list);
void _text_markup_filter_callback_del(Eo *obj, void *_pd, va_list *list);
void _text_markup_filter_callback_del_full(Eo *obj, void *_pd, va_list *list);
void _part_swallow(Eo *obj, void *_pd, va_list *list);
void _part_unswallow(Eo *obj, void *_pd, va_list *list);
void _part_swallow_get(Eo *obj, void *_pd, va_list *list);
void _size_min_get(Eo *obj, void *_pd, va_list *list);
void _size_max_get(Eo *obj, void *_pd, va_list *list);
void _calc_force(Eo *obj, void *_pd, va_list *list);
void _size_min_calc(Eo *obj, void *_pd, va_list *list);
void _parts_extends_calc(Eo *obj, void *_pd, va_list *list);
void _size_min_restricted_calc(Eo *obj, void *_pd, va_list *list);
void _part_state_get(Eo *obj, void *_pd, va_list *list);
void _part_drag_dir_get(Eo *obj, void *_pd, va_list *list);
void _part_drag_value_set(Eo *obj, void *_pd, va_list *list);
void _part_drag_value_get(Eo *obj, void *_pd, va_list *list);
void _part_drag_size_set(Eo *obj, void *_pd, va_list *list);
void _part_drag_size_get(Eo *obj, void *_pd, va_list *list);
void _part_drag_step_set(Eo *obj, void *_pd, va_list *list);
void _part_drag_step_get(Eo *obj, void *_pd, va_list *list);
void _part_drag_page_set(Eo *obj, void *_pd, va_list *list);
void _part_drag_page_get(Eo *obj, void *_pd, va_list *list);
void _part_drag_step(Eo *obj, void *_pd, va_list *list);
void _part_drag_page(Eo *obj, void *_pd, va_list *list);
void _part_box_append(Eo *obj, void *_pd, va_list *list);
void _part_box_prepend(Eo *obj, void *_pd, va_list *list);
void _part_box_insert_before(Eo *obj, void *_pd, va_list *list);
void _part_box_insert_at(Eo *obj, void *_pd, va_list *list);
void _part_box_remove(Eo *obj, void *_pd, va_list *list);
void _part_box_remove_at(Eo *obj, void *_pd, va_list *list);
void _part_box_remove_all(Eo *obj, void *_pd, va_list *list);
void _access_part_list_get(Eo *obj, void *_pd, va_list *list);
void _part_table_child_get(Eo *obj, void *_pd, va_list *list);
void _part_table_pack(Eo *obj, void *_pd, va_list *list);
void _part_table_unpack(Eo *obj, void *_pd, va_list *list);
void _part_table_col_row_size_get(Eo *obj, void *_pd, va_list *list);
void _part_table_clear(Eo *obj, void *_pd, va_list *list);
void _perspective_set(Eo *obj, void *_pd, va_list *list);
void _perspective_get(Eo *obj, void *_pd, va_list *list);
void _preload(Eo *obj, void *_pd, va_list *list);
void _update_hints_set(Eo *obj, void *_pd, va_list *list);
void _update_hints_get(Eo *obj, void *_pd, va_list *list);

void _part_external_object_get(Eo *obj, void *_pd, va_list *list);
void _part_external_param_set(Eo *obj, void *_pd, va_list *list);
void _part_external_param_get(Eo *obj, void *_pd, va_list *list);
void _part_external_content_get(Eo *obj, void *_pd, va_list *list);
void _part_external_param_type_get(Eo *obj, void *_pd, va_list *list);

void _file_get(Eo *obj, void *_pd, va_list *list);
void _load_error_get(Eo *obj, void *_pd, va_list *list);

void _message_send(Eo *obj, void *_pd, va_list *list);
void _message_handler_set(Eo *obj, void *_pd, va_list *list);
void _message_signal_process(Eo *obj, void *_pd, va_list *list);

void _signal_callback_add(Eo *obj, void *_pd, va_list *list);
void _signal_callback_del(Eo *obj, void *_pd, va_list *list);
void _signal_callback_del_full(Eo *obj, void *_pd, va_list *list);
void _signal_emit(Eo *obj, void *_pd, va_list *list);
void _play_set(Eo *obj, void *_pd, va_list *list);
void _play_get(Eo *obj, void *_pd, va_list *list);
void _animation_set(Eo *obj, void *_pd, va_list *list);
void _animation_get(Eo *obj, void *_pd, va_list *list);

void edje_signal_init(void);
void edje_signal_shutdown(void);

#ifdef HAVE_LIBREMIX
#include <remix/remix.h>
#endif
#include <Eina.h>

typedef struct _Edje_Multisense_Env  Edje_Multisense_Env;

struct _Edje_Multisense_Env
{
#ifdef HAVE_LIBREMIX
   RemixEnv *remixenv;
#endif
};

typedef Eina_Bool (*MULTISENSE_FACTORY_INIT_FUNC) (Edje_Multisense_Env *);
#ifdef HAVE_LIBREMIX
typedef RemixBase* (*MULTISENSE_SOUND_PLAYER_GET_FUNC) (Edje_Multisense_Env *);
#endif

#endif
