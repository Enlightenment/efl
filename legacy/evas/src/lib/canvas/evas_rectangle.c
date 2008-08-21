#include "evas_common.h"
#include "evas_private.h"

void
evas_rects_return_difference_rects(Evas_Rectangles *rects, int x, int y, int w, int h, int xx, int yy, int ww, int hh)
{
   if (!RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh))
     {
	evas_add_rect(rects, x, y, w, h);
	evas_add_rect(rects, xx, yy, ww, hh);
     }
   else
     {
	int x1[4], y1[4], i, j;

	if (x < xx)
	  {
	     x1[0] = x;
	     x1[1] = xx;
	  }
	else
	  {
	     x1[0] = xx;
	     x1[1] = x;
	  }
	if ((x + w) < (xx + ww))
	  {
	     x1[2] = x + w;
	     x1[3] = xx + ww;
	  }
	else
	  {
	     x1[2] = xx + ww;
	     x1[3] = x + w;
	  }
	if (y < yy)
	  {
	     y1[0] = y;
	     y1[1] = yy;
	  }
	else
	  {
	     y1[0] = yy;
	     y1[1] = y;
	  }
	if ((y + h) < (yy + hh))
	  {
	     y1[2] = y + h;
	     y1[3] = yy + hh;
	  }
	else
	  {
	     y1[2] = yy + hh;
	     y1[3] = y + h;
	  }
	for (j = 0; j < 3; j++)
	  {
	     for (i = 0; i < 3; i++)
	       {
		  int intsec1, intsec2;
		  int tx, ty, tw, th;

		  tx = x1[i];
		  ty = y1[j];
		  tw = x1[i + 1] - x1[i];
		  th = y1[j + 1] - y1[j];

		  intsec1 = (RECTS_INTERSECT(tx, ty, tw, th, x, y, w, h));
		  intsec2 = (RECTS_INTERSECT(tx, ty, tw, th, xx, yy, ww, hh));
		  if (intsec1 ^ intsec2)
		    {
		       evas_add_rect(rects, tx, ty, tw, th);
		    }
	       }
	  }
/* 	if (tmp.count > 0) */
/* 	  { */
/* 	     unsigned int i; */

/* 	     for (i = 0; i < tmp.count; ++i) */
/* 	       { */
/* 		  if ((tmp.array[i].w > 0) && (tmp.array[i].h > 0)) */
/* 		    { */
/* 		       int intsec1, intsec2; */

/* 		       intsec1 = (RECTS_INTERSECT(tmp.array[i].x, tmp.array[i].y, tmp.array[i].w, tmp.array[i].h, x, y, w, h)); */
/* 		       intsec2 = (RECTS_INTERSECT(tmp.array[i].x, tmp.array[i].y, tmp.array[i].w, tmp.array[i].h, xx, yy, ww, hh)); */
/* 		       if (intsec1 ^ intsec2) */
/* 			 { */
/* 			    evas_add_rect(rects, tmp.array[i].x, tmp.array[i].y, tmp.array[i].w, tmp.array[i].h); */
/* 			 } */
/* 		    } */
/* 	       } */
/* 	     free(tmp.array); */
/* 	  } */

     }
}
