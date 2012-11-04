#include "evas_common.h"
#include "evas_private.h"

void
evas_rects_return_difference_rects(Eina_Array *rects, int x, int y, int w, int h, int xx, int yy, int ww, int hh)
{
   if (!RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh))
     {
	evas_add_rect(rects, x, y, w, h);
	evas_add_rect(rects, xx, yy, ww, hh);
     }
   else
     {
	int pt_x[4], pt_y[4], i, j;

	if (x < xx)
	  {
	     pt_x[0] = x;
	     pt_x[1] = xx;
	  }
	else
	  {
	     pt_x[0] = xx;
	     pt_x[1] = x;
	  }
	if ((x + w) < (xx + ww))
	  {
	     pt_x[2] = x + w;
	     pt_x[3] = xx + ww;
	  }
	else
	  {
	     pt_x[2] = xx + ww;
	     pt_x[3] = x + w;
	  }
	if (y < yy)
	  {
	     pt_y[0] = y;
	     pt_y[1] = yy;
	  }
	else
	  {
	     pt_y[0] = yy;
	     pt_y[1] = y;
	  }
	if ((y + h) < (yy + hh))
	  {
	     pt_y[2] = y + h;
	     pt_y[3] = yy + hh;
	  }
	else
	  {
	     pt_y[2] = yy + hh;
	     pt_y[3] = y + h;
	  }
	for (j = 0; j < 3; j++)
	  {
	     for (i = 0; i < 3; i++)
	       {
		  int intsec1, intsec2;
		  int tx, ty, tw, th;

		  tx = pt_x[i];
		  ty = pt_y[j];
		  tw = pt_x[i + 1] - pt_x[i];
		  th = pt_y[j + 1] - pt_y[j];

		  intsec1 = (RECTS_INTERSECT(tx, ty, tw, th, x, y, w, h));
		  intsec2 = (RECTS_INTERSECT(tx, ty, tw, th, xx, yy, ww, hh));
		  if (intsec1 ^ intsec2)
		    {
		       evas_add_rect(rects, tx, ty, tw, th);
		    }
	       }
	  }
/*	if (tmp.count > 0) */
/*	  { */
/*	     unsigned int i; */

/*	     for (i = 0; i < tmp.count; ++i) */
/*	       { */
/*		  if ((tmp.array[i].w > 0) && (tmp.array[i].h > 0)) */
/*		    { */
/*		       int intsec1, intsec2; */

/*		       intsec1 = (RECTS_INTERSECT(tmp.array[i].x, tmp.array[i].y, tmp.array[i].w, tmp.array[i].h, x, y, w, h)); */
/*		       intsec2 = (RECTS_INTERSECT(tmp.array[i].x, tmp.array[i].y, tmp.array[i].w, tmp.array[i].h, xx, yy, ww, hh)); */
/*		       if (intsec1 ^ intsec2) */
/*			 { */
/*			    evas_add_rect(rects, tmp.array[i].x, tmp.array[i].y, tmp.array[i].w, tmp.array[i].h); */
/*			 } */
/*		    } */
/*	       } */
/*	     free(tmp.array); */
/*	  } */

     }
}
