#ifndef _EDJE_PRIVATE_H
#define _EDJE_PRIVATE_H

#include "config.h"

#include <Evas.h>
#include <Ecore.h>
#ifndef EDJE_FB_ONLY
#include <Ecore_X.h>
#else
#include <Ecore_Fb.h>
#endif
#include <Eet.h>

#include <math.h>
#include <fnmatch.h>

/* FIXME:
 * 
 * more example edje files
 * 
 * ? programs need to be able to cycle part states given a list of states
 * ? programs can do multiple actions from one signal
 * ? programs need to be able to set/get/add/sub and compare variables
 *   ie: 
 *     action, INT "active_state" = INT 1;
 *     ...
 *     action, INT "hidden" = INT 10;
 *     ...
 *     action, STR "my_string" = STR "some content here";
 *     ...
 *     action, INT "count" += INT 10;
 *     ...
 *     action, INT "count" -= INT 10;
 *     action, INT "count" += MOUSE_X "this_part";
 *     ...
 *     action, INT "count" *= INT "another_variable";
 *     action, INT "count" -= INT 10;
 *     action, INT "count" /= INT 2;
 *     ...
 * 
 *     if,  INT "active_state" == 1;
 *     ...
 *     if,  MOUSE_X     "" > 1;
 *     or,  MOUSE_X     "this_part" > 50;
 *     or,  MOUSE_X_REL "this_part" <= 0.5;
 *     and, STATE       "this_part" == "clicked";
 *     and, STATE_VAL   "this_part" == 0.0;
 *     ...
 * 
 *     if,  INT "active_state" != 0;
 *     and, INT "hidden" < 10;
 *     or,  STR "my_string" == "smelly";
 *     ...
 * 
 * ? add containering (hbox, vbox, table, wrapping multi-line hbox & vbox)
 * ? text entry widget (single line only)
 * ? reduce linked list walking and list_nth calls
 * 
 * ? recursions, unsafe callbacks outside edje etc. with freeze, ref/unref and blobk/unblock and break_programs needs to be redesigned & fixed
 * ? all unsafe calls that may result in callbacks must be marked and dealt with
 */

typedef struct _Edje_File                            Edje_File;
typedef struct _Edje_Data                            Edje_Data;
typedef struct _Edje_Image_Directory                 Edje_Image_Directory;
typedef struct _Edje_Image_Directory_Entry           Edje_Image_Directory_Entry;
typedef struct _Edje_Program                         Edje_Program;
typedef struct _Edje_Program_Target                  Edje_Program_Target;
typedef struct _Edje_Part_Collection_Directory       Edje_Part_Collection_Directory;
typedef struct _Edje_Part_Collection_Directory_Entry Edje_Part_Collection_Directory_Entry;
typedef struct _Edje_Part_Collection                 Edje_Part_Collection;
typedef struct _Edje_Part                            Edje_Part;
typedef struct _Edje_Part_Image_Id                   Edje_Part_Image_Id;
typedef struct _Edje_Part_Description                Edje_Part_Description;

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
#define EDJE_PART_TYPE_LAST      4

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
#define EDJE_ACTION_TYPE_LAST          7

#define EDJE_TWEEN_MODE_NONE       0
#define EDJE_TWEEN_MODE_LINEAR     1
#define EDJE_TWEEN_MODE_SINUSOIDAL 2
#define EDJE_TWEEN_MODE_ACCELERATE 3
#define EDJE_TWEEN_MODE_DECELERATE 4
#define EDJE_TWEEN_MODE_LAST       5

/*----------*/

struct _Edje_File
{
   char                           *path;
   
   Edje_Image_Directory           *image_dir;
   Edje_Part_Collection_Directory *collection_dir;
   Evas_List                      *data;
   
   Evas_Hash                      *collection_hash;
   int                             references;
};

/*----------*/

struct _Edje_Data
{
   char *key;
   char *value;
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
   int   source_param; /* extar params on encoding */
   int   id; /* the id no. of the image */
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
   
   int        after; /* an action id to run at the end of this, for looping */
};

struct _Edje_Program_Target /* the target of an action */
{
   int id; /* just the part id no, or action id no */
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
   
   struct {
      Evas_Hash                   *no_matches;
      Evas_Hash                   *matches;
   } prog_cache;
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
   char                  *text_class; /* how to apply/modify the font */
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
   } dragable;
   int                    load_error;
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
      int            w, h; /* min & max size, 0 = none */
   } min, max;

   struct {
      int            x, y; /* size stepping by n pixels, 0 = none */
   } step;

   struct {
      double         min, max; /* aspect = w/h */
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
      int            l, r, t, b; /* border scaling on image fill */
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
   } fill;
   
   char             *color_class; /* how to modify the color */
   
   struct {
      unsigned char  r, g, b, a; /* color for rect or text, shadow etc. */
   } color, color2, color3;

   struct {
      char          *text; /* if "" or NULL, then leave text unchanged */
      char          *font; /* if a specific font is asked for */
      
      int            size; /* 0 = use user set size */
      
      unsigned char  fit_x; /* resize font size down to fit in x dir */
      unsigned char  fit_y; /* resize font size down to fit in y dir */
      unsigned char  min_x; /* if text size should be part min size */
      unsigned char  min_y; /* if text size should be part min size */
      
      struct {
	 double      x, y; /* text alignment within bounds */
      } align;
   } text;
};

/*----------*/







typedef struct _Edje Edje;
typedef struct _Edje_Real_Part Edje_Real_Part;
typedef struct _Edje_Running_Program Edje_Running_Program;
typedef struct _Edje_Signal_Callback Edje_Signal_Callback;
typedef struct _Edje_Calc_Params Edje_Calc_Params;
typedef struct _Edje_Emission Edje_Emission;
typedef struct _Edje_Pending_Program Edje_Pending_Program;
typedef struct _Edje_Text_Style Edje_Text_Style;
typedef struct _Edje_Color_Class Edje_Color_Class;
typedef struct _Edje_Text_Class Edje_Text_Class;

struct _Edje
{
   char                 *path;
   char                 *part;
   
   int                   layer;
   double                x, y, w, h;
   struct {
      double             w, h;
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
   unsigned short        dont_clear_signals : 1;
   double                paused_at;
   Evas                 *evas; /* the evas this edje belongs to */
   Evas_Object          *obj; /* the smart object */
   Evas_Object          *clipper; /* a big rect to clip this edje to */
   Edje_File            *file; /* the file the data comes form */
   Edje_Part_Collection *collection; /* the description being used */
   Evas_List            *parts; /* private list of parts */
   Evas_List            *actions; /* currently running actions */   
   Evas_List            *callbacks;
   Evas_List            *pending_actions;
   Evas_List            *color_classes;
   Evas_List            *text_classes;
   int                   freeze;
   int                   references;
   int                   block;
   Evas_List            *emissions;
   int                   load_error;
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
   } swallow_params;
   unsigned char             calculated : 1;
   unsigned char             calculating : 1;
   unsigned char             still_in   : 1;
   int                       clicked_button;
   Edje_Part                *part;
   struct {
      struct {
	 unsigned count;
	 int  x, y;
      } down;
      struct {
	 int  x, y;
      } tmp;
      double x, y;
      char need_reset : 1;
      struct {
	 double x, y;
      } val, size, step, page;
   } drag;
   struct {
      char                  *text;
      char                  *font;
      int                    size;
      struct {
	 double              in_w, in_h;
	 int                 in_size;
	 char               *in_str;
	 char               *out_str;
	 int                 out_size;
      } cache;
   } text;
   double                    description_pos;
   Edje_Part_Description    *chosen_description;
   struct {
      Edje_Part_Description *description;
      Edje_Real_Part        *rel1_to_x;
      Edje_Real_Part        *rel1_to_y;
      Edje_Real_Part        *rel2_to_x;
      Edje_Real_Part        *rel2_to_y;
   } param1, param2;

   Edje_Real_Part           *confine_to;
   Edje_Real_Part           *clip_to;
   
   Edje_Running_Program     *program;
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
   char  *signal;
   char  *source;
   void (*func) (void *data, Evas_Object *o, const char *emission, const char *source);
   void  *data;
   int    just_added : 1;
   int    delete_me : 1;
};

struct _Edje_Calc_Params
{
   double           x, y, w, h;
   struct {
      double        x, y, w, h;
   } req;
   struct {
      double        x, y, w, h;
   } req_drag;
   char             visible : 1; 
   char             smooth : 1;
   struct {
      double        x, y, w, h;
   } fill;
   struct {
      unsigned char r, g, b, a;
   } color, color2, color3;
   struct {   
      int           l, r, t, b;
   } border;
};

struct _Edje_Emission
{
   char *signal;
   char *source;
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
      char          x, y; /* offset */
      unsigned char alpha;
   } members[32];
};

struct _Edje_Color_Class
{
   char          *name;
   unsigned char  r, g, b, a;
   unsigned char  r2, g2, b2, a2;
   unsigned char  r3, g3, b3, a3;
};

struct _Edje_Text_Class
{
   char   *name;
   char   *font;
   double  size;
};

void  _edje_part_pos_set(Edje *ed, Edje_Real_Part *ep, int mode, double pos);
void  _edje_part_description_apply(Edje *ed, Edje_Real_Part *ep, char  *d1, double v1, char *d2, double v2);
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

void  _edje_file_add(Edje *ed);
void  _edje_file_del(Edje *ed);
void  _edje_file_free(Edje_File *edf);
void  _edje_collection_free(Edje_Part_Collection *ec);

Edje *_edje_add(Evas_Object *obj);
void  _edje_del(Edje *ed);
void  _edje_clean_objects(Edje *ed);
void  _edje_ref(Edje *ed);
void  _edje_unref(Edje *ed);
    
int   _edje_program_run_iterate(Edje_Running_Program *runp, double tim);
void  _edje_program_end(Edje *ed, Edje_Running_Program *runp);
void  _edje_program_run(Edje *ed, Edje_Program *pr, int force);
void  _edje_emit(Edje *ed, char *sig, char *src);

void  _edje_text_init(void);
void  _edje_text_part_on_add(Edje *ed, Edje_Real_Part *ep);
void  _edje_text_part_on_add_clippers(Edje *ed, Edje_Real_Part *ep);
void  _edje_text_part_on_del(Edje *ed, Edje_Real_Part *ep);
void  _edje_text_recalc_apply(Edje *ed, Edje_Real_Part *ep, Edje_Calc_Params *params, Edje_Part_Description *chosen_desc);
    
Edje_Real_Part   *_edje_real_part_get(Edje *ed, char *part);
Edje_Color_Class *_edje_color_class_find(Edje *ed, char *color_class);
Edje_Text_Class  *_edje_text_class_find(Edje *ed, char *text_class);
Edje             *_edje_fetch(Evas_Object *obj);
int               _edje_glob_match(char *str, char *glob);
int               _edje_freeze(Edje *ed);
int               _edje_thaw(Edje *ed);
int               _edje_block(Edje *ed);
int               _edje_unblock(Edje *ed);
int               _edje_block_break(Edje *ed);
void              _edje_block_violate(Edje *ed);    
void              _edje_object_part_swallow_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);


extern Eet_Data_Descriptor *_edje_edd_edje_file;
extern Eet_Data_Descriptor *_edje_edd_edje_data;
extern Eet_Data_Descriptor *_edje_edd_edje_image_directory;
extern Eet_Data_Descriptor *_edje_edd_edje_image_directory_entry;
extern Eet_Data_Descriptor *_edje_edd_edje_program;
extern Eet_Data_Descriptor *_edje_edd_edje_program_target;
extern Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory;
extern Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory_entry;
extern Eet_Data_Descriptor *_edje_edd_edje_part_collection;
extern Eet_Data_Descriptor *_edje_edd_edje_part;
extern Eet_Data_Descriptor *_edje_edd_edje_part_description;
extern Eet_Data_Descriptor *_edje_edd_edje_part_image_id;

extern int              _edje_anim_count;
extern Ecore_Timer     *_edje_timer;
extern Evas_List       *_edje_animators;
extern Edje_Text_Style  _edje_text_styles[EDJE_TEXT_EFFECT_LAST];
extern Evas_List       *_edje_edjes;

#endif
