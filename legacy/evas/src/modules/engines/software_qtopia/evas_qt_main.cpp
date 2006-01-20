#include "evas_common.h"
#include "evas_engine.h"
#include <qwidget.h>
#include <qnamespace.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qdirectpainter_qws.h>

//#define QVFB_BUG

extern "C" {
   QT_Direct *
     evas_qt_main_direct_start(void *target)
       {
	  QT_Direct *d;
	  QDirectPainter *dp;
	  QWidget *widget;
	  int i;

	  widget = (QWidget *)target;
	  dp = new QDirectPainter(widget);
	  if (!dp) return NULL;
	  d = (QT_Direct *)calloc(1, sizeof(QT_Direct));
	  if (!d) return NULL;
	  d->target = target;
	  d->direct = (void *)dp;
	  d->fb.data = dp->frameBuffer();
	  d->fb.bpp = dp->depth();
	  d->fb.width = dp->lineStep() / (d->fb.bpp / 8);
	  if (dp->depth() == 16)
	    {
	       d->fb.mask.r = 0xf800;
	       d->fb.mask.g = 0x07e0;
	       d->fb.mask.b = 0x001f;
	    }
	  else if (dp->depth() == 32)
	    {
	       d->fb.mask.r = 0x00ff0000;
	       d->fb.mask.g = 0x0000ff00;
	       d->fb.mask.b = 0x000000ff;
	    }
	  else
	    {
	       delete dp;
	       free(d);
	       return NULL;
	    }
	  d->fb.rotation = dp->transformOrientation() * 90;
	  d->rects.count = dp->numRects();
	  if (d->rects.count > 0)
	    {
	       d->rects.r = (QT_Direct_Rect *)calloc(d->rects.count, sizeof(QT_Direct_Rect));
	       if (d->rects.r)
		 {
		    for (i = 0; i < d->rects.count; i++)
		      {
			 QRect r;

			 r = dp->rect(i);
			 if (d->fb.rotation == 0)
			   {
			      d->rects.r[i].x = r.x();
			      d->rects.r[i].y = r.y();
			      d->rects.r[i].w = r.width();
			      d->rects.r[i].h = r.height();
			   }
			 else if (d->fb.rotation == 270)
			   {
			      d->rects.r[i].x = r.y();
			      d->rects.r[i].y = d->fb.width - r.x() - r.width();
			      d->rects.r[i].w = r.height();
			      d->rects.r[i].h = r.width();
			   }
			 else if (d->fb.rotation == 90)
			   {
			      // FIXME: do
			   }
		      }
		 }
	       else
		 d->rects.count = 0;
	    }
	  d->location.x = dp->xOffset();
	  d->location.y = dp->yOffset();
	  d->location.w = dp->width();
	  d->location.h = dp->height();
	  return d;
       }
   void
     evas_qt_main_direct_stop(QT_Direct *d)
       {
	  QDirectPainter *dp;

#ifdef QVFB_BUG
	    {
	       QWidget *target = (QWidget *)d->target;
	       QRect rect(d->updated.x, d->updated.y,
			  d->updated.w, d->updated.h);
	       QPainter p(target);

	       p.fillRect(rect, QBrush::NoBrush);
	    }
#endif
	  dp = (QDirectPainter *)d->direct;
	  delete dp;
	  if (d->rects.r) free(d->rects.r);
	  free(d);
       }
}
