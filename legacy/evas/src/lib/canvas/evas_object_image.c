#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

/* private magic number for image objects */
static const char o_type[] = "image";

/* private struct for rectangle object internal data */
typedef struct _Evas_Object_Image      Evas_Object_Image;

struct _Evas_Object_Image
{
   DATA32            magic;
   
   struct {
      struct {
	 double      x, y, w, h;
      } fill;
      
      struct {
	 int         w, h;
      } image;

      struct {
	 int         l, r, t, b;
      } border;

      char          *file;
      char          *key;
      
      int            smooth_scale : 1;
      int            has_alpha :1;
   } cur, prev;
   
   int               changed : 1;
   
   int               load_error;
   Evas_List        *pixel_updates;
   
   void             *engine_data;
};

/* private methods for image objects */
static void evas_object_image_unload(Evas_Object *obj);
static void evas_object_image_load(Evas_Object *obj);
static double evas_object_image_figure_x_fill(Evas_Object *obj, double start, double size, double *size_ret);
static double evas_object_image_figure_y_fill(Evas_Object *obj, double start, double size, double *size_ret);

static void evas_object_image_init(Evas_Object *obj);
static void *evas_object_image_new(void);
static void evas_object_image_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_image_free(Evas_Object *obj);
static void evas_object_image_render_pre(Evas_Object *obj);
static void evas_object_image_render_post(Evas_Object *obj);

static int evas_object_image_is_opaque(Evas_Object *obj);
static int evas_object_image_was_opaque(Evas_Object *obj);

static Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_image_free,
     evas_object_image_render,
     evas_object_image_render_pre,
     evas_object_image_render_post,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_image_is_opaque,
     evas_object_image_was_opaque,
     NULL,
     NULL,
     NULL
};


Evas_Object *
evas_object_image_add(Evas *e)
{
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new();
   evas_object_image_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

void
evas_object_image_file_set(Evas_Object *obj, const char *file, const char *key)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.file) && (file) && (!strcmp(o->cur.file, file)))
     {
	if ((o->cur.key) && (key) && (!strcmp(o->cur.key, key)))
	  return;
	if ((!o->cur.key) && (!key))
	  return;
     }
   if (o->cur.file) free(o->cur.file);
   if (o->cur.key) free(o->cur.key);
   if (file) o->cur.file = strdup(file);
   else o->cur.file = NULL;
   if (key) o->cur.key = strdup(key);
   else o->cur.key = NULL;
   o->prev.file = NULL;
   o->prev.key = NULL;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
					       o->engine_data);
   o->load_error = EVAS_LOAD_ERROR_NONE;
   o->cur.has_alpha = 1;
   o->cur.image.w = 0;
   o->cur.image.h = 0;
   o->engine_data = obj->layer->evas->engine.func->image_load(obj->layer->evas->engine.data.output,
							      o->cur.file,
							      o->cur.key,
							      &o->load_error);
   if (o->engine_data)
     {
	obj->layer->evas->engine.func->image_size_get(obj->layer->evas->engine.data.output,
						      o->engine_data,
						      &(o->cur.image.w),
						      &(o->cur.image.h));
	o->cur.has_alpha = obj->layer->evas->engine.func->image_alpha_get(obj->layer->evas->engine.data.output,
									  o->engine_data);
     }
   else
     {
	o->load_error = EVAS_LOAD_ERROR_GENERIC;
     }
   o->changed = 1;
   evas_object_change(obj);
}

void
evas_object_image_file_get(Evas_Object *obj, char **file, char **key)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (file) *file = NULL;
   if (key) *key = NULL;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (file) *file = NULL;
   if (key) *key = NULL;
   return;
   MAGIC_CHECK_END();
   if (file) *file = o->cur.file;
   if (key) *key = o->cur.key;
}

void
evas_object_image_border_set(Evas_Object *obj, int l, int r, int t, int b)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (l < 0) l = 0;
   if (r < 0) r = 0;
   if (t < 0) t = 0;
   if (b < 0) b = 0;
   if ((o->cur.border.l == l) &&
       (o->cur.border.r == r) &&
       (o->cur.border.t == t) &&
       (o->cur.border.b == b)) return;
   o->cur.border.l = l;
   o->cur.border.r = r;
   o->cur.border.t = t;
   o->cur.border.b = b;
   o->changed = 1;
   evas_object_change(obj);   
}

void
evas_object_image_border_get(Evas_Object *obj, int *l, int *r, int *t, int *b)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   if (l) *l = o->cur.border.l;
   if (r) *r = o->cur.border.r;
   if (t) *t = o->cur.border.t;
   if (b) *b = o->cur.border.b;
}

void
evas_object_image_fill_set(Evas_Object *obj, double x, double y, double w, double h)
{
   Evas_Object_Image *o;
   
   if (w < 0) w = -w;
   if (h < 0) h = -h;
   if (w == 0.0) return;
   if (h == 0.0) return;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.fill.x == x) &&
       (o->cur.fill.y == y) &&
       (o->cur.fill.w == w) &&
       (o->cur.fill.h == h)) return;
   o->cur.fill.x = x;
   o->cur.fill.y = y;
   o->cur.fill.w = w;
   o->cur.fill.h = h;
   o->changed = 1;
   evas_object_change(obj);   
}

void
evas_object_image_fill_get(Evas_Object *obj, double *x, double *y, double *w, double *h)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (x) *x = o->cur.fill.x;
   if (y) *y = o->cur.fill.y;
   if (w) *w = o->cur.fill.w;
   if (h) *h = o->cur.fill.h;
}

void
evas_object_image_size_set(Evas_Object *obj, int w, int h)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if (w > 32768) return;
   if (h > 32768) return;
   if ((w == o->cur.image.w) &&
       (h == o->cur.image.h)) return;
   o->cur.image.w = w;
   o->cur.image.h = h;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_size_set(obj->layer->evas->engine.data.output,
								    o->engine_data,
								    w, h);
   else
     o->engine_data = obj->layer->evas->engine.func->image_new_from_copied_data(obj->layer->evas->engine.data.output,
										w, h, NULL);
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
   if (o->cur.file)  {free(o->cur.file); o->cur.file = NULL;}
   if (o->cur.key)   {free(o->cur.key); o->cur.key = NULL;}
   if (o->prev.file) {free(o->prev.file); o->prev.file = NULL;}
   if (o->prev.key)  {free(o->prev.key); o->prev.key = NULL;}
   o->changed = 1;
   evas_object_change(obj);   
}

void
evas_object_image_size_get(Evas_Object *obj, int *w, int *h)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (w) *w = o->cur.image.w;
   if (h) *h = o->cur.image.h;
}

int
evas_object_image_load_error_get(Evas_Object *obj)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->load_error;
}

void
evas_object_image_data_set(Evas_Object *obj, int *data)
{
   Evas_Object_Image *o;
   void *p_data;
   
   if (!data) return;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   p_data = o->engine_data;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
								    o->engine_data, 
								    data);
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
   if (p_data != o->engine_data)
     {
	if (o->cur.file)  {free(o->cur.file); o->cur.file = NULL;}
	if (o->cur.key)   {free(o->cur.key); o->cur.key = NULL;}
	if (o->prev.file) {free(o->prev.file); o->prev.file = NULL;}
	if (o->prev.key)  {free(o->prev.key); o->prev.key = NULL;}
     }
   o->changed = 1;
   evas_object_change(obj);   
}

int *
evas_object_image_data_get(Evas_Object *obj, int for_writing)
{
   Evas_Object_Image *o;
   DATA32 *data;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();
   if (!o->engine_data) return NULL;
   data = NULL;
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  for_writing, 
								  &data);
   if (o->cur.file)  {free(o->cur.file); o->cur.file = NULL;}
   if (o->cur.key)   {free(o->cur.key); o->cur.key = NULL;}
   if (o->prev.file) {free(o->prev.file); o->prev.file = NULL;}
   if (o->prev.key)  {free(o->prev.key); o->prev.key = NULL;}
   
   return data;
}

void
evas_object_image_data_copy_set(Evas_Object *obj, int *data)
{
   Evas_Object_Image *o;
   
   if (!data) return;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.image.w <= 0) ||
       (o->cur.image.h <= 0)) return;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
					       o->engine_data);
   o->engine_data = obj->layer->evas->engine.func->image_new_from_copied_data(obj->layer->evas->engine.data.output,
									      o->cur.image.w,
									      o->cur.image.h,
									      data);
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
   if (o->cur.file)  {free(o->cur.file); o->cur.file = NULL;}
   if (o->cur.key)   {free(o->cur.key); o->cur.key = NULL;}
   if (o->prev.file) {free(o->prev.file); o->prev.file = NULL;}
   if (o->prev.key)  {free(o->prev.key); o->prev.key = NULL;}
}

void
evas_object_image_data_update_add(Evas_Object *obj, int x, int y, int w, int h)
{
   Evas_Object_Image *o;
   Evas_Rectangle *r;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, o->cur.image.w, o->cur.image.h);
   if ((w <= 0)  || (h <= 0)) return;
   NEW_RECT(r, x, y, w, h);
   if (r) o->pixel_updates = evas_list_append(o->pixel_updates, r);
   o->changed = 1;
   evas_object_change(obj);   
}

void
evas_object_image_alpha_set(Evas_Object *obj, int has_alpha)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (((has_alpha) && (o->cur.has_alpha)) ||
       ((!has_alpha) && (!o->cur.has_alpha)))
     return;
   o->cur.has_alpha = has_alpha;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
   evas_object_image_data_update_add(obj, 0, 0, o->cur.image.w, o->cur.image.h);
   if (o->cur.file)  {free(o->cur.file); o->cur.file = NULL;}
   if (o->cur.key)   {free(o->cur.key); o->cur.key = NULL;}
   if (o->prev.file) {free(o->prev.file); o->prev.file = NULL;}
   if (o->prev.key)  {free(o->prev.key); o->prev.key = NULL;}
}

int
evas_object_image_alpha_get(Evas_Object *obj)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.has_alpha;
}

void
evas_object_image_smooth_scale_set(Evas_Object *obj, int smooth_scale)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();   
   if (((smooth_scale) && (o->cur.smooth_scale)) ||
       ((!smooth_scale) && (!o->cur.smooth_scale)))
     return;
   o->cur.smooth_scale = smooth_scale;
   evas_object_image_data_update_add(obj, 0, 0, o->cur.image.w, o->cur.image.h);
}

int
evas_object_image_smooth_scale_get(Evas_Object *obj)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();   
   return o->cur.smooth_scale;
}

void
evas_object_image_reload(Evas_Object *obj)
{
   Evas_Object_Image *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   evas_object_image_unload(obj);
   evas_image_cache_flush(obj->layer->evas);
   evas_object_image_load(obj);
   o->changed = 1;
   evas_object_change(obj);
}
    


void
evas_image_cache_flush(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   
   e->engine.func->image_cache_flush(e->engine.data.output);   
}

void
evas_image_cache_reload(Evas *e)
{
   Evas_Object_List *l;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   evas_image_cache_flush(e);
   for (l = (Evas_Object_List *)e->layers; l; l = l->next)
     {
	Evas_Layer *layer;
	Evas_Object_List *l2;
	
	layer = (Evas_Layer *)l;
        for (l2 = (Evas_Object_List *)layer->objects; l2; l2 = l2->next)
	  {
	     Evas_Object *obj;
	     Evas_Object_Image *o;
	     
	     obj = (Evas_Object *)l2;
	     o = (Evas_Object_Image *)(obj->object_data);
	     if (o->magic == MAGIC_OBJ_IMAGE) 
	       {
		  evas_object_image_unload(obj);
	       }
	  }
     }
   evas_image_cache_flush(e);
   for (l = (Evas_Object_List *)e->layers; l; l = l->next)
     {
	Evas_Layer *layer;
	Evas_Object_List *l2;
	
	layer = (Evas_Layer *)l;
        for (l2 = (Evas_Object_List *)layer->objects; l2; l2 = l2->next)
	  {
	     Evas_Object *obj;
	     Evas_Object_Image *o;
	     
	     obj = (Evas_Object *)l2;
	     o = (Evas_Object_Image *)(obj->object_data);
	     if (o->magic == MAGIC_OBJ_IMAGE) 
	       {
		  evas_object_image_load(obj);
		  o->changed = 1;
		  evas_object_change(obj);		  
	       }
	  }
     }
   evas_image_cache_flush(e);
}

void
evas_image_cache_set(Evas *e, int size)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   
   if (size < 0) size = 0;
   e->engine.func->image_cache_set(e->engine.data.output, size);
}

int
evas_image_cache_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   
   return e->engine.func->image_cache_get(e->engine.data.output);
}









/* all nice and private */

static void
evas_object_image_unload(Evas_Object *obj)
{
   Evas_Object_Image *o;
   
   o = (Evas_Object_Image *)(obj->object_data);
   
   if (!o->cur.file) return;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output,
									o->engine_data,
									0, 0, 
									o->cur.image.w, o->cur.image.h);	
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
					       o->engine_data);
   o->engine_data = NULL;   
   o->load_error = EVAS_LOAD_ERROR_NONE;
   o->cur.has_alpha = 1;
   o->cur.image.w = 0;
   o->cur.image.h = 0;
}

static void
evas_object_image_load(Evas_Object *obj)
{
   Evas_Object_Image *o;
   
   o = (Evas_Object_Image *)(obj->object_data);
   
   o->engine_data = obj->layer->evas->engine.func->image_load(obj->layer->evas->engine.data.output,
							      o->cur.file,
							      o->cur.key,
							      &o->load_error);
   if (o->engine_data)
     {
	obj->layer->evas->engine.func->image_size_get(obj->layer->evas->engine.data.output,
						      o->engine_data,
						      &(o->cur.image.w),
						      &(o->cur.image.h));
	o->cur.has_alpha = obj->layer->evas->engine.func->image_alpha_get(obj->layer->evas->engine.data.output,
									  o->engine_data);
     }
   else
     {
	o->load_error = EVAS_LOAD_ERROR_GENERIC;
     }
}

static double
evas_object_image_figure_x_fill(Evas_Object *obj, double start, double size, double *size_ret)
{
   double w;

   w = ((size * obj->layer->evas->output.w) / 
	(double)obj->layer->evas->viewport.w);
   if (start > 0)
     {
	while (start - size > 0) start -= size;
     }
   else if (start < 0)
     {
	while (start < 0) start += size;
     }
   start = ((start * obj->layer->evas->output.w) /
	    (double)obj->layer->evas->viewport.w);	    
   *size_ret = w;
   return start;
}

static double
evas_object_image_figure_y_fill(Evas_Object *obj, double start, double size, double *size_ret)
{
   double h;

   h = ((size * obj->layer->evas->output.h) / 
	(double)obj->layer->evas->viewport.h);
   if (start > 0)
     {
	while (start - size > 0) start -= size;
     }
   else if (start < 0)
     {
	while (start < 0) start += size;
     }
   start = ((start * obj->layer->evas->output.h) /
	    (double)obj->layer->evas->viewport.h);
   *size_ret = h;
   return start;
}

static void
evas_object_image_init(Evas_Object *obj)
{
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_image_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0.0;
   obj->cur.geometry.y = 0.0;
   obj->cur.geometry.w = 32.0;
   obj->cur.geometry.h = 32.0;
   obj->cur.layer = 0;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_image_new(void)
{
   Evas_Object_Image *o;
   
   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Image));
   o->magic = MAGIC_OBJ_IMAGE;
   o->cur.fill.w = 32.0;
   o->cur.fill.h = 32.0;
   o->cur.smooth_scale = 1;
   o->prev = o->cur;
   return o;
}

static void
evas_object_image_free(Evas_Object *obj)
{
   Evas_Object_Image *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   if (o->cur.file) free(o->cur.file);
   if (o->cur.key) free(o->cur.key);
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
					       o->engine_data);
   o->magic = 0;
   while (o->pixel_updates)
     {
	Evas_Rectangle *r;
	
	r = (Evas_Rectangle *)o->pixel_updates->data;
	o->pixel_updates = evas_list_remove(o->pixel_updates, r);
	free(r);
     }
   free(o);
}

static void
evas_object_image_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Image *o;

   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Image *)(obj->object_data);
   obj->layer->evas->engine.func->context_color_set(output,
						    context,
						    255, 255, 255, 255);
   
   if ((obj->cur.cache.clip.r == 255) &&
       (obj->cur.cache.clip.g == 255) &&
       (obj->cur.cache.clip.b == 255) &&
       (obj->cur.cache.clip.a == 255))
     {
	obj->layer->evas->engine.func->context_multiplier_unset(output,
								context);
     }
   else
     obj->layer->evas->engine.func->context_multiplier_set(output,
							   context,
							   obj->cur.cache.clip.r,
							   obj->cur.cache.clip.g,
							   obj->cur.cache.clip.b,
							   obj->cur.cache.clip.a);
   if (o->engine_data)
     {
	double idw, idh, idx, idy;
	int ix, iy, iw, ih;
		  
	idx = evas_object_image_figure_x_fill(obj, o->cur.fill.x, o->cur.fill.w, &idw);
	idy = evas_object_image_figure_y_fill(obj, o->cur.fill.y, o->cur.fill.h, &idh);
	if (idw < 1.0) idw = 1.0;
	if (idh < 1.0) idh = 1.0;
	if (idx > 0.0) idx -= idw;
	if (idy > 0.0) idy -= idh;
	while (idx < (double)obj->cur.cache.geometry.w)
	  {
	     double ydy;
	     
	     ydy = idy;
	     ix = idx;
	     iw = ((int)(idx + idw)) - ix;
	     while (idy < (double)obj->cur.cache.geometry.h)
	       {
		  iy = idy;
		  ih = ((int)(idy + idh)) - iy;
		  if ((o->cur.border.l == 0) &&
		      (o->cur.border.r == 0) &&
		      (o->cur.border.t == 0) &&
		      (o->cur.border.b == 0))
		    obj->layer->evas->engine.func->image_draw(output,
							      context,
							      surface,
							      o->engine_data,
							      0, 0, 
							      o->cur.image.w,
							      o->cur.image.h,
							      obj->cur.cache.geometry.x + ix + x, 
							      obj->cur.cache.geometry.y + iy + y, 
							      iw, ih, 
							      o->cur.smooth_scale);
		  else
		    {
		       int inx, iny, inw, inh, outx, outy, outw, outh;
		       int bl, br, bt, bb;
		       int imw, imh, ox, oy;
		       
		       ox = obj->cur.cache.geometry.x + ix + x;
		       oy = obj->cur.cache.geometry.y + iy + y;
		       imw = o->cur.image.w;
		       imh = o->cur.image.h;
		       bl = o->cur.border.l;
		       br = o->cur.border.r;
		       bt = o->cur.border.t;
		       bb = o->cur.border.b;
		       if ((bl + br) > iw)
			 {
			    bl = iw / 2;
			    br = iw - bl;
			 }
		       if ((bl + br) > imw)
			 {
			    bl = imw / 2;
			    br = imw - bl;
			 }
		       if ((bt + bb) > ih)
			 {
			    bt = ih / 2;
			    bb = ih - bt;
			 }
		       if ((bt + bb) > imh)
			 {
			    bt = imh / 2;
			    bb = imh - bt;
			 }
		       
		       inx = 0; iny = 0; 
		       inw = bl; inh = bt;
		       outx = ox; outy = oy;
		       outw = bl; outh = bt;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       inx = bl; iny = 0; 
		       inw = imw - bl - br; inh = bt;
		       outx = ox + bl; outy = oy;
		       outw = iw - bl - br; outh = bt;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       inx = imw - br; iny = 0; 
		       inw = br; inh = bt;
		       outx = ox + iw - br; outy = oy;
		       outw = br; outh = bt;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       
		       inx = 0; iny = bt; 
		       inw = bl; inh = imh - bt - bb;
		       outx = ox; outy = oy + bt;
		       outw = bl; outh = ih - bt - bb;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       inx = bl; iny = bt; 
		       inw = imw - bl - br; inh = imh - bt - bb;
		       outx = ox + bl; outy = oy + bt;
		       outw = iw - bl - br; outh = ih - bt - bb;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       inx = imw - br; iny = bt; 
		       inw = br; inh = imh - bt - bb;
		       outx = ox + iw - br; outy = oy + bt;
		       outw = br; outh = ih - bt - bb;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       
		       inx = 0; iny = imh - bb; 
		       inw = bl; inh = bb;
		       outx = ox; outy = oy + ih - bb;
		       outw = bl; outh = bb;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       inx = bl; iny = imh - bb; 
		       inw = imw - bl - br; inh = bb;
		       outx = ox + bl; outy = oy + ih - bb;
		       outw = iw - bl - br; outh = bb;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       inx = imw - br; iny = imh - bb; 
		       inw = br; inh = bb;
		       outx = ox + iw - br; outy = oy + ih - bb;
		       outw = br; outh = bb;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		    }
		  idy += idh;
	       }
	     idx += idw;
	     idy = ydy;
	  }
     }
}

static void
evas_object_image_render_pre(Evas_Object *obj)
{
   Evas_List *updates = NULL;
   Evas_Object_Image *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add thr appropriate redraw rectangles */
   o = (Evas_Object_Image *)(obj->object_data);
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	updates = evas_object_render_pre_visible_change(updates, obj, is_v, was_v);
	goto done;
     }
   /* its not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   updates = evas_object_render_pre_clipper_change(updates, obj);
   /* if we restacked (layer or just within a layer) and dont clip anyone */
   if (obj->restack)
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   if (o->changed)
     {
	if (((o->cur.file) && (!o->prev.file)) ||
	    ((!o->cur.file) && (o->prev.file)) ||
	    (((o->cur.file) && (o->prev.file) && (strcmp(o->cur.file, o->prev.file)))) ||
	    ((o->cur.key) && (!o->prev.key)) ||
	    ((!o->cur.key) && (o->prev.key)) ||
	    (((o->cur.key) && (o->prev.key) && (strcmp(o->cur.key, o->prev.key))))
	    )
	  {
	     updates = evas_object_render_pre_prev_cur_add(updates, obj);
	     goto done;
	  }
	if ((o->cur.image.w != o->prev.image.w) ||
	    (o->cur.image.h != o->prev.image.h) ||
	    (o->cur.has_alpha != o->prev.has_alpha) ||
	    (o->cur.smooth_scale != o->prev.smooth_scale))
	  {
	     updates = evas_object_render_pre_prev_cur_add(updates, obj);
	     goto done;
	  }
	if ((o->cur.border.l != o->prev.border.l) ||
	    (o->cur.border.r != o->prev.border.r) ||
	    (o->cur.border.t != o->prev.border.t) ||
	    (o->cur.border.b != o->prev.border.b))
	  {
	     updates = evas_object_render_pre_prev_cur_add(updates, obj);
	     goto done;
	  }
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* caluclate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if (((obj->cur.geometry.x != obj->prev.geometry.x) ||
	(obj->cur.geometry.y != obj->prev.geometry.y) ||
	(obj->cur.geometry.w != obj->prev.geometry.w) ||
	(obj->cur.geometry.h != obj->prev.geometry.h)) &&
       (o->cur.fill.w == o->prev.fill.w) &&
       (o->cur.fill.h == o->prev.fill.h) &&
       ((o->cur.fill.x + obj->cur.geometry.x) == (o->prev.fill.x + obj->prev.geometry.x)) &&
       ((o->cur.fill.y + obj->cur.geometry.y) == (o->prev.fill.y + obj->prev.geometry.y)) &&
       (!o->pixel_updates)
       )
     {
	Evas_Rectangle *r;
	Evas_List *rl;
	
	rl = evas_rects_return_difference_rects(obj->cur.cache.geometry.x, 
						obj->cur.cache.geometry.y, 
						obj->cur.cache.geometry.w, 
						obj->cur.cache.geometry.h,
						obj->prev.cache.geometry.x, 
						obj->prev.cache.geometry.y, 
						obj->prev.cache.geometry.w, 
						obj->prev.cache.geometry.h);
	while (rl)
	  {
	     r = rl->data;
	     rl = evas_list_remove(rl, r);
	     updates = evas_list_append(updates, r);	     
	  }
	goto done;
     }
   if (((obj->cur.geometry.x != obj->prev.geometry.x) ||
	(obj->cur.geometry.y != obj->prev.geometry.y) ||
	(obj->cur.geometry.w != obj->prev.geometry.w) ||
	(obj->cur.geometry.h != obj->prev.geometry.h))
       )
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   if (o->changed)
     {
	if ((o->cur.fill.x != o->prev.fill.x) ||
	    (o->cur.fill.y != o->prev.fill.y) ||
	    (o->cur.fill.w != o->prev.fill.w) ||
	    (o->cur.fill.h != o->prev.fill.h))
	  {
	     updates = evas_object_render_pre_prev_cur_add(updates, obj);
	     goto done;
	  }
	if ((o->cur.border.l == 0) &&
	    (o->cur.border.r == 0) &&
	    (o->cur.border.t == 0) &&
	    (o->cur.border.b == 0))
	  {
	     while (o->pixel_updates)
	       {
		  Evas_Rectangle *r, *rr;
		  double idw, idh, idx, idy;
		  int x, y, w, h;
		  
		  rr = o->pixel_updates->data;
		  o->pixel_updates = evas_list_remove(o->pixel_updates, rr);
		  idx = evas_object_image_figure_x_fill(obj, o->cur.fill.x, o->cur.fill.w, &idw);
		  idy = evas_object_image_figure_y_fill(obj, o->cur.fill.y, o->cur.fill.h, &idh);
		  
		  if (idw < 1.0) idw = 1.0;
		  if (idh < 1.0) idh = 1.0;
		  if (idx > 0) idx -= idw;
		  if (idy > 0) idy -= idh;
		  while (idx < obj->cur.cache.geometry.w)
		    {
		       double ydy;
		       
		       ydy = idy;
		       x = idx;
		       w = ((int)(idx + idw)) - x;
		       while (idy < obj->cur.cache.geometry.h)
			 {
			    y = idy;
			    h = ((int)(idy + idh)) - y;
			    NEW_RECT(r, x, y, w, h);
			    r->x = ((rr->x - 1) * r->w) / o->cur.image.w;
			    r->y = ((rr->y - 1) * r->h) / o->cur.image.h;
			    r->w = ((rr->w + 2) * r->w) / o->cur.image.w;
			    r->h = ((rr->h + 2) * r->h) / o->cur.image.h;
			    r->x += obj->cur.cache.geometry.x + x;
			    r->y += obj->cur.cache.geometry.y + y;
			    if (r) updates = evas_list_append(updates, r);
			    idy += h;
			 }
		       idx += idw;
		       idy = ydy;
		    }
		  free(rr);
	       }
	     goto done;
	  }
	else
	  {
	     if (o->pixel_updates)
	       {
		  while (o->pixel_updates)
		    {
		       Evas_Rectangle *r;
		       
		       r = (Evas_Rectangle *)o->pixel_updates->data;
		       o->pixel_updates = evas_list_remove(o->pixel_updates, r);
		       free(r);
		    }
		  updates = evas_object_render_pre_prev_cur_add(updates, obj);		  
		  goto done;
	       }
	  }
     }
   /* it obviously didn't change - add a NO obscure - this "unupdates"  this */
   /* area so if there were updates for it they get wiped. don't do it if we */
   /* arent fully opaque and we are visible */
   if (evas_object_is_visible(obj) &&
       evas_object_is_opaque(obj))
     obj->layer->evas->engine.func->output_redraws_rect_del(obj->layer->evas->engine.data.output,
							    obj->cur.cache.clip.x, 
							    obj->cur.cache.clip.y, 
							    obj->cur.cache.clip.w, 
							    obj->cur.cache.clip.h);
   done:
   evas_object_render_pre_effect_updates(updates, obj, is_v, was_v);
}

static void
evas_object_image_render_post(Evas_Object *obj)
{
   Evas_Object_Image *o;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Image *)(obj->object_data);
   /* remove those pesky changes */
   while (obj->clip.changes)
     {
	Evas_Rectangle *r;
	
	r = (Evas_Rectangle *)obj->clip.changes->data;
	obj->clip.changes = evas_list_remove(obj->clip.changes, r);
	free(r);
     }
   while (o->pixel_updates)
     {
	Evas_Rectangle *r;
	
	r = (Evas_Rectangle *)o->pixel_updates->data;
	o->pixel_updates = evas_list_remove(o->pixel_updates, r);
	free(r);
     }
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   o->prev = o->cur;
   o->changed = 0;
   /* FIXME: copy strings across */
}

static int
evas_object_image_is_opaque(Evas_Object *obj)
{
   Evas_Object_Image *o;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fulyl opque over the entire rectangle it occupies */
   o = (Evas_Object_Image *)(obj->object_data);
   if (!o->engine_data) return 0;
   if (o->cur.has_alpha) return 0;
   return 1;
}

static int
evas_object_image_was_opaque(Evas_Object *obj)
{
   Evas_Object_Image *o;
   
   /* this returns 1 if the internal object data implies that the object was */
   /* currently fulyl opque over the entire rectangle it occupies */
   o = (Evas_Object_Image *)(obj->object_data);
   if (!o->engine_data) return 0;
   if (o->prev.has_alpha) return 0;
   return 1;
}
