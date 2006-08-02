#ifndef _EDJE_PRIVATE_H
#define _EDJE_PRIVATE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Evas.h>
#include <Ecore.h>
#ifdef HAVE_ECORE_X
#include <Ecore_X.h>
#endif
#ifdef HAVE_ECORE_FB
#include <Ecore_Fb.h>
#endif
#include <Eet.h>
#include <Embryo.h>

#include <math.h>
#include <fnmatch.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>


#ifdef EAPI
#undef EAPI
#endif
#ifdef WIN32
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

/* increment this when the EET data descriptors have changed and old
 * EETs cannot be loaded/used correctly anymore.
 */
#define EDJE_FILE_VERSION 2

/* FIXME:
 * 
 * More example Edje files
 * 
 * ? programs can do multiple actions from one signal
 * ? add containering (hbox, vbox, table, wrapping multi-line hbox & vbox)
 * ? text entry widget (single line only)
 * ? reduce linked list walking and list_nth calls
 * 
 * ? recursions, unsafe callbacks outside Edje etc. with freeze, ref/unref and block/unblock and break_programs needs to be redesigned & fixed
 * ? all unsafe calls that may result in callbacks must be marked and dealt with
 */


/* Cache result of program glob matches - this uses up extra ram with the gain
 * of faster program matching if a part has LOTS of programs.
#define EDJE_PROGRAM_CACHE
*/

typedef struct _Edje_File                            Edje_File;
typedef struct _Edje_Style                           Edje_Style;
typedef struct _Edje_Style_Tag                       Edje_Style_Tag;
typedef struct _Edje_Data                            Edje_Data;
typedef struct _Edje_Font_Directory                  Edje_Font_Directory;
typedef struct _Edje_Font_Directory_Entry            Edje_Font_Directory_Entry;
typedef struct _Edje_Image_Directory                 Edje_Image_Directory;
typedef struct _Edje_Image_Directory_Entry           Edje_Image_Directory_Entry;
typedef struct _Edje_Spectrum_Directory                 Edje_Spectrum_Directory;
typedef struct _Edje_Spectrum_Directory_Entry           Edje_Spectrum_Directory_Entry;
typedef struct _Edje_Program                         Edje_Program;
typedef struct _Edje_Program_Target                  Edje_Program_Target;
typedef struct _Edje_Program_After                   Edje_Program_After;
typedef struct _Edje_Part_Collection_Directory       Edje_Part_Collection_Directory;
typedef struct _Edje_Part_Collection_Directory_Entry Edje_Part_Collection_Directory_Entry;
typedef struct _Edje_Part_Collection                 Edje_Part_Collection;
typedef struct _Edje_Part                            Edje_Part;
typedef struct _Edje_Part_Image_Id                   Edje_Part_Image_Id;
typedef struct _Edje_Part_Description                Edje_Part_Description;
typedef struct _Edje_Spectrum_Color                  Edje_Spectrum_Color;

#define PI 3.14159265358979323846

#define EDJE_IMAGE_SOURCE_TYPE_NONE           0
#define EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT 1
#define EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY   2
#define EDJE_IMAGE_SOURCE_TYPE_EXTERNAL       3
#define EDJE_IMAGE_SOURCE_TYPE_LAST           4

#define EDJE_PART_TYPE_NONE      0
#define EDJE_PART_TYPE_RECTANGLE 1
#define EDJE_PART_TYPE_TEXT      2
#define EDJE_PART_TYPE_IMAGE     3
#define EDJE_PART_TYPE_SWALLOW   4
#define EDJE_PART_TYPE_TEXTBLOCK 5
#define EDJE_PART_TYPE_GRADIENT  6
#define EDJE_PART_TYPE_LAST      7

#define EDJE_TEXT_EFFECT_NONE                0
#define EDJE_TEXT_EFFECT_PLAIN               1
#define EDJE_TEXT_EFFECT_OUTLINE             2
#define EDJE_TEXT_EFFECT_SOFT_OUTLINE        3
#define EDJE_TEXT_EFFECT_SHADOW              4
#define EDJE_TEXT_EFFECT_SOFT_SHADOW         5
#define EDJE_TEXT_EFFECT_OUTLINE_SHADOW      6
#define EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW 7
#define EDJE_TEXT_EFFECT_LAST                8

#define EDJE_ACTION_TYPE_NONE          0
#define EDJE_ACTION_TYPE_STATE_SET     1
#define EDJE_ACTION_TYPE_ACTION_STOP   2
#define EDJE_ACTION_TYPE_SIGNAL_EMIT   3
#define EDJE_ACTION_TYPE_DRAG_VAL_SET  4
#define EDJE_ACTION_TYPE_DRAG_VAL_STEP 5
#define EDJE_ACTION_TYPE_DRAG_VAL_PAGE 6
#define EDJE_ACTION_TYPE_SCRIPT        7
#define EDJE_ACTION_TYPE_LAST          8

#define EDJE_TWEEN_MODE_NONE       0
#define EDJE_TWEEN_MODE_LINEAR     1
#define EDJE_TWEEN_MODE_SINUSOIDAL 2
#define EDJE_TWEEN_MODE_ACCELERATE 3
#define EDJE_TWEEN_MODE_DECELERATE 4
#define EDJE_TWEEN_MODE_LAST       5

#define EDJE_VAR_NONE   0
#define EDJE_VAR_INT    1
#define EDJE_VAR_FLOAT  2
#define EDJE_VAR_STRING 3
#define EDJE_VAR_LIST   4
#define EDJE_VAR_HASH   5

#define EDJE_ASPECT_PREFER_NONE       0
#define EDJE_ASPECT_PREFER_VERTICAL   1
#define EDJE_ASPECT_PREFER_HORIZONTAL 2
#define EDJE_ASPECT_PREFER_BOTH       3
#define EDJE_VAR_MAGIC_BASE 0x12fe84ba

#define EDJE_STATE_PARAM_NONE         0
#define EDJE_STATE_PARAM_ALIGNMENT    1
#define EDJE_STATE_PARAM_MIN          2
#define EDJE_STATE_PARAM_MAX          3
#define EDJE_STATE_PARAM_STEP         4
#define EDJE_STATE_PARAM_ASPECT       5
#define EDJE_STATE_PARAM_ASPECT_PREF  6
#define EDJE_STATE_PARAM_COLOR        7
#define EDJE_STATE_PARAM_COLOR2       8
#define EDJE_STATE_PARAM_COLOR3       9
#define EDJE_STATE_PARAM_COLOR_CLASS  10
#define EDJE_STATE_PARAM_REL1         11
#define EDJE_STATE_PARAM_REL1_TO      12
#define EDJE_STATE_PARAM_REL1_OFFSET  13
#define EDJE_STATE_PARAM_REL2         14
#define EDJE_STATE_PARAM_REL2_TO      15
#define EDJE_STATE_PARAM_REL2_OFFSET  16
#define EDJE_STATE_PARAM_IMAGE        17
#define EDJE_STATE_PARAM_BORDER       18
#define EDJE_STATE_PARAM_FILL_SMOOTH  19
#define EDJE_STATE_PARAM_FILL_POS     20
#define EDJE_STATE_PARAM_FILL_SIZE    21
#define EDJE_STATE_PARAM_TEXT         22
#define EDJE_STATE_PARAM_TEXT_CLASS   23
#define EDJE_STATE_PARAM_TEXT_FONT    24
#define EDJE_STATE_PARAM_TEXT_STYLE   25
#define EDJE_STATE_PARAM_TEXT_SIZE    26
#define EDJE_STATE_PARAM_TEXT_FIT     27
#define EDJE_STATE_PARAM_TEXT_MIN     28
#define EDJE_STATE_PARAM_TEXT_ALIGN   29
#define EDJE_STATE_PARAM_VISIBLE      30
#define EDJE_STATE_PARAM_LAST         31

/*----------*/

struct _Edje_File
{
   const char                     *path;
   
   Edje_Font_Directory            *font_dir;
   Edje_Image_Directory           *image_dir;
   Edje_Spectrum_Directory        *spectrum_dir;
   Edje_Part_Collection_Directory *collection_dir;
   Evas_List                      *data;
   Evas_List                      *styles;
   Evas_List                      *color_classes;
   
   int                             references;
   char                           *compiler;
   int                             version;
   int                             feature_ver;
   
   Evas_Hash                      *collection_hash;
   Evas_List                      *collection_cache;
};

struct _Edje_Style
{
   char                           *name;
   Evas_List                      *tags;
   Evas_Textblock_Style           *style;
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

struct _Edje_Data
{
   char *key;
   char *value;
};

/*----------*/

struct _Edje_Font_Directory
{
   Evas_List *entries; /* a list of Edje_Font_Directory_Entry */
};

struct _Edje_Font_Directory_Entry
{
   char *entry; /* the name of the font */
};

	
/*----------*/

struct _Edje_Image_Directory
{
   Evas_List *entries; /* a list of Edje_Image_Directory_Entry */
};

struct _Edje_Image_Directory_Entry
{
   char *entry; /* the nominal name of the image - if any */
   int   source_type; /* alternate source mode. 0 = none */
   int   source_param; /* extra params on encoding */
   int   id; /* the id no. of the image */
};

/*----------*/

struct _Edje_Spectrum_Directory
{
  Evas_List *entries; /* a list of Edje_Spectrum_Directory_Entry */
};

struct _Edje_Spectrum_Directory_Entry
{
  char      *entry;
  /* only one of the following two should be included. filename takes precedence */
  char      *filename; /* filename of external spectrum. */
  Evas_List *color_list; /* list of Edje_Spectrum_Color */
  int        id;
};

struct _Edje_Spectrum_Color
{
  int r, g, b, a;
  int d;
};


/*----------*/

struct _Edje_Program /* a conditional program to be run */
{
   int        id; /* id of program */   
   char      *name; /* name of the action */
   
   char      *signal; /* if signal emission name matches the glob here... */
   char      *source; /* if part that emitted this (name) matches this glob */
   
   struct {
      double  from;
      double  range;
   } in;
   
   int        action; /* type - set state, stop action, set drag pos etc. */
   char      *state; /* what state of alternates to apply, NULL = default */
   char      *state2; /* what other state to use - for signal emit action */
   double     value; /* value of state to apply (if multiple names match) */
   double     value2; /* other value for drag actions */
   
   struct {
      int     mode; /* how to tween - linear, sinusoidal etc. */
      double  time; /* time to graduate between current and new state */
   } tween;
   
   Evas_List *targets; /* list of target parts to apply the state to */
   
   Evas_List *after; /* list of actions to run at the end of this, for looping */
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

struct _Edje_Part_Collection_Directory
{
   Evas_List *entries; /* a list of Edje_Part_Collection_Directory_Entry */

   int        references;
};

struct _Edje_Part_Collection_Directory_Entry
{
   char *entry; /* the nominal name of the part collection */
   int   id; /* the id of this named part collection */
};

/*----------*/

struct _Edje_Part_Collection
{
   Evas_List *programs; /* a list of Edje_Program */
   Evas_List *parts; /* a list of Edje_Part */
   Evas_List *data;
   
   int        id; /* the collection id */
   
   struct {
      struct {
	 int w, h;
      } min, max;
   } prop;
   
   int        references;
#ifdef EDJE_PROGRAM_CACHE   
   struct {
      Evas_Hash                   *no_matches;
      Evas_Hash                   *matches;
   } prog_cache;
#endif   
   
   Embryo_Program   *script; /* all the embryo script code for this group */
   
   const char       *part;
};

struct _Edje_Part
{
   char                  *name; /* the name if any of the part */
   int                    id; /* its id number */
   unsigned char          type; /* what type (image, rect, text) */
   unsigned char          effect; /* 0 = plain... */
   unsigned char          mouse_events; /* it will affect/respond to mouse events */
   unsigned char          repeat_events; /* it will repeat events to objects below */
   int                    clip_to_id; /* the part id to clip this one to */   
   Edje_Part_Description *default_desc; /* the part descriptor for default */
   Evas_List             *other_desc; /* other possible descriptors */
   struct {
      signed char         x; /* can u click & drag this bit in x dir */
      signed char         y; /* can u click & drag this bit in y dir */
      
      int                 step_x; /* drag jumps n pixels (0 = no limit) */
      int                 step_y; /* drag jumps n pixels (0 = no limit) */
      
      int                 count_x; /* drag area divided by n (0 = no limit) */
      int                 count_y; /* drag area divided by n (0 = no limit) */
      
      int                 confine_id; /* dragging within this bit, -1 = no */

      /* davinchi */
      int		  events_id; /* If it is used as scrollbar */
   } dragable;
};

struct _Edje_Part_Image_Id
{
   int id;
};

struct _Edje_Part_Description
{
   struct {
      char          *name; /* the named state if any */
      double         value; /* the value of the state (for ranges) */
   } state;
   
   unsigned char     visible; /* is it shown */

   struct {
      double         x, y; /* 0 <-> 1.0 alignment within allocated space */
   } align;
   
   struct {
      unsigned char  w, h; /* width or height is fixed in side (cannot expand with Edje object size) */
   } fixed;
   
   struct {
      int            w, h; /* min & max size, 0 = none */
   } min, max;

   struct {
      int            x, y; /* size stepping by n pixels, 0 = none */
   } step;

   struct {
      double         min, max; /* aspect = w/h */
      unsigned char  prefer; /* NEITHER = 0, VERTICAL = 1, HORIZONTAL = 2 */
   } aspect;
   
   struct {
      double         relative_x;
      double         relative_y;
      int            offset_x;
      int            offset_y;
      int            id_x; /* -1 = whole part collection, or part ID */
      int            id_y; /* -1 = whole part collection, or part ID */
   } rel1, rel2;

   struct {
      int            id; /* the image id to use */   
      Evas_List     *tween_list; /* list of Edje_Part_Image_Id */
   } image;

   struct {
     int            id; /* the spectrum id to use */
     char          *type; /* type of spectrum - 'linear', 'radial', etc */
     char          *params; /* params for spectrum type */
   } gradient;
   
   struct {
      int            l, r, t, b; /* border scaling on image fill */
      unsigned char  no_fill; /* do we fill the center of the image if bordered? 1 == NO!!!! */
   } border;

   struct {
      char           smooth; /* fill with smooth scaling or not */
      double         pos_rel_x; /* fill offset x relative to area */
      int            pos_abs_x; /* fill offset x added to fill offset */
      double         rel_x; /* relative size compared to area */
      int            abs_x; /* size of fill added to relative fill */
      double         pos_rel_y; /* fill offset y relative to area */
      int            pos_abs_y; /* fill offset y added to fill offset */
      double         rel_y; /* relative size compared to area */
      int            abs_y; /* size of fill added to relative fill */
      int            angle; /* angle of fill -- currently only used by grads */
      int            spread; /* spread of fill -- currently only used by grads */
   } fill;
   
   char             *color_class; /* how to modify the color */
   
   struct {
      unsigned char  r, g, b, a; /* color for rect or text, shadow etc. */
   } color, color2, color3;

   struct {
      char          *text; /* if "" or NULL, then leave text unchanged */
      char          *text_class; /* how to apply/modify the font */
      char          *style; /* the text style if a textblock */
      char          *font; /* if a specific font is asked for */
      
      int            size; /* 0 = use user set size */
      
      unsigned char  fit_x; /* resize font size down to fit in x dir */
      unsigned char  fit_y; /* resize font size down to fit in y dir */
      unsigned char  min_x; /* if text size should be part min size */
      unsigned char  min_y; /* if text size should be part min size */
      
      struct {
	 double      x, y; /* text alignment within bounds */
      } align;
      
      int            id_source; /* -1 if none */
      int            id_text_source; /* -1 if none */
      double         elipsis; /* 0.0 - 1.0 defining where the elipsis align */
   } text;
};


/*----------*/


typedef struct _Edje Edje;
typedef struct _Edje_Real_Part Edje_Real_Part;
typedef struct _Edje_Running_Program Edje_Running_Program;
typedef struct _Edje_Signal_Callback Edje_Signal_Callback;
typedef struct _Edje_Calc_Params Edje_Calc_Params;
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

struct _Edje
{
   const char           *path;
   const char           *part;
   
   Evas_Coord            x, y, w, h;
   struct {
      Evas_Coord         w, h;
   } min;
   unsigned short        dirty : 1;
   unsigned short        recalc : 1;
   unsigned short        walking_callbacks : 1;
   unsigned short        delete_callbacks : 1;
   unsigned short        just_added_callbacks : 1;
   unsigned short        have_objects : 1;
   unsigned short        paused : 1;
   unsigned short        no_anim : 1;
   unsigned short        calc_only : 1;
   unsigned short        walking_actions : 1;
   unsigned short        block_break : 1;
   unsigned short        delete_me : 1;
   double                paused_at;
   Evas                 *evas; /* the Evas this Edje belongs to */
   Evas_Object          *obj; /* the smart object */
   Evas_Object          *clipper; /* a big rect to clip this Edje to */
   Edje_File            *file; /* the file the data comes form */
   Edje_Part_Collection *collection; /* the description being used */
   Evas_List            *parts; /* private list of parts */
   Evas_List            *actions; /* currently running actions */   
   Evas_List            *callbacks;
   Evas_List            *pending_actions;
   Evas_List            *color_classes;
   Evas_List            *text_classes;
   int                   references;
   int                   block;
   int                   load_error;
   int                   freeze;
   /* variable pool for Edje Embryo scripts */
   Edje_Var_Pool        *var_pool;
   /* for faster lookups to avoid nth list walks */
   int                   table_parts_size;
   Edje_Real_Part      **table_parts;
   int                   table_programs_size;
   Edje_Program        **table_programs;

   struct {
      void (*func) (void *data, Evas_Object *obj, const char *part);
      void *data;
   } text_change;
   
   int                      processing_messages;
   struct {
      int                   num;
      void                (*func) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg);
      void                 *data;
   } message;
};

struct _Edje_Real_Part
{
   int                       x, y, w, h;
   struct {
      int                    x, y, w, h;
   } req;
   struct {
      int                    x, y;
   } offset;
   Evas_Object              *object;
   Evas_List                *extra_objects;
   Evas_Object              *swallowed_object;
   struct {
      struct {
	 int                 w, h;
      } min, max;
      struct {
	 unsigned char       mode;
	 int                 w, h;
      } aspect;
   } swallow_params;
   unsigned char             calculated;
   unsigned char             calculating;
   int                       clicked_button;
   unsigned char             still_in   : 1;
   Edje_Part                *part;
   struct {
      struct {
	 unsigned count;
	 int  x, y;
      } down;
      struct {
	 int  x, y;
      } tmp;
      double        x, y;
      unsigned char need_reset : 1;
      struct {
	 double x, y;
      } val, size, step, page;
   } drag;
   struct {
      const char            *text;
      const char	    *font;
      const char	    *style;
      int                    size;
      struct {
	 double              in_w, in_h;
	 int                 in_size;
	 const char	    *in_str;
	 const char         *out_str;
	 int                 out_size;
	 double              align_x, align_y;
	 double              elipsis;
	 int                 fit_x, fit_y;
      } cache;
      Edje_Real_Part        *source;
      Edje_Real_Part        *text_source;
   } text;
   double                    description_pos;
   Edje_Part_Description    *chosen_description;
   struct {
      Edje_Part_Description *description;
      Edje_Real_Part        *rel1_to_x;
      Edje_Real_Part        *rel1_to_y;
      Edje_Real_Part        *rel2_to_x;
      Edje_Real_Part        *rel2_to_y;
   } param1, param2, custom;

   Edje_Real_Part           *confine_to;
   Edje_Real_Part           *clip_to;
   
   Edje_Running_Program     *program;
   Edje_Real_Part           *events_to;
};

struct _Edje_Running_Program
{
   Edje           *edje;
   Edje_Program   *program;
   double          start_time;
   char            delete_me : 1;
};

struct _Edje_Signal_Callback
{
   const char	  *signal;
   const char	  *source;
   void (*func) (void *data, Evas_Object *o, const char *emission, const char *source);
   void  *data;
   unsigned char just_added : 1;
   unsigned char delete_me : 1;
};

struct _Edje_Calc_Params
{
   int              x, y, w, h;
   struct {
      int           x, y, w, h;
   } req;
   struct {
      int           x, y, w, h;
   } req_drag;
   unsigned char    visible : 1; 
   unsigned char    smooth : 1;
   struct {
      int           x, y, w, h;
      int           angle;
      int           spread;
   } fill;
   struct {
      unsigned char r, g, b, a;
   } color, color2, color3;
   struct {   
      int           l, r, t, b;
   } border;
   struct {
      struct {
         double      x, y; /* text alignment within bounds */
      } align;
      double         elipsis;
   } text;
   struct {
     int             id;
     char           *type;
   } gradient;
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
   const char   *name;
   const char   *font;
   double        size;
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
   Evas_List *v;
};

struct _Edje_Var_Hash
{
   Evas_Hash *v;
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
   Evas_List   *timers;
   Evas_List   *animators;
   int          size;
   Edje_Var    *vars;
   int          walking_list;
};

struct _Edje_Var
{
   unsigned char type;
   union {
      Edje_Var_Int    i;
      Edje_Var_Float  f;
      Edje_Var_String s;
      Edje_Var_List   l;
      Edje_Var_Hash   h;
   } data;
};


typedef enum _Edje_Queue
{
   EDJE_QUEUE_APP,
     EDJE_QUEUE_SCRIPT
} Edje_Queue;

typedef struct _Edje_Message_Signal Edje_Message_Signal;
typedef struct _Edje_Message        Edje_Message;

struct _Edje_Message_Signal
{
   const char *sig;
   const char *src;
};

struct _Edje_Message
{
   Edje              *edje;
   Edje_Queue         queue;
   Edje_Message_Type  type;
   int                id;
   unsigned char     *msg;
};

EAPI extern Eet_Data_Descriptor *_edje_edd_edje_file;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_style;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_style_tag;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_data;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_font_directory;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_font_directory_entry;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_image_directory;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_image_directory_entry;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_spectrum_directory;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_spectrum_directory_entry;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_program;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_program_target;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory_entry;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_part_collection;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_part;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_part_description;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_part_image_id;
EAPI extern Eet_Data_Descriptor *_edje_edd_edje_spectrum_color;

extern int              _edje_anim_count;
extern Ecore_Animator  *_edje_timer;
extern Evas_List       *_edje_animators;
extern Evas_List       *_edje_edjes;

extern char            *_edje_fontset_append;

void  _edje_part_pos_set(Edje *ed, Edje_Real_Part *ep, int mode, double pos);
Edje_Part_Description *_edje_part_description_find(Edje *ed, Edje_Real_Part *rp, const char *name, double val);
void  _edje_part_description_apply(Edje *ed, Edje_Real_Part *ep, const char  *d1, double v1, const char *d2, double v2);
void  _edje_recalc(Edje *ed);
int   _edje_part_dragable_calc(Edje *ed, Edje_Real_Part *ep, double *x, double *y);
void  _edje_dragable_pos_set(Edje *ed, Edje_Real_Part *ep, double x, double y);
    
void  _edje_mouse_in_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
void  _edje_mouse_out_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
void  _edje_mouse_down_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
void  _edje_mouse_up_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
void  _edje_mouse_move_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
void  _edje_mouse_wheel_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
int   _edje_timer_cb(void *data);
int   _edje_pending_timer_cb(void *data);

void  _edje_edd_setup(void);
void  _edje_edd_free(void);

void  _edje_file_add(Edje *ed);
void  _edje_file_del(Edje *ed);
void  _edje_file_free(Edje_File *edf);
void  _edje_file_cache_shutdown(void);
void  _edje_collection_free(Edje_File *edf, Edje_Part_Collection *ec);

Edje *_edje_add(Evas_Object *obj);
void  _edje_del(Edje *ed);
void  _edje_ref(Edje *ed);
void  _edje_unref(Edje *ed);
void  _edje_clean_objects(Edje *ed);
void  _edje_ref(Edje *ed);
void  _edje_unref(Edje *ed);
    
int   _edje_program_run_iterate(Edje_Running_Program *runp, double tim);
void  _edje_program_end(Edje *ed, Edje_Running_Program *runp);
void  _edje_program_run(Edje *ed, Edje_Program *pr, int force, const char *ssig, const char *ssrc);
void  _edje_emit(Edje *ed, const char *sig, const char *src);
void  _edje_emit_handle(Edje *ed, const char *sig, const char *src);

void  _edje_text_init(void);
void  _edje_text_part_on_add(Edje *ed, Edje_Real_Part *ep);
void  _edje_text_part_on_add_clippers(Edje *ed, Edje_Real_Part *ep);
void  _edje_text_part_on_del(Edje *ed, Edje_Part *ep);
void  _edje_text_real_part_on_del(Edje *ed, Edje_Real_Part *ep);
void  _edje_text_recalc_apply(Edje *ed, Edje_Real_Part *ep, Edje_Calc_Params *params, Edje_Part_Description *chosen_desc);
    
Edje_Real_Part   *_edje_real_part_get(Edje *ed, const char *part);
Edje_Color_Class *_edje_color_class_find(Edje *ed, const char *color_class);
void              _edje_color_class_member_add(Edje *ed, const char *color_class);
void              _edje_color_class_member_del(Edje *ed, const char *color_class);
void              _edje_color_class_on_del(Edje *ed, Edje_Part *ep);
void              _edje_color_class_members_free(void);
void              _edje_color_class_hash_free(void);

Edje_Text_Class  *_edje_text_class_find(Edje *ed, const char *text_class);
void              _edje_text_class_member_add(Edje *ed, const char *text_class);
void              _edje_text_class_member_del(Edje *ed, const char *text_class);
void              _edje_text_class_members_free(void);
void              _edje_text_class_hash_free(void);

Edje             *_edje_fetch(Evas_Object *obj);
int               _edje_glob_match(const char *str, const char *glob);
int               _edje_freeze(Edje *ed);
int               _edje_thaw(Edje *ed);
int               _edje_block(Edje *ed);
int               _edje_unblock(Edje *ed);
int               _edje_block_break(Edje *ed);
void              _edje_block_violate(Edje *ed);    
void              _edje_object_part_swallow_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

void          _edje_embryo_script_init      (Edje *ed);
void          _edje_embryo_script_shutdown  (Edje *ed);
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
void          _edje_message_send            (Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id, void *emsg);
void          _edje_message_process         (Edje_Message *em);
void          _edje_message_queue_process   (void);
void          _edje_message_queue_clear     (void);
void          _edje_message_del             (Edje *ed);

void _edje_textblock_styles_add(Edje *ed);
void _edje_textblock_styles_del(Edje *ed);
void _edje_textblock_style_all_update(Edje *ed);
void _edje_textblock_style_parse_and_fix(Edje_File *edf);
void _edje_textblock_style_cleanup(Edje_File *edf);
Edje_File *_edje_cache_file_coll_open(const char *file, const char *coll, int *error_ret, Edje_Part_Collection **edc_ret);
void _edje_cache_coll_clean(Edje_File *edf);
void _edje_cache_coll_flush(Edje_File *edf);
void _edje_cache_coll_unref(Edje_File *edf, Edje_Part_Collection *edc);
void _edje_cache_file_unref(Edje_File *edf);

void _edje_embryo_globals_init(Edje *ed);

#endif
