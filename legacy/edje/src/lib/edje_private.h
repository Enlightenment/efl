#ifndef _EDJE_PRIVATE_H
#define _EDJE_PRIVATE_H

#include "config.h"

#include <Evas.h>
#include <Ecore.h>
#include <Eet.h>

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
 * "mouse.down"
 * "mouse.up"
 * "mouse.in"
 * "mouse.out"
 * "mouse.move"
 * "clicked"
 * "drag.start"
 * "drag"
 * "drag.stop"
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
   
   Evas_List                      *collection_loaded;
   
   int                             references;
};

/*----------*/

struct _Edje_Image_Directory
{
   Evas_List *entries; /* a list of Edje_Image_Directory_Entry */
   int        references;
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

struct _Edje
{
   char                 *part;
   int                   layer;
   int                   x, y, w, h;
   unsigned char         dirty : 1;
   Evas                 *evas; /* the evas this edje belongs to */
   Evas_Object          *clipper; /* a big rect to clip this edje to */
   Edje_File            *file; /* the file the data comes form */
   Edje_Part_Collection *collection; /* the description being used */
   Evas_List            *parts; /* private list of parts */
   Evas_List            *actions; /* currently running actions */   
};

struct _Edje_Real_Part
{
   int                       x, y, w, h;
   Evas_Object              *object;
   unsigned char             calculated : 1;
   unsigned char             dirty      : 1;
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
};

typedef struct _Edje_Calc_Params Edje_Calc_Params;

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

#endif
