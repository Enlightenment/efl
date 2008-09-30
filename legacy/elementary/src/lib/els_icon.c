#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{ 
   Evas_Coord   x, y, w, h;
   Evas_Object *obj;
   int          size;
   double       scale;
   unsigned char fill_inside : 1;
   unsigned char scale_up : 1;
   unsigned char scale_down : 1;
}; 

/* local subsystem functions */
static void _smart_reconfigure(Smart_Data *sd);
static void _smart_init(void);
static void _smart_add(Evas_Object *obj);
static void _smart_del(Evas_Object *obj);
static void _smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *obj);
static void _smart_hide(Evas_Object *obj);
static void _smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object *obj, Evas_Object * clip);
static void _smart_clip_unset(Evas_Object *obj);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
Evas_Object *
_els_smart_icon_add(Evas *evas)
{
   _smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

void
_els_smart_icon_file_key_set(Evas_Object *obj, const char *file, const char *key)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* smart code here */
   if (sd->size != 0)
     evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
   evas_object_image_file_set(sd->obj, file, key);
   _smart_reconfigure(sd);
}

void
_els_smart_icon_file_edje_set(Evas_Object *obj, const char *file, const char *part)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* smart code here */
   if (sd->obj) evas_object_del(sd->obj);
   sd->obj = edje_object_add(evas_object_evas_get(obj));
   edje_object_file_set(sd->obj, file, part);
   evas_object_smart_member_add(sd->obj, obj);
   _smart_reconfigure(sd);
}

void
_els_smart_icon_smooth_scale_set(Evas_Object *obj, int smooth)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     return;
   evas_object_image_smooth_scale_set(sd->obj, smooth);
}

void
_els_smart_icon_size_get(Evas_Object *obj, int *w, int *h)
{
   Smart_Data *sd;
   int tw, th;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_image_size_get(sd->obj, &tw, &th);
   tw = ((double)tw) * sd->scale;
   th = ((double)th) * sd->scale;
   if (w) *w = tw;
   if (h) *h = th;
}

void
_els_smart_icon_fill_inside_set(Evas_Object *obj, int fill_inside)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (((sd->fill_inside) && (fill_inside)) ||
       ((!sd->fill_inside) && (!fill_inside))) return;
   sd->fill_inside = fill_inside;
   _smart_reconfigure(sd);
}

void
_els_smart_icon_scale_up_set(Evas_Object *obj, int scale_up)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (((sd->scale_up) && (scale_up)) ||
       ((!sd->scale_up) && (!scale_up))) return;
   sd->scale_up = scale_up;
   _smart_reconfigure(sd);
}

void
_els_smart_icon_scale_down_set(Evas_Object *obj, int scale_down)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (((sd->scale_down) && (scale_down)) ||
       ((!sd->scale_down) && (!scale_down))) return;
   sd->scale_down = scale_down;
   _smart_reconfigure(sd);
}

void
_els_smart_icon_scale_size_set(Evas_Object *obj, int size)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->size = size;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     return;   
   evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
}

void
_els_smart_icon_scale_set(Evas_Object *obj, double scale)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->scale = scale;
   _smart_reconfigure(sd);
}

/* local subsystem globals */
static void
_smart_reconfigure(Smart_Data *sd)
{
   int iw, ih;
   Evas_Coord x, y, w, h;
   
   if (!sd->obj) return;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     {
	w = sd->w;
	h = sd->h;
	x = sd->x;
	y = sd->y;
	evas_object_move(sd->obj, x, y);
	evas_object_resize(sd->obj, w, h);
     }
   else
     {
	ih = 0;
	ih = 0;
	evas_object_image_size_get(sd->obj, &iw, &ih);
	
	iw = ((double)iw) * sd->scale;
	ih = ((double)ih) * sd->scale;
	
	if (iw < 1) iw = 1;
	if (ih < 1) ih = 1;
	
	if (sd->fill_inside)
	  {
	     w = sd->w;
	     h = ((double)ih * w) / (double)iw;
	     if (h > sd->h)
	       {
		  h = sd->h;
		  w = ((double)iw * h) / (double)ih;
	       }
	  }
	else
	  {
	     w = sd->w;
	     h = ((double)ih * w) / (double)iw;
	     if (h < sd->h)
	       {
		  h = sd->h;
		  w = ((double)iw * h) / (double)ih;
	       }	
	  }
	if (!sd->scale_up)
	  {
	     if ((w > iw) || (h > ih))
	       {
		  w = iw;
		  h = ih;
	       }
	  }
	if (!sd->scale_down)
	  {
	     if ((w < iw) || (h < ih))
	       {
		  w = iw;
		  h = ih;
	       }
	  }
	x = sd->x + ((sd->w - w) / 2);
	y = sd->y + ((sd->h - h) / 2);
	evas_object_move(sd->obj, x, y);
	evas_object_image_fill_set(sd->obj, 0, 0, w, h);
	evas_object_resize(sd->obj, w, h);
     }
}

static void
_smart_init(void)
{
   if (_e_smart) return;
     {
	static const Evas_Smart_Class sc =
	  {
	     "e_icon",
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
	       NULL
	  };
	_e_smart = evas_smart_class_new(&sc);
     }
}

static void
_smart_add(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   sd->obj = evas_object_image_add(evas_object_evas_get(obj));
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->fill_inside = 1;
   sd->scale_up = 1;
   sd->scale_down = 1;
   sd->size = 64;
   sd->scale = 1.0;
   evas_object_smart_member_add(sd->obj, obj);
   evas_object_smart_data_set(obj, sd);
}
   
static void
_smart_del(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_del(sd->obj);
   free(sd);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((sd->x == x) && (sd->y == y)) return;
   sd->x = x;
   sd->y = y;
   _smart_reconfigure(sd);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((sd->w == w) && (sd->h == h)) return;
   sd->w = w;
   sd->h = h;
   _smart_reconfigure(sd);
}

static void
_smart_show(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_show(sd->obj);
}

static void
_smart_hide(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_hide(sd->obj);
}

static void
_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_color_set(sd->obj, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object *obj, Evas_Object * clip)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_set(sd->obj, clip);
}

static void
_smart_clip_unset(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_unset(sd->obj);
}  
