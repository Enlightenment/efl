#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* create and destroy */
Evas
evas_new(void)
{
   Evas e;
   
   e = malloc(sizeof(struct _Evas));
   memset(e, 0, sizeof(struct _Evas));
   e->current.viewport.x = 0.0;
   e->current.viewport.y = 0.0;
   e->current.viewport.w = 0.0;
   e->current.viewport.h = 0.0;
   e->current.output.x = 0;
   e->current.output.y = 0;
   e->current.output.w = 0;
   e->current.output.h = 0;   
   e->current.render_method = RENDER_METHOD_BASIC_HARDWARE;
   return e;
}

void
evas_free(Evas e)
{
   Evas_List l;
   
   for (l = e->layers; l; l = l->next)
     {
	/* free layer */
     }
   if (e->layers) evas_list_free(e->layers);
   free(e);
}

/* for exposes or forced redraws (relative to output drawable) */
void
evas_update_rect(Evas e, int x, int y, int w, int h)
{
}

/* drawing */
void
evas_render(Evas e)
{
}

/* query for settings to use */
Visual *
evas_get_optimal_visual(Display *disp)
{
}

Colormap
evas_get_optimal_colormap(Display *disp)
{
}

/* the output settings */
void
evas_set_output(Evas e, Display *disp, Drawable d, Visual *v, Colormap c)
{
}

void
evas_set_output_rect(Evas e, int x, int y, int w, int h)
{
}

void
evas_set_viewport(Evas e, double x, double y, double w, double h)
{
}

/* deleting objects */
void
evas_del_object(Evas e, Evas_Object o)
{
}

/* adding objects */
Evas_Object
evas_add_image_from_file(Evas e, char *file)
{
}

Evas_Object
evas_add_image_from_data(Evas e, void *data, Evas_Image_Format format, int w, int h)
{
}

Evas_Object
evas_add_text(Evas e, char *font, int size, char *text)
{
}

Evas_Object
evas_add_rectangle(Evas e, int r, Evas_Group g, int b)
{
}

Evas_Object
evas_add_line(Evas e, int r, Evas_Group g, int b, int a)
{
}

Evas_Object
evas_add_gradient_box(Evas e)
{
}

Evas_Object
evas_add_bits(Evas e, char *file)
{
}

Evas_Object
evas_add_evas(Evas e, Evas evas)
{
}

/* set object settings */
void
evas_set_image_file(Evas e, Evas_Object o, char *file)
{
}

void
evas_set_image_fill(Evas e, Evas_Object o, double x, double y, double w, double h)
{
}

void
evas_set_bits_file(Evas e, Evas_Object o, char *file)
{
}

void
evas_set_color(Evas e, Evas_Object o, int r, Evas_Group g, int b, int a)
{
}

void
evas_set_gradient(Evas e, Evas_Object o, Evas_Gradient grad)
{
}

void
evas_set_angle(Evas e, Evas_Object o, double angle)
{
}

void
evas_set_blend_mode(Evas e, Evas_Blend_Mode mode)
{
}

void
evas_set_zoom_scale(Evas e, Evas_Object o, int scale)
{
}

void
evas_set_line_xy(Evas e, Evas_Object o, double x1, double y1, double x2, double y2)
{
}

/* layer stacking for object */
void
evas_set_layer(Evas e, Evas_Object o, int l)
{
}

void
evas_set_layer_store(Evas e, int l, int store)
{
}

/* gradient creating / deletion / modification */
Evas_Gradient
evas_gradient_new(void)
{
}

void
evas_gradient_free(Evas_Gradient grad)
{
}

void
evas_gradient_add_color(Evas_Gradient grad, int r, int g, int b, int a, int dist)
{
}

/* stacking within a layer */
void
evas_raise(Evas e, Evas_Object o)
{
}

void
evas_lower(Evas e, Evas_Object o)
{
}

void
evas_stack_above(Evas e, Evas_Object o, int above)
{
}

void
evas_stack_below(Evas e, Evas_Object o, int above)
{
}

/* object geoemtry */
void
evas_move(Evas e, Evas_Object o, double x, double y)
{
}

void
evas_resize(Evas e, Evas_Object o, double w, double h)
{
}

void
evas_get_geometry(Evas e, Evas_Object o, double *x, double *y, double *w, double *h)
{
}

/* object visibility */
void
evas_show(Evas e, Evas_Object o)
{
}

void
evas_hide(Evas e, Evas_Object o)
{
}

/* group operations */
Evas_Group
evas_add_group(Evas e)
{
}

void
evas_add_to_group(Evas e, Evas_Object o, Evas_Group g)
{
}

void
evas_disband_group(Evas e, Evas_Group g)
{
}

void
evas_free_group(Evas e, Evas_Group g)
{
}

void
evas_del_from_group(Evas e, Evas_Object o, Evas_Group g)
{
}

/* evas bits ops */
void
evas_bits_get_padding(Evas e, Evas_Object o, double *l, double *r, double *t, double *b)
{
}

void
evas_bits_get_min(Evas e, Evas_Object o, double *w, double *h)
{
}

void
evas_bits_get_max(Evas e, Evas_Object o, double *w, double *h)
{
}

void
evas_bits_get_classed_bit_geoemtry(Evas e, Evas_Object o, char *class, double *x, double *y, double *w, double *h)
{
}

/* image query ops */
void
evas_get_image_size(Evas e, Evas_Object o, int *w, int *h)
{
}

/* events */
void
evas_event_button_down(Evas e, int x, int y, int b)
{
}

void
evas_event_button_up(Evas e, int x, int y, int b)
{
}

void
evas_event_move(Evas e, int x, int y)
{
}

void
evas_event_enter(Evas e)
{
}

void
evas_event_leave(Evas e)
{
}

/* callbacks */
void
evas_callback_add(Evas e, Evas_Object o, Evas_Callback_Type callback, void (*func) (void *_data, Evas _e, char *_class, Evas_Object _o, int _b, int _x, int _y), void *data)

{
}

void
evas_callback_del(Evas e, Evas_Object o, Evas_Callback_Type callback)
{
}

/* list ops */
Evas_List
evas_list_append(Evas_List list, void *data)
{
   Evas_List l, new_l;
   
   new_l = malloc(sizeof(struct _Evas_List));
   new_l->next = NULL;
   new_l->prev = NULL;
   new_l->data = data;
   if (!list) return new_l;
   for (l = list; l; l = l->next)
     {
	if (!l->next)
	  {
	     l->next = new_l;
	     new_l->prev = l;
	     return list;
	  }
     }
   return list;
}

Evas_List
evas_list_prepend(Evas_List list, void *data)
{
   Evas_List new_l;
   
   new_l = malloc(sizeof(struct _Evas_List));
   new_l->next = NULL;
   new_l->prev = NULL;
   new_l->data = data;
   if (!list) return new_l;
   new_l->next = list;
   list->prev = new_l;
   return new_l;
}

Evas_List
evas_list_append_relative(Evas_List list, void *data, void *relative)
{
   Evas_List l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  {
	     Evas_List new_l;
	     
	     new_l = malloc(sizeof(struct _Evas_List));
	     new_l->next = NULL;
	     new_l->prev = NULL;
	     new_l->data = data;
	     if (l->next)
	       {
		  new_l->next = l->next;
		  l->next->prev = new_l;
	       }
	     l->next = new_l;
	     new_l->prev = l;
	     return list;
	  }
     }
   return evas_list_append(list, data);
}

Evas_List
evas_list_prepend_relative(Evas_List list, void *data, void *relative)
{
   Evas_List l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  {
	     Evas_List new_l;
	     
	     new_l = malloc(sizeof(struct _Evas_List));
	     new_l->next = NULL;
	     new_l->prev = NULL;
	     new_l->data = data;
	     new_l->prev = l->prev;
	     new_l->next = l;
	     if (l->prev)
		l->prev->next = new_l;
	     l->prev = new_l;
	     if (new_l->prev)
		return list;
	     else
		return new_l;
	  }
     }
   return evas_list_prepend(list, data);
}

Evas_List
evas_list_remove(Evas_List list, void *data)
{
   Evas_List l, return_l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == data)
	  {
	     if (l->next)
		l->next->prev = l->prev;
	     if (l->prev)
	       {
		  l->prev->next = l->next;
		  return_l = l->prev;
	       }
	     else
		return_l = l->next;
	     free(l);
	     return return_l;
	  }
     }
   return list;
}

void *
evas_list_find(Evas_List list, void *data)
{
   Evas_List l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == data) return data;
     }
   return NULL;
}

Evas_List
evas_list_free(Evas_List list)
{
   Evas_List l, free_l;
   
   for (l = list; l;)
     {
	free_l = l;
	l = l->next;
	free(free_l);
     }
   return NULL;
}

