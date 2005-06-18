#include "evas_common.h"
#include "evas_private.h"

Evas_List *
evas_rects_return_difference_rects(int x, int y, int w, int h, int xx, int yy, int ww, int hh)
{
   Evas_List *rects = NULL;
   Evas_Rectangle *r;

   if (!RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh))
     {
	NEW_RECT(r, x, y, w, h);
	if (r) rects = evas_list_append(rects, r);
	NEW_RECT(r, xx, yy, ww, hh);
	if (r) rects = evas_list_append(rects, r);
     }
   else
     {
	int x1[4], y1[4], i, j;
	Evas_List *rl = NULL, *rll;

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
		  NEW_RECT(r, x1[i], y1[j], x1[i + 1] - x1[i], y1[j + 1] - y1[j]);
		  if (r) rl = evas_list_append(rl, r);
	       }
	  }
	if (rl)
	  {
	     for (rll = rl; rll; rll = rll->next)
	       {
		  r = rll->data;
		  if ((r->w > 0) && (r->h > 0))
		    {
		       int intsec1, intsec2;

		       intsec1 = (RECTS_INTERSECT(r->x, r->y, r->w, r->h, x, y, w, h));
		       intsec2 = (RECTS_INTERSECT(r->x, r->y, r->w, r->h, xx, yy, ww, hh));
		       if (intsec1 ^ intsec2)
			 rects = evas_list_append(rects, r);
		       else
			 free(r);
		    }
		  else
		    free(r);
	       }
	     rl = evas_list_free(rl);
	  }

     }
   return rects;
}
