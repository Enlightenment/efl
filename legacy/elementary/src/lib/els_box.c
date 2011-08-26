#include <Elementary.h>
#include "elm_priv.h"

static void
_smart_extents_calculate(Evas_Object *box, Evas_Object_Box_Data *priv, int horizontal, int homogeneous)
{
   Evas_Coord minw, minh, mnw, mnh;
   const Eina_List *l;
   Evas_Object_Box_Option *opt;
   int c;

   /* FIXME: need to calc max */
   minw = 0;
   minh = 0;
   if (homogeneous)
     {
        EINA_LIST_FOREACH(priv->children, l, opt)
          {
             evas_object_size_hint_min_get(opt->obj, &mnw, &mnh);
             if (minh < mnh) minh = mnh;
             if (minw < mnw) minw = mnw;
          }
        if (horizontal)
          minw *= eina_list_count(priv->children);
        else
          minh *= eina_list_count(priv->children);
     }
   else
     {
        EINA_LIST_FOREACH(priv->children, l, opt)
          {
             evas_object_size_hint_min_get(opt->obj, &mnw, &mnh);
             if (horizontal)
               {
                  if (minh < mnh) minh = mnh;
                  minw += mnw;
               }
             else
               {
                  if (minw < mnw) minw = mnw;
                  minh += mnh;
               }
          }
     }
   c = eina_list_count(priv->children) - 1;
   if (c > 0)
     {
        if (horizontal) minw += priv->pad.h * c;
        else            minh += priv->pad.v * c;
     }
   evas_object_size_hint_min_set(box, minw, minh);
}

void
_els_box_layout(Evas_Object *o, Evas_Object_Box_Data *priv, int horizontal, int homogeneous, int rtl)
{
   Evas_Coord x, y, w, h, xx, yy;
   const Eina_List *l;
   Evas_Object *obj;
   Evas_Coord minw, minh, wdif, hdif;
   int count = 0, expand = 0;
   double ax, ay;
   Evas_Object_Box_Option *opt;

   _smart_extents_calculate(o, priv, horizontal, homogeneous);
   if (evas_object_smart_need_recalculate_get(o)) return;

   evas_object_geometry_get(o, &x, &y, &w, &h);

   evas_object_size_hint_min_get(o, &minw, &minh);
   evas_object_size_hint_align_get(o, &ax, &ay);
   count = eina_list_count(priv->children);
   if (rtl) ax = 1.0 - ax;

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
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        double wx, wy;

        evas_object_size_hint_weight_get(opt->obj, &wx, &wy);
        if (horizontal)
          {
             if (wx > 0.0) expand++;
          }
        else
          {
             if (wy > 0.0) expand++;
          }
     }
   if (!expand)
     {
        evas_object_size_hint_align_get(o, &ax, &ay);
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
   wdif = w - minw;
   hdif = h - minh;
   xx = x;
   yy = y;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Coord mnw, mnh, mxw, mxh;
        double wx, wy;
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
                       if (expand == 1) ow = wdif;
                       else ow = (w - minw) / expand;
                       wdif -= ow;
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
                       if (expand == 1) oh = hdif;
                       else oh = (h - minh) / expand;
                       hdif -= oh;
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

