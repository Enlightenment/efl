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
	/* FIXME: free layer */
     }
   if (e->layers) evas_list_free(e->layers);
   free(e);
}

void
evas_set_color(Evas e, Evas_Object o, int r, int g, int b, int a)
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
