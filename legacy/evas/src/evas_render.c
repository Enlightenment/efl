#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* for exposes or forced redraws (relative to output drawable) */
void
evas_update_rect(Evas e, int x, int y, int w, int h)
{
   Evas_Rectangle r;
   
   r = malloc(sizeof(struct _Evas_Rectangle));
   r->x = x;
   r->y = y;
   r->w = w;
   r->h = h;
   e->updates = evas_list_prepend(e->updates, r);
}

/* drawing */
void
evas_render(Evas e)
{
}

/* query for settings to use */
Visual *
evas_get_optimal_visual(Evas e, Display *disp)
{
}

Colormap
evas_get_optimal_colormap(Evas e, Display *disp)
{
}

/* the output settings */
void
evas_set_output(Evas e, Display *disp, Drawable d, Visual *v, Colormap c)
{
   e->current.display = disp;
   e->current.drawable = d;
   e->current.visual = v;
   e->current.colormap = c;
}

void
evas_set_output_rect(Evas e, int x, int y, int w, int h)
{
   e->current.output.x = x;
   e->current.output.y = y;
   e->current.output.w = w;
   e->current.output.h = h;
}

void
evas_set_output_viewport(Evas e, double x, double y, double w, double h)
{
   e->current.viewport.x = x;
   e->current.viewport.y = y;
   e->current.viewport.w = w;
   e->current.viewport.h = h;
}

void
evas_set_output_method(Evas e, Evas_Render_Method method)
{
   e->current.render_method = method;
}
