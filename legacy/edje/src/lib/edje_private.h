#ifndef _EDJE_PRIVATE_H
#define _EDJE_PRIVATE_H

#include "config.h"

#include <Evas.h>
#include <Ecore.h>
#include <Eet.h>

#include <math.h>
#include <fnmatch.h>

/* FIXME: need "random" signals and events to hook to */
/* FIXME: clip_to needs to work */
/* FIXME: free stuff - no more leaks */
/* FIXME: dragables have to work */
/* FIXME: drag start/top signals etc. */
/* FIXME: drag needs to have signals with relative pos */
/* FIXME: drag vals 0.0 -> 1.0. "rest" pos == 0.0 */
/* FIXME: text parts need to work */
/* FIXME: reduce linked list walking and list_nth calls */
/* FIXME: named parts need to be able to be "replaced" with new evas objects */
/* FIXME: need to be able to calculate min & max size of a whole edje */
/* FIXME: add code to list collections in an eet */
/* FIXME: part replacement with objec t+callbacks */
/* FIXME: part queries for geometry etc. */

/* FIXME: ? somehow handle double click? */
/* FIXME: ? add numeric params to conditions for progs (ranges etc.) */

  
/* HOW THIS ALL WORKS:
 * -------------------
 * 
 * A part file contains:
 * a list of images stored
 * the images
 * a list of part collections
 * the part collections
 * 
 * A part collection contains:
 * a list of programs
 * a list of actions
 * a list of parts
 * 
 * when a signal is emitted and matches the conditionals of a program a
 * specific action is started. that action may affect one or more parts in
 * a part collection and may operate over a period of time. it may also
 * spawn another action when it is done (able to loop!) over a period of
 * time. A part is set into a certain state and the calculated outputs of the
 * staret and end state are mixed based on the kind of tweening and time. a
 * tween time of 0.0 = do it right away. an image_id list for tweening is
 * also able ot be provided and shoudl be run through over time during the
 * tween to the new state.
 * 
 * the signals that can be emitted are:
 * "mouse,down"
 * "mouse,up"
 * "mouse,in"
 * "mouse,out"
 * "mouse,move"
 * "drag,start"
 * "drag"
 * "drag,stop"
 */

typedef struct _Edje_File                            Edje_File;
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

#define EDJE_TEXT_EFFECT_NONE           0
#define EDJE_TEXT_EFFECT_PLAIN          1
#define EDJE_TEXT_EFFECT_OUTLINE        2
#define EDJE_TEXT_EFFECT_SHADOW         3
#define EDJE_TEXT_EFFECT_OUTLINE_SHADOW 4
#define EDJE_TEXT_EFFECT_LAST           5

#define EDJE_ACTION_TYPE_NONE        0
#define EDJE_ACTION_TYPE_STATE_SET   1
#define EDJE_ACTION_TYPE_ACTION_STOP 2
#define EDJE_ACTION_TYPE_SIGNAL_EMIT 3
#define EDJE_ACTION_TYPE_LAST        4

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
   
   Evas_Hash                      *collection_hash;
   int                             references;
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
   
   int        action; /* type - set state, stop action, set drag pos etc. */
   char      *state; /* what state of alternates to apply, NULL = default */
   char      *state2; /* what other state to use - for signal emit action */
   double     value; /* value of state to apply (if multiple names match) */
   
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
   
   int        id; /* the collection id */
   
   int        references;
};

struct _Edje_Part
{
   char                  *name; /* the name if any of the part */
   unsigned char          type; /* what type (image, rect, text) */
   unsigned char          mouse_events; /* it will affect/respond to mouse events */
   int                    id; /* its id number */
   char                  *color_class; /* how to modify the color */
   char                  *text_class; /* how to apply/modify the font */
   Edje_Part_Description *default_desc; /* the part descriptor for default */
   Evas_List             *other_desc; /* other possible descriptors */
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
      char           x; /* can u click & draqg this bit & which dir */
      int            step_x; /* drag jumps n pixels (0 = no limit) */
      int            count_x; /* drag area divided by n (0 = no limit) */
      
      char           y; /* can u click & drag this bit & which dir */
      int            step_y; /* drag jumps n pixels (0 = no limit) */
      int            count_y; /* drag area divided by n (0 = no limit) */
      
      int            confine_id; /* dragging within this bit, -1 = no */
   } dragable;
   
   int               clip_to_id; /* the part id to clip this one to */
   
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
      int            id; /* -1 = whole part collection, or part ID */
   } rel1, rel2;

   struct {
      int            id; /* the image id to use */   
      Evas_List     *tween_list; /* list of Edje_Part_Image_Id */
   } image;
   
   struct {
      int            l, r, t, b; /* border scaling on image fill */
   } border;

   struct {
      double         pos_rel_x; /* fill offset x relative to area */
      int            pos_abs_x; /* fill offset x added to fill offset */
      double         rel_x; /* relative size compared to area */
      int            abs_x; /* size of fill added to relative fill */
      double         pos_rel_y; /* fill offset y relative to area */
      int            pos_abs_y; /* fill offset y added to fill offset */
      double         rel_y; /* relative size compared to area */
      int            abs_y; /* size of fill added to relative fill */
   } fill;
   
   struct {
      unsigned char  r, g, b, a; /* color for rect or text, shadow etc. */
   } color, color2, color3;

   struct {
      char          *text; /* if "" or NULL, then leave text unchanged */
      char          *font; /* if a specific font is asked for */
      
      int            size; /* 0 = use user set size */
      
      unsigned char  effect; /* 0 = plain... */
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

struct _Edje
{
   char                 *path;
   char                 *part;
   
   int                   layer;
   double                x, y, w, h;
   unsigned char         dirty : 1;
   unsigned char         recalc : 1;
   Evas                 *evas; /* the evas this edje belongs to */
   Evas_Object          *obj; /* the smart object */
   Evas_Object          *clipper; /* a big rect to clip this edje to */
   Edje_File            *file; /* the file the data comes form */
   Edje_Part_Collection *collection; /* the description being used */
   Evas_List            *parts; /* private list of parts */
   Evas_List            *actions; /* currently running actions */   
   Evas_List            *callbacks;
   int                   freeze;
};

struct _Edje_Real_Part
{
   int                       x, y, w, h;
   Evas_Object              *object;
   unsigned char             calculated : 1;
   unsigned char             dirty      : 1;
   unsigned char             still_in   : 1;
   int                       clicked_button;
   Edje_Part                *part;
   struct {
      int x, y;
   } drag;
   struct {
      char                  *text;
      char                  *font;
      int                    size;
   } text;
   double                    description_pos;
   struct {
      Edje_Part_Description *description;
      Edje_Real_Part        *rel1_to;
      Edje_Real_Part        *rel2_to;
      Edje_Real_Part        *confine_to;
   } param1, param2;
   
   Edje_Running_Program     *program;
};

struct _Edje_Running_Program
{
   Edje           *edje;
   Edje_Program   *program;
   double          start_time;
};

struct _Edje_Signal_Callback
{
   char  *signal;
   char  *source;
   void (*func) (void *data, Evas_Object *o, const char *emission, const char *source);
   void  *data;
};

struct _Edje_Calc_Params
{
   double           x, y, w, h;
   char             visible : 1; 
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

void  _edje_part_pos_set(Edje *ed, Edje_Real_Part *ep, int mode, double pos);
void  _edje_part_description_apply(Edje *ed, Edje_Real_Part *ep, char  *d1, double v1, char *d2, double v2);
void  _edje_recalc(Edje *ed);

void  _edje_mouse_in_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
void  _edje_mouse_out_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
void  _edje_mouse_down_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
void  _edje_mouse_up_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
void  _edje_mouse_move_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
void  _edje_mouse_wheel_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
int   _edje_timer_cb(void *data);

void  _edje_edd_setup(void);

void  _edje_file_add(Edje *ed);
void  _edje_file_del(Edje *ed);
void  _edje_file_free(Edje_File *edf);
void  _edje_collection_free(Edje_Part_Collection *ec);

Edje *_edje_add(Evas_Object *obj);
void  _edje_del(Edje *ed);

int   _edje_program_run_iterate(Edje_Running_Program *runp, double tim);
void  _edje_program_end(Edje *ed, Edje_Running_Program *runp);
void  _edje_program_run(Edje *ed, Edje_Program *pr);
void  _edje_emit(Edje *ed, char *sig, char *src);

Edje *_edje_fetch(Evas_Object *obj);
int   _edje_glob_match(char *str, char *glob);
int   _edje_freeze(Edje *ed);
int   _edje_thaw(Edje *ed);


extern Eet_Data_Descriptor *_edje_edd_edje_file;
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

extern int          _edje_anim_count;
extern Ecore_Timer *_edje_timer;
extern Evas_List   *_edje_animators;

#endif
