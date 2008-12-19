#include <string.h>

#include <Elementary.h>
#include "elm_priv.h"

#define SMART_NAME "elm_pan"
#define API_ENTRY Smart_Data *sd; sd = evas_object_smart_data_get(obj); if ((!obj) || (!sd) || (evas_object_type_get(obj) && strcmp(evas_object_type_get(obj), SMART_NAME)))
#define INTERNAL_ENTRY Smart_Data *sd; sd = evas_object_smart_data_get(obj); if (!sd) return;
typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{ 
   Evas_Object *smart_obj;
   Evas_Object *child_obj;
   Evas_Coord   x, y, w, h;
   Evas_Coord   child_w, child_h, px, py;
}; 

/* local subsystem functions */
static void _smart_child_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _smart_child_resize_hook(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _smart_reconfigure(Smart_Data *sd);
static void _smart_add(Evas_Object *obj);
static void _smart_del(Evas_Object *obj);
static void _smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *obj);
static void _smart_hide(Evas_Object *obj);
static void _smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object *obj, Evas_Object * clip);
static void _smart_clip_unset(Evas_Object *obj);
static void _smart_init(void);

/* local subsystem globals */
static Evas_Smart *_smart = NULL;

/* externally accessible functions */
Evas_Object *
_elm_smart_pan_add(Evas *evas)
{
   _smart_init();
   return evas_object_smart_add(evas, _smart);
}

void
_elm_smart_pan_child_set(Evas_Object *obj, Evas_Object *child)
{
   API_ENTRY return;
   if (child == sd->child_obj) return;
   if (sd->child_obj)
     {
	evas_object_clip_unset(sd->child_obj);
	evas_object_smart_member_del(sd->child_obj);
        evas_object_event_callback_del(sd->child_obj, EVAS_CALLBACK_FREE, _smart_child_del_hook);
	evas_object_event_callback_del(sd->child_obj, EVAS_CALLBACK_RESIZE, _smart_child_resize_hook);
	sd->child_obj = NULL;
     }
   if (child)
     {
	Evas_Coord w, h;
	int r, g, b, a;
	
	sd->child_obj = child;
	evas_object_smart_member_add(sd->child_obj, sd->smart_obj);
	evas_object_geometry_get(sd->child_obj, NULL, NULL, &w, &h);
	sd->child_w = w;
	sd->child_h = h;
	evas_object_event_callback_add(child, EVAS_CALLBACK_FREE, _smart_child_del_hook, sd);
	evas_object_event_callback_add(child, EVAS_CALLBACK_RESIZE, _smart_child_resize_hook, sd);
	evas_object_color_get(sd->smart_obj, &r, &g, &b, &a);
	evas_object_color_set(sd->child_obj, r, g, b, a);
	evas_object_clip_set(sd->child_obj, evas_object_clip_get(sd->smart_obj));
	if (evas_object_visible_get(sd->smart_obj)) evas_object_show(sd->child_obj);
	else evas_object_hide(sd->child_obj);
	_smart_reconfigure(sd);
     }
   evas_object_smart_callback_call(sd->smart_obj, "changed", NULL);
}

Evas_Object *
_elm_smart_pan_child_get(Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->child_obj;
}

void
_elm_smart_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   API_ENTRY return;
   if (x > (sd->child_w - sd->w)) x = sd->child_w - sd->w;
   if (y > (sd->child_h - sd->h)) y = sd->child_h - sd->h;
   if (x < 0) x = 0;
   if (y < 0) y = 0;
   if ((x == sd->px) && (y == sd->py)) return;
   sd->px = x;
   sd->py = y;
   _smart_reconfigure(sd);
   evas_object_smart_callback_call(sd->smart_obj, "changed", NULL);
}

void
_elm_smart_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   API_ENTRY return;
   if (x) *x = sd->px;
   if (y) *y = sd->py;
}

void
_elm_smart_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   API_ENTRY return;
   if (x)
     {
	if (sd->w < sd->child_w) *x = sd->child_w - sd->w;
	else *x = 0;
     }
   if (y)
     {
	if (sd->h < sd->child_h) *y = sd->child_h - sd->h;
	else *y = 0;
     }
}

void
_elm_smart_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   API_ENTRY return;
   if (w) *w = sd->child_w;
   if (h) *h = sd->child_h;
}

/* local subsystem functions */
static void
_smart_child_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Smart_Data *sd;
   
   sd = data;
   sd->child_obj = NULL;
   evas_object_smart_callback_call(sd->smart_obj, "changed", NULL);
}

static void
_smart_child_resize_hook(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Smart_Data *sd;
   Evas_Coord w, h;
   
   sd = data;
   evas_object_geometry_get(sd->child_obj, NULL, NULL, &w, &h);
   if ((w != sd->child_w) || (h != sd->child_h))
     {
	sd->child_w = w;
	sd->child_h = h;
	_smart_reconfigure(sd);
     }
   evas_object_smart_callback_call(sd->smart_obj, "changed", NULL);
}

static void
_smart_reconfigure(Smart_Data *sd)
{
   evas_object_move(sd->child_obj, sd->x - sd->px, sd->y - sd->py);
}

static void
_smart_add(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   sd->smart_obj = obj;
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   evas_object_smart_data_set(obj, sd);
}

static void
_smart_del(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   _elm_smart_pan_child_set(obj, NULL);
   free(sd);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   INTERNAL_ENTRY;
   sd->x = x;
   sd->y = y;
   _smart_reconfigure(sd);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   INTERNAL_ENTRY;
   sd->w = w;
   sd->h = h;
   _smart_reconfigure(sd);
   evas_object_smart_callback_call(sd->smart_obj, "changed", NULL);
}

static void
_smart_show(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_show(sd->child_obj);
}

static void
_smart_hide(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_hide(sd->child_obj);
}

static void
_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   INTERNAL_ENTRY;
   evas_object_color_set(sd->child_obj, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   INTERNAL_ENTRY;
   evas_object_clip_set(sd->child_obj, clip);
}

static void
_smart_clip_unset(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_clip_unset(sd->child_obj);
}  

/* never need to touch this */

static void
_smart_init(void)
{
   if (_smart) return;
     {
	static const Evas_Smart_Class sc =
	  {
	     SMART_NAME,
	       EVAS_SMART_CLASS_VERSION,
	       _smart_add,
	       _smart_del, 
	       _smart_move,
	       _smart_resize,
	       _smart_show,
	       _smart_hide,
	       _smart_color_set,
	       _smart_clip_set,
	       _smart_clip_unset,
	       NULL,
	       NULL,
	       NULL,
	       NULL
	  };
	_smart = evas_smart_class_new(&sc);
     }
}

