#include "Evas.h"

/* create and destroy */
Evas
evas_new(void)
{
}

void
evas_free(Evas e)
{
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
evas_set_image_fill_size(Evas e, Evas_Object o, double w, double h)
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
evas_set_gradient_angle(Evas e, Evas_Object o, double angle)
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
evas_set_blend_mode(Evas e, int mode)
{
}

void
evas_set_zoom_scale(Evas e, Evas_Object o, int scale)
{
}

/* layer stacking for object */
void
evas_set_layer(Evas e, Evas_Object o, int l)
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

