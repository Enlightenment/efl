#include "evas_common.h"

#if 0
Regionbuf *
evas_common_regionbuf_new(int w, int h)
{
   Regionbuf *rb;

   rb = calloc(1, sizeof(Regionbuf) + (h * sizeof(Regionspan)));
   if (!rb) return NULL;
   rb->spans = (Regionspan **)(rb + sizeof(Regionbuf));
   rb->w = w;
   rb->h = h;
   return rb;
}

void
evas_common_regionbuf_free(Regionbuf *rb)
{
   evas_common_regionbuf_clear(rb);
   free(rb);
}

void
evas_common_regionbuf_clear(Regionbuf *rb)
{
   int y;

   for (y = 0; y < rb->h; y++)
     {
	while (rb->spans[y])
	  {
	     Regionspan *span;

	     span = rb->spans[y];
	     rb->spans[y] = eina_inlist_remove(rb->spans[y], rb->spans[y]);
	     free(span);
	  }
     }
}

void
evas_common_regionbuf_span_add(Regionbuf *rb, int x1, int x2, int y)
{
   Regionspan *span, *span2, *nspan, *sp_start, *sp_stop;

   /* abort if outside */
   if ((y < 0) ||
       (y >= rb->h) ||
       (x2 < 0) ||
       (x1 >= rb->w)) return;
   /* clip to horiz bounds */
   if (x1 < 0) x1 = 0;
   if (x2 < (rb->w - 1)) x2 = rb->w - 1;
   sp_start = NULL;
   sp_stop = NULL;
   EINA_INLIST_ITER_NEXT(rb->spans[y], span)
     {
	nspan = (Regionspan *)(EINA_INLIST_GET(span))->next;
	/* we dont know what t do with the span yet */
	if (!sp_start)
	  {
	     /* if new span starts before or on this span or just after
	      * with no gap */
	     if (x1 <= (span->x2 + 1))
	       sp_start = span;
	     /* if there is no next span */
	     if (!nspan)
	       {
		  sp_stop = span;
		  break;
	       }
	     /* if new span ends before the next span starts with a gap of
	      * 1 pixel (or more) */
	     else if (x2 < (nspan->x1 - 1))
	       {
		  sp_stop = span;
		  break;
	       }
	  }
	/* we already know it already starts before or in sp_start */
	else
	  {
	     /* there is no span after this one, so this has to be the stop */
	     if (!nspan)
	       {
		  sp_stop = span;
		  break;
	       }
	     /* if new span ends before the next span starts with a gap of
	      * 1 pixel (or more) */
	     else if (x2 < (nspan->x1 - 1))
	       {
		  sp_stop = span;
		  break;
	       }
	  }
     }
   /* sp_start is where the new span starts in or before */
   /* sp_stop is where the new span stops in or after */
   if ((sp_start) && (sp_stop))
     {
	/* same start and stop */
	if (sp_start == sp_stop)
	  {
	     if (x2 < (sp_start->x1 - 1))
	       {
		  span2 = calloc(1, sizeof(Regionspan));
		  span2->x1 = x1;
		  span2->x2 = x2;
		  rb->spans[y] = eina_inlist_prepend_relative(rb->spans[y], span2, sp_start);
		  return;
	       }
	     if (x1 < sp_start->x1)
	       sp_start->x1 = x1;
	     if (x2 > sp_start->x2)
	       sp_start->x2 = x2;
	     return;
	  }
	else
	  {
	     Eina_Inlist *l;

	     /* remove all nodes after sp_start and before_sp_stop because
	      * the new  */
	     for (l = (EINA_INLIST_GET(sp_start))->next; l != EINA_INLIST_GET(sp_stop);)
	       {
		  span = (Regionspan *)l;
		  l = l->next;
		  rb->spans[y] = eina_inlist_remove(rb->spans[y], span);
		  free(span);
	       }
	     /* remove the end span */
	     rb->spans[y] = eina_inlist_remove(rb->spans[y], sp_stop);
	     /* if the new span is before the start span - extend */
	     if (x1 < sp_start->x1)
	       sp_start->x1 = x1;
	     /* if it goes beyond the stop span - extend stop span */
	     if (x2 > sp_stop->x2)
	       sp_stop->x2 = x2;
	     /* extend start span to stop span */
	     sp_start->x2 = sp_stop->x2;
	     /* don't need stop span anymore */
	     free(sp_stop);
	     return;
	  }
     }
   /* no start AND stop... just append */
   span2 = calloc(1, sizeof(Regionspan));
   span2->x1 = x1;
   span2->x2 = x2;
   rb->spans[y] = eina_inlist_append(rb->spans[y], span2);
}

void
evas_common_regionbuf_span_del(Regionbuf *rb, int x1, int x2, int y)
{
   /* FIXME: del span */
   Regionspan *span, *span2, *nspan, *sp_start, *sp_stop;

   /* abort if outside */
   if ((y < 0) ||
       (y >= rb->h) ||
       (x2 < 0) ||
       (x1 >= rb->w)) return;
   /* clip to horiz bounds */
   if (x1 < 0) x1 = 0;
   if (x2 < (rb->w - 1)) x2 = rb->w - 1;
   sp_start = NULL;
   sp_stop = NULL;
   EINA_INLIST_ITER_NEXT(rb->spans[y], span)
     {
	nspan = (Regionspan *)(EINA_INLIST_GET(l))->next;
	/* we dont know what t do with the span yet */
	if (!sp_start)
	  {
	     /* if new span starts before or on this span or just after
	      * with no gap */
	     if (x1 <= (span->x2))
	       sp_start = span;
	     /* if there is no next span */
	     if (!nspan)
	       {
		  sp_stop = span;
		  break;
	       }
	     /* if new span ends before the next span starts with a gap of
	      * 1 pixel (or more) */
	     else if (x2 < nspan->x1)
	       {
		  sp_stop = span;
		  break;
	       }
	  }
	/* we already know it already starts before or in sp_start */
	else
	  {
	     /* there is no span after this one, so this has to be the stop */
	     if (!nspan)
	       {
		  sp_stop = span;
		  break;
	       }
	     /* if new span ends before the next span starts with a gap of
	      * 1 pixel (or more) */
	     else if (x2 < nspan->x1)
	       {
		  sp_stop = span;
		  break;
	       }
	  }
     }
   /* sp_start is where the new span starts in or before */
   /* sp_stop is where the new span stops in or after */
   if ((sp_start) && (sp_stop))
     {
	/* same start and stop */
	if (sp_start == sp_stop)
	  {
	     /* if it ends before this the span start starts... return */
	     if (x2 < sp_start->x1)
	       return;
	     /* it starts on or before this span */
	     else if (x1 <= sp_start->x1)
	       {
		  /* right edge is within the span */
		  if (x2 < sp_start->x2)
		    {
		       sp_start->x2 = x2;
		       return;
		    }
		  else
		    {
		       rb->spans[y] = eina_inlist_remove(rb->spans[y], sp_start);
		       return;
		    }
	       }
	     /* it ends on or after the end of thsi span */
	     else if (x2 >= sp_start->x2)
	       {
		  /* it starts after the start */
		  if (x1 > sp_start->x1)
		    {
		       sp_start->x1 = x1;
		       return;
		    }
		  /* remove it all */
		  else
		    {
		       rb->spans[y] = eina_inlist_remove(rb->spans[y], sp_start);
		       return;
		    }
		  return;
	       }
	     /* this breaks the span into 2 */
	     else
	       {
		  span2 = calloc(1, sizeof(Regionspan));
		  span2->x1 = sp_start->x1;
		  span2->x2 = x1 - 1;
		  rb->spans[y] = eina_inlist_prepend_relative(rb->spans[y], span2, sp_start);
		  sp_start->x1 = x2 + 1;
		  return;
	       }
	  }
	else
	  {
	     Eina_Inlist *l;

	     /* remove all nodes after sp_start and before_sp_stop because
	      * the new  */
	     for (l = (EINA_INLIST_GET(sp_start))->next; l != EINA_INLIST_GET(sp_stop);)
	       {
		  span = (Regionspan *)l;
		  l = l->next;
		  rb->spans[y] = eina_inlist_remove(rb->spans[y], span);
		  free(span);
	       }
	     /* all of the start span is cut out */
	     if (x1 <= sp_start->x1)
	       {
		  rb->spans[y] = eina_inlist_remove(rb->spans[y], sp_start);
		  free(sp_start);
	       }
	     /* chup it off at the new span start */
	     else
	       sp_start->x2 = x1 - 1;
	     /* all of the end span is cut out */
	     if (x2 >= sp_stop->x2)
	       {
		  rb->spans[y] = eina_inlist_remove(rb->spans[y], sp_stop);
		  free(sp_stop);
	       }
	     /* chop it up at the end */
	     else
	       sp_stop->x1 = x2 + 1;
	     return;
	  }
     }
}

Tilebuf_Rect *
evas_common_regionbuf_rects_get(Regionbuf *rb)
{
   Tilebuf_Rect *rects = NULL, *r;
   int y;

   /* FIXME: take spans, make rects */
   for (y = 0; y < rb->h; y++)
     {
	Regionspan *sp_start;
	Eina_Inlist *l, *ll;

	for (l = EINA_INLIST_GET(rb->spans[y]); l;)
	  {
	     Regionspan *span;
	     int yy;

	     sp_start = (Regionspan *)l;
	     l = l->next;
	     rb->spans[y] = eina_inlist_remove(rb->spans[y], sp_start);
	     for (yy = y + 1; yy < rb->h; yy++)
	       {
		  int match = 0;

		  for (ll = EINA_INLIST_GET(rb->spans[yy]); ll;)
		    {
		       span = (Regionspan *)ll;
		       ll = ll->next;
		       if (span->x1 == sp_start->x1)
			 {
			    if ((span->x1 != sp_start->x1) ||
				(span->x2 != sp_start->x2))
			      {
				 goto coallate;
			      }
			    match = 1;
			    rb->spans[yy] = eina_inlist_remove(rb->spans[yy], span);
			    free(span);
			 }
		    }
		  if (!match) goto coallate;
	       }
	     coallate:
	     r = calloc(1, sizeof(Tilebuf_Rect));
	     r->x = sp_start->x1;
	     r->y = y;
	     r->w = sp_start->x2 - sp_start->x1 + 1;
	     r->h = yy - y;
	     rects = eina_inlist_append(rects, r);
	     free(sp_start);
	  }
     }
   evas_common_regionbuf_clear(rb);
   return rects;
}
#endif
