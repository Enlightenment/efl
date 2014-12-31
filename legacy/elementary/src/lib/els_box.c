#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "els_box.h"

static void
_smart_extents_calculate(Evas_Object *box, Evas_Object_Box_Data *priv, Eina_Bool horizontal, Eina_Bool homogeneous)
{
   Evas_Coord minw, minh, mnw, mnh, maxw, maxh;
   const Eina_List *l;
   Evas_Object_Box_Option *opt;
   Eina_Bool max = EINA_TRUE;
   int c;

   minw = 0;
   minh = 0;
   maxw = -1;
   maxh = -1;
   if (homogeneous)
     {
        EINA_LIST_FOREACH(priv->children, l, opt)
          {
             evas_object_size_hint_min_get(opt->obj, &mnw, &mnh);
             if (minh < mnh) minh = mnh;
             if (minw < mnw) minw = mnw;

             evas_object_size_hint_max_get(opt->obj, &mnw, &mnh);
             if (mnh >= 0)
               {
                  if (maxh == -1) maxh = mnh;
                  else if (maxh > mnh) maxh = mnh;
               }
             if (mnw >= 0)
               {
                  if (maxw == -1) maxw = mnw;
                  else if (maxw > mnw) maxw = mnw;
               }
          }
        if (horizontal)
          {
             minw *= eina_list_count(priv->children);
             if (maxw != -1)
                maxw *= eina_list_count(priv->children);
             else maxw = -1;
          }
        else
          {
             minh *= eina_list_count(priv->children);
             if (maxh != -1)
                maxh *= eina_list_count(priv->children);
             else maxh = -1;
          }
     }
   else
     {
        /* calculate but after switched w and h for horizontal mode */
        Evas_Coord *rw, *rh, *rminw, *rminh, *rmaxw, *rmaxh;
        if (!horizontal)
          {
             rw = &mnw;
             rh = &mnh;
             rminw = &minw;
             rminh = &minh;
             rmaxw = &maxw;
             rmaxh = &maxh;
          }
        else
          {
             rw = &mnh;
             rh = &mnw;
             rminw = &minh;
             rminh = &minw;
             rmaxw = &maxh;
             rmaxh = &maxw;
          }
        EINA_LIST_FOREACH(priv->children, l, opt)
          {
             evas_object_size_hint_min_get(opt->obj, &mnw, &mnh);
             if (*rminw < *rw) *rminw = *rw;
             *rminh += *rh;

             evas_object_size_hint_max_get(opt->obj, &mnw, &mnh);
             if (*rh < 0)
               {
                  *rmaxh = -1;
                  max = EINA_FALSE;
               }
             if (max) *rmaxh += *rh;

             if (*rw >= 0)
               {
                  if (*rmaxw == -1) *rmaxw = *rw;
                  else if (*rmaxw > *rw) *rmaxw = *rw;
               }
          }
     }
   if ((maxw >= 0) && (minw > maxw)) maxw = minw;
   if ((maxh >= 0) && (minh > maxh)) maxh = minh;
   c = eina_list_count(priv->children) - 1;
   if (c > 0)
     {
        if (horizontal)
          {
             minw += priv->pad.h * c;
             if (maxw != -1) maxw += priv->pad.h * c;
          }
        else
          {
             minh += priv->pad.v * c;
             if (maxh != -1) maxh += priv->pad.v * c;
          }
     }
   evas_object_size_hint_min_set(box, minw, minh);
   evas_object_size_hint_max_set(box, maxw, maxh);
}

void
_els_box_layout(Evas_Object *o, Evas_Object_Box_Data *priv, Eina_Bool horizontal, Eina_Bool homogeneous, Eina_Bool rtl)
{
   Evas_Coord x, y, w, h, xx, yy;
   const Eina_List *l;
   Evas_Object *obj;
   Evas_Coord minw, minh;
   int count = 0;
   double expand = 0.0;
   double ax, ay;
   double wx, wy;
   double *rwy;
   Evas_Object_Box_Option *opt;

   _smart_extents_calculate(o, priv, horizontal, homogeneous);

   evas_object_geometry_get(o, &x, &y, &w, &h);

   evas_object_size_hint_min_get(o, &minw, &minh);
   evas_object_box_align_get(o, &ax, &ay);
   if (w < minw)
     {
        x = x + ((w - minw) * (1.0 - ax));
        w = minw;
     }
   if (h < minh)
     {
        y = y + ((h - minh) * (1.0 - ay));
        h = minh;
     }
   count = eina_list_count(priv->children);

   /* accummulate expand as same way but after switched x and y for horizontal mode */
   if (!horizontal)
     rwy = &wy;
   else
     rwy = &wx;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        evas_object_size_hint_weight_get(opt->obj, &wx, &wy);
        if (*rwy > 0.0) expand += *rwy;
     }
   if (!expand)
     {
        if (rtl) ax = 1.0 - ax;
        if (horizontal)
          {
             x += (double)(w - minw) * ax;
             w = minw;
          }
        else
          {
             y += (double)(h - minh) * ay;
             h = minh;
          }
     }
   xx = x;
   yy = y;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Coord mnw, mnh, mxw, mxh;
        int fw, fh, xw, xh;

        obj = opt->obj;
        evas_object_size_hint_align_get(obj, &ax, &ay);
        evas_object_size_hint_weight_get(obj, &wx, &wy);
        evas_object_size_hint_min_get(obj, &mnw, &mnh);
        evas_object_size_hint_max_get(obj, &mxw, &mxh);
        fw = fh = 0;
        xw = xh = 0;
        if (ax == -1.0) {fw = 1; ax = 0.5;}
        if (ay == -1.0) {fh = 1; ay = 0.5;}
        if (rtl) ax = 1.0 - ax;
        if (wx > 0.0) xw = 1;
        if (wy > 0.0) xh = 1;
        if (horizontal)
          {
             Evas_Coord ww, hh, ow, oh;

             if (homogeneous)
               {
                  ww = (w / (Evas_Coord)count);
               }
             else
               {
                  ww = mnw;
                  if ((expand > 0) && (xw))
                    {
                       ow = ((w - minw) * wx) / expand;
                       ww += ow;
                    }
               }
             hh = h;
             ow = mnw;
             if (fw) ow = ww;
             if ((mxw >= 0) && (mxw < ow)) ow = mxw;
             oh = mnh;
             if (fh) oh = hh;
             if ((mxh >= 0) && (mxh < oh)) oh = mxh;
             evas_object_move(obj,
                              ((!rtl) ? (xx) : (x + (w - (xx - x) - ww)))
                              + (Evas_Coord)(((double)(ww - ow)) * ax),
                              yy + (Evas_Coord)(((double)(hh - oh)) * ay));
             evas_object_resize(obj, ow, oh);
             xx += ww;
             xx += priv->pad.h;
          }
        else
          {
             Evas_Coord ww, hh, ow, oh;

             if (homogeneous)
               {
                  hh = (h / (Evas_Coord)count);
               }
             else
               {
                  hh = mnh;
                  if ((expand > 0) && (xh))
                    {
                       oh = ((h - minh) * wy) / expand;
                       hh += oh;
                    }
               }
             ww = w;
             ow = mnw;
             if (fw) ow = ww;
             if ((mxw >= 0) && (mxw < ow)) ow = mxw;
             oh = mnh;
             if (fh) oh = hh;
             if ((mxh >= 0) && (mxh < oh)) oh = mxh;
             evas_object_move(obj,
                              xx + (Evas_Coord)(((double)(ww - ow)) * ax),
                              yy + (Evas_Coord)(((double)(hh - oh)) * ay));
             evas_object_resize(obj, ow, oh);
             yy += hh;
             yy += priv->pad.v;
          }
     }
}
