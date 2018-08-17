#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "els_box.h"

/* calculate an object's aspected size */
static Eina_Bool
_box_object_aspect_calc(int *ow, int *oh, int minw, int minh, int maxw, int maxh,
                        int fw /* fill width */, int fh /* fill height */,
                        int ww /* "maximum" width */, int hh /* "maximum" height */,
                        Evas_Aspect_Control aspect, double ratio)
{
   *ow = minw;
   *oh = minh;

   switch (aspect)
     {
      case EVAS_ASPECT_CONTROL_HORIZONTAL:
        /* set height using aspect+width */
        if (fw) *ow = ww;
        if ((maxw >= 0) && (maxw < *ow)) *ow = maxw;
        *oh = (1 / ratio) * *ow;
        /* apply min/max */
        if ((maxh >= 0) && (maxh < *oh)) *oh = maxh;
        else if ((minh >= 0) && (minh > *oh)) *oh = minh;
        else return EINA_TRUE;
        return EINA_FALSE;
      case EVAS_ASPECT_CONTROL_VERTICAL:
        /* set width using aspect+height */
        if (fh) *oh = hh;
        if ((maxh >= 0) && (maxh < *oh)) *oh = maxh;
        *ow = ratio * *oh;
        /* apply min/max */
        if ((maxw >= 0) && (maxw < *ow)) *ow = maxw;
        else if ((minw >= 0) && (minw > *ow)) *ow = minw;
        else return EINA_TRUE;
        return EINA_FALSE;
      case EVAS_ASPECT_CONTROL_BOTH:
        /* try width then height */

        /* set width using aspect+height */
        if (fh) *oh = hh;
        if ((maxh >= 0) && (maxh < *oh)) *oh = maxh;
        *ow = ratio * *oh;
        /* apply min/max */
        if ((maxw >= 0) && (maxw < *ow)) *ow = maxw;
        else if ((minw >= 0) && (minw > *ow)) *ow = minw;
        else return EINA_TRUE;
        /* set height using aspect+width */
        if (fw) *ow = ww;
        if ((maxw >= 0) && (maxw < *ow)) *ow = maxw;
        *oh = (1 / ratio) * *ow;
        /* apply min/max */
        if ((maxh >= 0) && (maxh < *oh)) *oh = maxh;
        else if ((minh >= 0) && (minh > *oh)) *oh = minh;
        else return EINA_TRUE;
        /* fallthrough on BOTH failure */
      default: break;
     }
   /* on failure or ASPECT_NONE, use default size calc:
    * - apply fill
    * - apply max size constraints
    */
   if (fw) *ow = ww;
   if ((maxw >= 0) && (maxw < *ow)) *ow = maxw;
   if (fh) *oh = hh;
   if ((maxh >= 0) && (maxh < *oh)) *oh = maxh;
   return EINA_FALSE;
}

/* add box w/h padding to min/max totals */
static void
_smart_extents_padding_calc(Evas_Object_Box_Data *priv, int *minw, int *minh, int *maxw, int *maxh, Eina_Bool horizontal)
{
   int c;

   if ((*maxw >= 0) && (*minw > *maxw)) *maxw = *minw;
   if ((*maxh >= 0) && (*minh > *maxh)) *maxh = *minh;
   c = eina_list_count(priv->children) - 1;
   if (c > 0)
     {
        if (horizontal)
          {
             *minw += priv->pad.h * c;
             if (*maxw != -1) *maxw += priv->pad.h * c;
          }
        else
          {
             *minh += priv->pad.v * c;
             if (*maxh != -1) *maxh += priv->pad.v * c;
          }
     }
}

/* calculate extents for non-homogeneous layout;
 * called twice if aspected items exist
 */
static Eina_Bool
_smart_extents_non_homogeneous_calc(Evas_Object_Box_Data *priv, int w, int h, int *minw, int *minh, int *maxw, int *maxh, double expand, Eina_Bool horizontal, Eina_Bool do_aspect)
{
   const Eina_List *l;
   Evas_Object_Box_Option *opt;
   int mnw, mnh, mxw, mxh, cminw, cminh;
   Evas_Coord pad_l, pad_r, pad_t, pad_b;
   Evas_Coord *rw, *rh, *rxw, *rxh, *rminw, *rminh, *rmaxw, *rmaxh;
   Eina_Bool max = EINA_TRUE, asp = EINA_FALSE;

   cminw = *minw, cminh = *minh;
   *minw = *minh = 0;
   *maxw = *maxh = -1;
   /* use pointers to values to simplify horizontal vs vertical calculations into
    * a single algorithm for both orientations
    */
   if (!horizontal)
     {
        rw = &mnw;
        rh = &mnh;
        rxw = &mxw;
        rxh = &mxh;
        rminw = minw;
        rminh = minh;
        rmaxw = maxw;
        rmaxh = maxh;
     }
   else
     {
        rw = &mnh;
        rh = &mnw;
        rxw = &mxh;
        rxh = &mxw;
        rminw = minh;
        rminh = minw;
        rmaxw = maxh;
        rmaxh = maxw;
     }
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Aspect_Control aspect = EVAS_ASPECT_CONTROL_NONE;
        int asx, asy, ow = 0, oh = 0, *rrw, *rrh;

        if (!horizontal)
          rrw = &ow, rrh = &oh;
        else
          rrw = &oh, rrh = &ow;

        evas_object_size_hint_padding_get(opt->obj, &pad_l, &pad_r, &pad_t, &pad_b);
        evas_object_size_hint_combined_min_get(opt->obj, &mnw, &mnh);
        mnw += pad_l + pad_r;
        mnh += pad_t + pad_b;
        if (*rminw < *rw) *rminw = *rw;
        *rminh += *rh;

        evas_object_size_hint_aspect_get(opt->obj, &aspect, &asx, &asy);
        if (aspect && ((!EINA_DBL_NONZERO(asx)) || (asx < 0.0) ||
          (!EINA_DBL_NONZERO(asy)) || (asy < 0.0)))
          {
             aspect = EVAS_ASPECT_CONTROL_NONE;
             ERR("Invalid aspect specified!");
          }
        /* return whether any aspected items exist */
        asp |= !!aspect;

        evas_object_size_hint_max_get(opt->obj, &mxw, &mxh);
        if (mxw >= 0) mxw += pad_l + pad_r;
        if (mxh >= 0) mxh += pad_t + pad_b;
        if (*rxh < 0)
          {
             *rmaxh = -1;
             max = EINA_FALSE;
          }
        if (max) *rmaxh += *rxh;

        if (do_aspect && aspect)
          {
             int ww, hh, fw = 0, fh = 0;
             double wx, wy, ax, ay;

             evas_object_size_hint_weight_get(opt->obj, &wx, &wy);

             if (horizontal)
               {
                  /* use min size to start */
                  ww = mnw;
                  if ((expand > 0) && (wx > 0.0))
                    {
                       /* add remaining container value after applying weight hint */
                       ow = ((w - cminw) * wx) / expand;
                       ww += ow;
                    }
                  hh = h;
               }
             else
               {
                  hh = mnh;
                  if ((expand > 0) && (wy > 0.0))
                    {
                       oh = ((h - cminh) * wy) / expand;
                       hh += oh;
                    }
                  ww = w;
               }
             evas_object_size_hint_align_get(opt->obj, &ax, &ay);
             if (ax < 0) fw = 1;
             if (ay < 0) fh = 1;

             /* if aspecting succeeds, use aspected size for min size */
             if (_box_object_aspect_calc(&ow, &oh, mnw, mnh, mxw, mxh,
                 fw, fh, ww, hh, aspect, asx / (double)asy))
               {
                  *rminh += (*rrh - *rh);
                  if (*rminw < *rrw) *rminw = *rrw;
               }
          }
        if (*rxw >= 0)
          {
             if (*rmaxw == -1) *rmaxw = *rxw;
             else if (*rmaxw > *rxw) *rmaxw = *rxw;
          }
     }
   return asp;
}

static void
_smart_extents_calculate(Evas_Object *box, Evas_Object_Box_Data *priv, int w, int h, double expand, Eina_Bool horizontal, Eina_Bool homogeneous)
{
   Evas_Coord minw, minh, child_mxw, child_mxh, maxw, maxh;
   Evas_Coord pad_l, pad_r, pad_t, pad_b;
   const Eina_List *l;
   Evas_Object_Box_Option *opt;
   int c;

   minw = 0;
   minh = 0;
   maxw = -1;
   maxh = -1;
   c = eina_list_count(priv->children);
   if (homogeneous || (c == 1))
     {
        Evas_Aspect_Control paspect = -1; //causes overflow
        int pasx = -1, pasy = -1;

        EINA_LIST_FOREACH(priv->children, l, opt)
          {
             Evas_Aspect_Control aspect = EVAS_ASPECT_CONTROL_NONE;
             int asx, asy, ow = 0, oh = 0, fw, fh, ww, hh;
             double ax, ay;

             evas_object_size_hint_align_get(opt->obj, &ax, &ay);
             if (ax < 0) fw = 1;
             if (ay < 0) fh = 1;

             evas_object_size_hint_padding_get(opt->obj, &pad_l, &pad_r, &pad_t, &pad_b);
             evas_object_size_hint_combined_min_get(opt->obj, &child_mxw, &child_mxh);
             child_mxw += pad_l + pad_r;
             child_mxh += pad_t + pad_b;
             if (minh < child_mxh) minh = child_mxh;
             if (minw < child_mxw) minw = child_mxw;

             evas_object_size_hint_aspect_get(opt->obj, &aspect, &asx, &asy);
             if (aspect && ((asx < 1) || (asy < 1)))
               {
                  aspect = EVAS_ASPECT_CONTROL_NONE;
                  ERR("Invalid aspect specified!");
               }
             if ((unsigned) paspect < 100) //value starts overflowed as UINT_MAX
               {
                  /* this condition can cause some items to not be the same size,
                   * resulting in a non-homogeneous homogeneous layout
                   */
                  if ((aspect != paspect) || (asx != pasx) || (asy != pasy))
                    ERR("Homogeneous box with differently-aspected items!");
               }

             evas_object_size_hint_max_get(opt->obj, &child_mxw, &child_mxh);
             if (child_mxh >= 0)
               {
                  child_mxh += pad_t + pad_b;
                  if (maxh == -1) maxh = child_mxh;
                  else if (maxh > child_mxh) maxh = child_mxh;
               }
             if (child_mxw >= 0)
               {
                  child_mxw += pad_l + pad_r;
                  if (maxw == -1) maxw = child_mxw;
                  else if (maxw > child_mxw) maxw = child_mxw;
               }
             if (aspect)
               {
                  if (horizontal)
                    {
                       ww = ((w - (c - 1) * priv->pad.h) / c);
                       hh = h;
                    }
                  else
                    {
                       hh = ((h - (c - 1) * priv->pad.v) / c);
                       ww = w;
                    }
                  if (_box_object_aspect_calc(&ow, &oh, child_mxw, child_mxh, maxw, maxh,
                    fw, fh, ww, hh, aspect, asx / (double)asy))
                    {
                       if ((oh > child_mxh) && (minh < oh)) minh = oh;
                       if ((ow > child_mxw) && (minw < ow)) minw = ow;
                    }
               }

             paspect = aspect;
             pasx = asx, pasy = asy;
          }
        if (horizontal)
          {
             minw *= c;
             if (maxw != -1)
                maxw *= c;
             else maxw = -1;
          }
        else
          {
             minh *= c;
             if (maxh != -1)
                maxh *= c;
             else maxh = -1;
          }
     }
   else
     {
        /* returns true if at least one item has aspect hint */
        if (_smart_extents_non_homogeneous_calc(priv, w, h, &minw, &minh, &maxw, &maxh, expand, horizontal, 0))
          {
             /* aspect can only be accurately calculated after the full (non-aspected) min size of the box has
              * been calculated due to the use of this min size during aspect calculations
              */
             int aminw = minw;
             int aminh = minh;
             _smart_extents_padding_calc(priv, &minw, &minh, &maxw, &maxh, horizontal);
             _smart_extents_non_homogeneous_calc(priv, w, h, &aminw, &aminh, &maxw, &maxh, expand, horizontal, 1);
             if (horizontal) minh = aminh;
             else minw = aminw;
          }
     }
   _smart_extents_padding_calc(priv, &minw, &minh, &maxw, &maxh, horizontal);
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

   evas_object_geometry_get(o, &x, &y, &w, &h);
   /* accummulate expand after switched x and y for horizontal mode */
   if (!horizontal)
     rwy = &wy;
   else
     rwy = &wx;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        evas_object_size_hint_weight_get(opt->obj, &wx, &wy);
        if (*rwy > 0.0) expand += *rwy;
     }
   _smart_extents_calculate(o, priv, w, h, expand, horizontal, homogeneous);
   evas_object_geometry_get(o, &x, &y, &w, &h);

   evas_object_size_hint_combined_min_get(o, &minw, &minh);
   evas_object_box_align_get(o, &ax, &ay);
   /* if object size is less than min, apply align to trigger viewporting */
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
        Evas_Coord pad_l, pad_r, pad_t, pad_b;
        int fw, fh, xw, xh;//fillw, fillw, expandw, expandh
        Evas_Aspect_Control aspect = EVAS_ASPECT_CONTROL_NONE;
        int asx, asy;

        obj = opt->obj;
        evas_object_size_hint_align_get(obj, &ax, &ay);
        evas_object_size_hint_weight_get(obj, &wx, &wy);
        evas_object_size_hint_padding_get(obj, &pad_l, &pad_r, &pad_t, &pad_b);
        evas_object_size_hint_combined_min_get(obj, &mnw, &mnh);
        mnw += pad_l + pad_r;
        mnh += pad_t + pad_b;
        evas_object_size_hint_max_get(obj, &mxw, &mxh);
        if (mxw >= 0) mxw += pad_l + pad_r;
        if (mxh >= 0) mxh += pad_t + pad_b;
        evas_object_size_hint_aspect_get(obj, &aspect, &asx, &asy);
        if (aspect && ((asx < 1) || (asy < 1)))
          aspect = EVAS_ASPECT_CONTROL_NONE;
        fw = fh = 0;
        xw = xh = 0;
        /* align(-1) means fill to maximum apportioned size */
        if (ax == -1.0) {fw = 1; ax = 0.5;}
        if (ay == -1.0) {fh = 1; ay = 0.5;}
        if (rtl) ax = 1.0 - ax;
        if (wx > 0.0) xw = 1;
        if (wy > 0.0) xh = 1;
        if (horizontal)
          {
             Evas_Coord ww, hh, ow = 0, oh = 0;

             if (homogeneous)
               {
                  ww = ((w - (count - 1) * priv->pad.h) / (Evas_Coord)count);
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

             _box_object_aspect_calc(&ow, &oh, mnw, mnh, mxw, mxh, fw, fh, ww, hh, aspect, asx / (double)asy);
             /* non-homogeneous, aspected, expending items are calculated based on object size
              * during extents calc, so use this for positioning during layout as well
              */
             if (xw && aspect && (!homogeneous))
               ww = ow;
             evas_object_move(obj,
                              ((!rtl) ? (xx + pad_l) : (x + (w - (xx - x) - ww) + pad_r))
                              + (Evas_Coord)(((double)(ww - ow)) * ax),
                              yy + (Evas_Coord)(((double)(hh - oh)) * ay) + pad_t);
             ow -= pad_l + pad_r;
             oh -= pad_t + pad_b;
             evas_object_resize(obj, ow, oh);
             xx += ww;
             xx += priv->pad.h;
          }
        else
          {
             Evas_Coord ww, hh, ow = 0, oh = 0;

             if (homogeneous)
               {
                  hh = ((h - (count - 1) * priv->pad.v) / (Evas_Coord)count);
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

             _box_object_aspect_calc(&ow, &oh, mnw, mnh, mxw, mxh, fw, fh, ww, hh, aspect, asx / (double)asy);
             if (xh && aspect && (!homogeneous))
               hh = oh;
             evas_object_move(obj,
                              xx + (Evas_Coord)(((double)(ww - ow)) * ax) + pad_l,
                              yy + (Evas_Coord)(((double)(hh - oh)) * ay) + pad_t);
             ow -= pad_l + pad_r;
             oh -= pad_t + pad_b;
             evas_object_resize(obj, ow, oh);
             yy += hh;
             yy += priv->pad.v;
          }
     }
}
