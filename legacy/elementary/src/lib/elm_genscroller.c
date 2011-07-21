#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

/**
 * @defgroup Genscroller Genscroller
 *
 */

typedef struct _Widget_Data Widget_Data;
typedef struct _Pan         Pan;

struct _Widget_Data
{
   Evas_Object      *obj, *scr, *pan_smart;
   Pan              *pan;
   Evas_Coord        pan_x, pan_y, minw, minh;

   struct {
      int w, h;
      Evas_Coord total_w, total_h;
   } cells;
};

struct _Pan
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data                   *wd;
};

static const char *widtype = NULL;
static void      _del_hook(Evas_Object *obj);
static void      _mirrored_set(Evas_Object *obj,
                               Eina_Bool    rtl);
static void      _theme_hook(Evas_Object *obj);
static void      _show_region_hook(void        *data,
                                   Evas_Object *obj);
static void      _sizing_eval(Evas_Object *obj);
static void      _on_focus_hook(void        *data,
                                Evas_Object *obj);
static Eina_Bool _event_hook(Evas_Object       *obj,
                             Evas_Object       *src,
                             Evas_Callback_Type type,
                             void              *event_info);
static void      _signal_emit_hook(Evas_Object *obj,
                                   const char *emission,
                                   const char *source);
static void      _pan_calculate(Evas_Object *obj);

static Evas_Smart_Class _pan_sc = EVAS_SMART_CLASS_INIT_VERSION;

static const char SIG_SCROLL_EDGE_TOP[] = "scroll,edge,top";
static const char SIG_SCROLL_EDGE_BOTTOM[] = "scroll,edge,bottom";
static const char SIG_SCROLL_EDGE_LEFT[] = "scroll,edge,left";
static const char SIG_SCROLL_EDGE_RIGHT[] = "scroll,edge,right";

static const Evas_Smart_Cb_Description _signals[] = {
      {SIG_SCROLL_EDGE_TOP, ""},
      {SIG_SCROLL_EDGE_BOTTOM, ""},
      {SIG_SCROLL_EDGE_LEFT, ""},
      {SIG_SCROLL_EDGE_RIGHT, ""},
      {NULL, NULL}
};

static Eina_Bool
_event_hook(Evas_Object       *obj,
            Evas_Object       *src __UNUSED__,
            Evas_Callback_Type type,
            void              *event_info)
{
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;

   elm_smart_scroller_child_pos_get(wd->scr, &x, &y);
   elm_smart_scroller_step_size_get(wd->scr, &step_x, &step_y);
   elm_smart_scroller_page_size_get(wd->scr, &page_x, &page_y);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &v_w, &v_h);

   if ((!strcmp(ev->keyname, "Left")) || (!strcmp(ev->keyname, "KP_Left")))
     {
        x -= step_x;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            (!strcmp(ev->keyname, "KP_Right")))
     {
        x += step_x;
     }
   else if ((!strcmp(ev->keyname, "Up")) || (!strcmp(ev->keyname, "KP_Up")))
     {
        y -= step_y;
     }
   else if ((!strcmp(ev->keyname, "Down")) || (!strcmp(ev->keyname, "KP_Down")))
     {
        y += step_y;
     }
   else return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   elm_smart_scroller_child_pos_set(wd->scr, x, y);
   return EINA_TRUE;
}

static void
_on_focus_hook(void        *data __UNUSED__,
               Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        elm_object_signal_emit(wd->obj, "elm,action,focus", "elm");
        evas_object_focus_set(wd->obj, EINA_TRUE);
     }
   else
     {
        elm_object_signal_emit(wd->obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->obj, EINA_FALSE);
     }
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool    rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_mirrored_set(wd->scr, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_event_freeze(evas_object_evas_get(wd->obj));
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   elm_smart_scroller_object_theme_set(obj, wd->scr, "genscroller", "base",
                                       elm_widget_style_get(obj));
   edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

static void
_show_region_hook(void        *data,
                  Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y, w, h;
   if (!wd) return;
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   //x & y are screen coordinates, Add with pan coordinates
   x += wd->pan_x;
   y += wd->pan_y;
   elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord vmw, vmh;
   if (!wd) return;
   evas_object_size_hint_min_get(wd->scr, &minw, &minh);
   evas_object_size_hint_max_get(wd->scr, &maxw, &maxh);
   edje_object_size_min_calc
      (elm_smart_scroller_edje_object_get(wd->scr), &vmw, &vmh);
   minw = vmw;
   minh = vmh;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_signal_emit_hook(Evas_Object *obj,
                  const char  *emission,
                  const char  *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_signal_emit(elm_smart_scroller_edje_object_get(wd->scr),
                           emission, source);
}

static void
_pan_set(Evas_Object *obj,
         Evas_Coord   x,
         Evas_Coord   y)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if ((x == sd->wd->pan_x) && (y == sd->wd->pan_y)) return;
   sd->wd->pan_x = x;
   sd->wd->pan_y = y;
   // FIXME: pan virtual scroll pos set
   printf("PAN SET: %i %i\n", x, y);
}

static void
_pan_get(Evas_Object *obj,
         Evas_Coord  *x,
         Evas_Coord  *y)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (x) *x = sd->wd->pan_x;
   if (y) *y = sd->wd->pan_y;
}

static void
_pan_max_get(Evas_Object *obj,
             Evas_Coord  *x,
             Evas_Coord  *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = sd->wd->minw - ow;
   if (ow < 0) ow = 0;
   oh = sd->wd->minh - oh;
   if (oh < 0) oh = 0;
   if (x) *x = ow;
   if (y) *y = oh;
}

static void
_pan_min_get(Evas_Object *obj __UNUSED__,
             Evas_Coord  *x,
             Evas_Coord  *y)
{
   if (x) *x = 0;
   if (y) *y = 0;
}

static void
_pan_child_size_get(Evas_Object *obj,
                    Evas_Coord  *w,
                    Evas_Coord  *h)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (w) *w = sd->wd->minw;
   if (h) *h = sd->wd->minh;
}

static void
_pan_add(Evas_Object *obj)
{
   Pan *sd;
   Evas_Object_Smart_Clipped_Data *cd;

   _pan_sc.add(obj);
   cd = evas_object_smart_data_get(obj);
   sd = ELM_NEW(Pan);
   if (!sd) return;
   sd->__clipped_data = *cd;
   free(cd);
   evas_object_smart_data_set(obj, sd);
}

static void
_pan_del(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (!sd) return;
   _pan_sc.del(obj);
}

static void
_pan_resize(Evas_Object *obj,
            Evas_Coord   w,
            Evas_Coord   h)
{
//   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   // FIXME: pan resized
   printf("PAN SIZE: %i %i\n", w, h);
}

static void
_pan_calculate(Evas_Object *obj)
{
//   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ox, oy, ow, oh, cvx, cvy, cvw, cvh;

   evas_event_freeze(evas_object_evas_get(obj));
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   evas_output_viewport_get(evas_object_evas_get(obj), &cvx, &cvy, &cvw, &cvh);
   // FIXME: move/resize/show/realize/unrealize stuff
   printf("PAN CALC\n");
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_pan_move(Evas_Object *obj __UNUSED__,
          Evas_Coord   x,
          Evas_Coord   y)
{
//   Pan *sd = evas_object_smart_data_get(obj);
   // FIXME: pan moved
   printf("PAN MOVE: %i %i\n", x, y);
}

static void
_hold_on(void        *data __UNUSED__,
         Evas_Object *obj,
         void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 1);
}

static void
_hold_off(void        *data __UNUSED__,
          Evas_Object *obj,
          void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 0);
}

static void
_freeze_on(void        *data __UNUSED__,
           Evas_Object *obj,
           void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 1);
}

static void
_freeze_off(void        *data __UNUSED__,
            Evas_Object *obj,
            void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 0);
}

static void
_scroll_edge_left(void        *data,
                  Evas_Object *scr __UNUSED__,
                  void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_SCROLL_EDGE_LEFT, NULL);
}

static void
_scroll_edge_right(void        *data,
                   Evas_Object *scr __UNUSED__,
                   void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_SCROLL_EDGE_RIGHT, NULL);
}

static void
_scroll_edge_top(void        *data,
                 Evas_Object *scr __UNUSED__,
                 void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_SCROLL_EDGE_TOP, NULL);
}

static void
_scroll_edge_bottom(void        *data,
                    Evas_Object *scr __UNUSED__,
                    void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_SCROLL_EDGE_BOTTOM, NULL);
}

/****************************************************************************/

typedef struct _Cell Cell;
typedef struct _Span Span;

struct _Span
{
   Span       *parent;
   Evas_Coord  pos; // position RELATIVE to parent (geom)
   Evas_Coord  size; // size of whole set of children (geom)
   int         total_child_count; // total number of children in all sub trees
   int         child_count; // number of children in children array
   Span      **child; // child array (ordered)
};

/*
 *       ^ PARENT SPAN
 *       |
 *     SPAN
 *     / | \
 *    /  |  \
 *   /   |   \
 *  /    |    \
 * SPAN SPAN SPAN ... N SPAN CHILDREN

 *
 * +-SPAN-SPAN-SPAN-SPAN-SPAN-SPAN
 * |
 * S +--+ +--+ +--+ +--+ +--+ +--+
 * P |  | |  | |  | |  | |  | |  |<- Cell
 * A |  | |  | |  | |  | |  | |  |
 * N +--+ +--+ +--+ +--+ +--+ +--+
 * |
 * S +--+ +--+ +--+ +--+ +--+ +--+
 * P |  | |  | |  | |  | |  | |  |
 * A |  | |  | |  | |  | |  | |  |
 * N +--+ +--+ +--+ +--+ +--+ +--+
 * |
 * S +--+ +--+ +--+ +--+ +--+ +--+
 * P |  | |  | |  | |  | |  | |  |
 * A |  | |  | |  | |  | |  | |  |
 * N +--+ +--+ +--+ +--+ +--+ +--+
 *
 */

static Span *
__span_build(int total, Evas_Coord size, int levels, Evas_Coord pos, int bucketsize)
{
   Span *sp;
   int i, num, bucket;
   Evas_Coord p;

   static int lv = 0;

   sp = calloc(1, sizeof(Span));
   for (i = 0; i < lv; i++) printf(" ");
   printf("SP: %i tot\n", total);
   // FIXME: alloc fail handle
   sp->size = size * total;
   sp->total_child_count = total;
   sp->pos = pos;
   if (bucketsize == 1) return sp;

   // get max number of children per bucket
   num = bucket = (bucketsize + (levels - 1)) / levels;
   sp->child = calloc(levels, sizeof(Span *));
   // FIXME: alloc fail handle
   p = pos;
   for (i = 0; i < levels; i++)
     {
        if (total < num) num = total;
        total -= num;
        if (num <= 0) break;
        lv++;
        sp->child[i] = __span_build(num, size, levels, p - pos, bucket);
        lv--;
        // FIXME: alloc fail handle
        sp->child[i]->parent = sp;
        p += sp->child[i]->size;
        sp->child_count++;
     }
   return sp;
}

static Span *
_span_build(int total, Evas_Coord size, int levels)
{
   // total == total # of leaf nodes (# of cells)
   // size == size of each leaf node (geom)
   // levels == number of children per node (preferred), eg 2, 3, 4, 5 etc.
   int bucketsize = ((total + (levels - 1)) / levels) * levels;
   return __span_build(total, size, levels, 0, bucketsize);
}

static Span *
_span_first(Span *sp)
{
   Span *sp2;

   if (!sp->child) return sp;
   sp2 = _span_first(sp->child[0]);
   return sp2;
}

static Span *
_span_last(Span *sp)
{
   Span *sp2;

   if (!sp->child) return sp;
   sp2 = _span_last(sp->child[sp->child_count - 1]);
   return sp2;
}

static Span *
_span_next(Span *sp)
{
   Span *spp, *spn;
   int i;

   spp = sp->parent;
   if (!spp) return NULL;
   for (i = 0; i < spp->child_count; i++)
     {
        if (spp->child[i] == sp)
          {
             if (i < (spp->child_count - 1)) return spp->child[i + 1];
             else
               {
                  spn = _span_next(spp);
                  if (!spn) return NULL;
                  return _span_first(spn);
               }
          }
     }
   return NULL;
}

static Span *
_span_prev(Span *sp)
{
   Span *spp, *spn;
   int i;

   spp = sp->parent;
   if (!spp) return NULL;
   for (i = 0; i < spp->child_count; i++)
     {
        if (spp->child[i] == sp)
          {
             if (i > 0) return spp->child[i - 1];
             else
               {
                  spn = _span_prev(spp);
                  if (!spn) return NULL;
                  return _span_last(spn);
               }
          }
     }
   return NULL;
}

static Evas_Coord
_span_real_pos_get(Span *sp)
{
   Span *spp;
   Evas_Coord pos = sp->pos;

   for (spp = sp->parent; spp; spp = spp->parent)
      pos += spp->pos;
   return pos;
}

static int
_span_real_num_get(Span *sp)
{
   Span *spp, *spp_prev;
   int i, num = 0;

   for (spp_prev = sp, spp = sp->parent; spp;
        spp_prev = spp, spp = spp->parent)
     {
        if (spp->child)
          {
             for (i = 0; i < spp->child_count; i++)
               {
                  if (spp->child[i] == spp_prev) break;
                  num += spp->child[i]->total_child_count;
               }
          }
     }
   return num;
}

static Span *
_span_num_get(Span *sp, int num)
{
   int i, n, cnt;

   if (num < 0) return NULL;
   if (!sp->child) return sp;
   for (n = 0, i = 0; i < sp->child_count; i++)
     {
        cnt = sp->child[i]->total_child_count;
        n += cnt;
        if (n > num) return _span_num_get(sp->child[i], num - (n - cnt));
     }
   return NULL;
}

static Span *
_span_pos_get(Span *sp, Evas_Coord pos)
{
   int i;
   Evas_Coord p, sz;

   if (pos < 0) return NULL;
   if (!sp->child) return sp;
   for (p = 0, i = 0; i < sp->child_count; i++)
     {
        sz = sp->child[i]->size;
        p += sz;
        if (p > pos) return _span_pos_get(sp->child[i], pos - (p - sz));
     }
   return NULL;
}

static int
__span_del(Span *sp, int num, int count, Evas_Coord *delsize)
{
   int i, n, cnt, reduce = 0, deleted = 0, delstart = -1, num2, done;
   Evas_Coord deleted_size = 0, size;

   if (!sp->child)
     {
        *delsize = sp->size;
        free(sp);
        return 1;
     }
   for (n = 0, i = 0; i < sp->child_count; i++)
     {
        cnt = sp->child[i]->total_child_count;
        n += cnt;
        if (n > num)
          {
             num2 = num - (n - cnt);
             if (num2 < 0) num2 = 0;
             size = 0;
             done = 0;
             if (count > 0)
                done = __span_del(sp->child[i], num2, count, &size);
             deleted_size += size;
             if (i < (sp->child_count - 1))
                sp->child[i + 1]->pos -= deleted_size;
             count -= done;
             if (done == cnt)
               {
                  deleted++;
                  if (delstart == -1) delstart = i;
               }
             reduce += done;
          }
     }
   if (delstart >= 0)
     {
        for (i = delstart; i < sp->child_count; i++)
          {
             if ((i + deleted) < sp->child_count)
                sp->child[i] = sp->child[i + deleted];
          }
     }
   sp->size -= deleted_size;
   sp->child_count -= deleted;
   sp->total_child_count -= reduce;
   if (sp->child_count == 0)
     {
        free(sp->child);
        free(sp);
     }
   *delsize = deleted_size;
   return reduce;
}

static int
_span_del(Span *sp, int num, int count)
{
   Evas_Coord deleted;
   return __span_del(sp, num, count, &deleted);
}

static void
__span_insert(Span *sp, int num, int count, Evas_Coord size, Evas_Coord pos)
{
   Span *sp2;
   int i, j, n, src;

   if (count <= 0) return;
   if (num < 0) return;
next:
   printf("...... hunt for spot at %p, %i %i\n", sp, count, size * count);
   // total child count and size go up by what we are inserting
   sp->total_child_count += count;
   sp->size += size * count;
   // if we have more than 1 child we have to find out which branch to go down
   // or if we have only 1 child AND that child has another child
   if ((sp->child_count > 1) ||
       ((sp->child_count == 1) && (sp->child[0]->child_count >= 1)))
     {
        for (n = 0, i = 0; i < sp->child_count; i++)
          {
             printf("   look in %i\n", i);
             sp2 = sp->child[i];
             n += sp2->total_child_count;
             // if num is within the child we are looking at
             if (n > num)
               {
                  printf("    %i > %i\n", n, num);
                  // advance all children along by size * count
                  for (j = (i + 1); j < sp->child_count; j++)
                     sp->child[j]->pos += (size * count);
                  // now adjust num for new span in child
                  num -= (n - sp2->total_child_count);
                  // and check in new child span and try next child down
                  sp = sp2;
                  goto next;
               }
          }
     }
   printf("FOUND child %p @ %i, %i, cnt %i totcnt %i\n", sp, sp->pos, sp->size, sp->child_count, sp->total_child_count);
   // now that we are just up from a leaf node... do this
   if (!sp->child)
     {
        // no child at all... just fill it in
        sp->child_count = count;
        sp->child = calloc(count, sizeof(Span *));
        sp->size = size * count;
        sp->pos = pos;
        for (i = 0; i < count; i++)
          {
             sp->child[i] = calloc(1, sizeof(Span));
             sp->child[i]->size = size;
             sp->child[i]->pos = sp->pos + (i * size);
             sp->child[i]->total_child_count = 1;
             sp->child[i]->child_count = 0;
             sp->child[i]->child = NULL;
          }
        return;
     }
   else
     {
        // we have some children - find a spot and plug 'er in
        Span **child;

        src = 0;
        // alloc a new child array and copy in old child ptrs from old array
        // up until the insertion point (num)
        child = calloc(count + sp->child_count, sizeof(Span *));
        for (i = 0; i < num; i++)
          {
             child[i] = sp->child[src];
             pos = child[i]->pos + child[i]->size;
             src++;
          }
        // now alloc new children of the right size and stick them in
        for (i = 0; i < count; i++)
          {
             child[num + i] = calloc(1, sizeof(Span));
             sp->child[num + i]->size = size;
             sp->child[num + i]->pos = pos;
             sp->child[num + i]->total_child_count = 1;
             sp->child[num + i]->child_count = 0;
             sp->child[num + i]->child = NULL;
             pos += size;
          }
        // append rest of old children and adjust their pos values
        for (i = num; i < (count + sp->child_count); i++)
          {
             child[count + i] = sp->child[src];
             sp->child[count + i]->pos = pos;
             pos += sp->child[num + count + i]->size;
             src++;
          }
        sp->child_count += count;
        free(sp->child);
        sp->child = child;
     }
}

static void
_span_insert(Span *sp, int num, int count, Evas_Coord size)
{
   __span_insert(sp, num, count, size, 0);
}

static Span *
_span_rebalance(Span *sp, int levels)
{
   // FIXME: do
}

static void
_span_resize(Span *sp, int num, Evas_Coord size)
{
   // FIXME: do
}

/****************************************************************************/

/**
 * Add a new Genscroller object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Genscroller
 */
EAPI Evas_Object *
elm_genscroller_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord minw, minh;
   static Evas_Smart *smart = NULL;

   if (!smart)
     {
        static Evas_Smart_Class sc;

        evas_object_smart_clipped_smart_set(&_pan_sc);
        sc = _pan_sc;
        sc.name = "elm_genscroller_pan";
        sc.version = EVAS_SMART_CLASS_VERSION;
        sc.add = _pan_add;
        sc.del = _pan_del;
        sc.resize = _pan_resize;
        sc.move = _pan_move;
        sc.calculate = _pan_calculate;
        if (!(smart = evas_smart_class_new(&sc))) return NULL;
     }

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "genscroller");
   elm_widget_type_set(obj, "genscroller");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_on_show_region_hook_set(obj, _show_region_hook, obj);

   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_widget_set(wd->scr, obj);
   elm_smart_scroller_object_theme_set(obj, wd->scr, "genscroller", "base",
                                       elm_widget_style_get(obj));
   elm_smart_scroller_bounce_allow_set(wd->scr,
                                       _elm_config->thumbscroll_bounce_enable,
                                       _elm_config->thumbscroll_bounce_enable);
   elm_widget_resize_object_set(obj, wd->scr);

   evas_object_smart_callback_add(wd->scr, "edge,left", _scroll_edge_left, obj);
   evas_object_smart_callback_add(wd->scr, "edge,right", _scroll_edge_right,
                                  obj);
   evas_object_smart_callback_add(wd->scr, "edge,top", _scroll_edge_top, obj);
   evas_object_smart_callback_add(wd->scr, "edge,bottom", _scroll_edge_bottom,
                                  obj);

   wd->obj = obj;

   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);

   wd->pan_smart = evas_object_smart_add(e, smart);
   wd->pan = evas_object_smart_data_get(wd->pan_smart);
   wd->pan->wd = wd;

   elm_smart_scroller_extern_pan_set(wd->scr, wd->pan_smart,
                                     _pan_set, _pan_get, _pan_max_get,
                                     _pan_min_get, _pan_child_size_get);

   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr),
                             &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);

     {
        Span *sp0, *sp;

        sp0 = _span_build(46, 10, 4);
        sp = _span_first(sp0);
        if (sp) printf("first @ %i [%i], size %i\n", sp->pos, _span_real_pos_get(sp), sp->size);
        sp = _span_last(sp0);
        if (sp) printf("last @ %i [%i], size %i\n", sp->pos, _span_real_pos_get(sp), sp->size);
        for (sp = _span_first(sp0); sp; sp = _span_next(sp))
          {
             if (sp) printf("  @ %i [%i], size %i t: %i %i\n",
                            sp->pos,
                            _span_real_pos_get(sp),
                            sp->size,
                            sp->child_count,
                            sp->total_child_count);
          }
        for (sp = _span_last(sp0); sp; sp = _span_prev(sp))
          {
             if (sp) printf("  @ %i [%i], size %i\n", sp->pos, _span_real_pos_get(sp), sp->size);
          }
        sp = _span_num_get(sp0, 0);
        if (sp) printf("sp 0 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));
        sp = _span_num_get(sp0, 1);
        if (sp) printf("sp 1 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));
        sp = _span_num_get(sp0, 7);
        if (sp) printf("sp 7 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));
        sp = _span_num_get(sp0, 39);
        if (sp) printf("sp 39 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));
        sp = _span_num_get(sp0, 44);
        if (sp) printf("sp 44 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));

        sp = _span_pos_get(sp0, -1);
        if (sp) printf("sp pos -1 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));
        sp = _span_pos_get(sp0, 0);
        if (sp) printf("sp pos 0 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));
        sp = _span_pos_get(sp0, 1);
        if (sp) printf("sp pos 1 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));
        sp = _span_pos_get(sp0, 13);
        if (sp) printf("sp pos 13 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));
        sp = _span_pos_get(sp0, 159);
        if (sp) printf("sp pos 159 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));
        sp = _span_pos_get(sp0, 371);
        if (sp) printf("sp pos 371 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));
        sp = _span_pos_get(sp0, 455);
        if (sp) printf("sp pos 455 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));
        sp = _span_pos_get(sp0, 461);
        if (sp) printf("sp pos 461 @ %i [%i]\n", _span_real_pos_get(sp), _span_real_num_get(sp));

        printf("del @13, 11 spans\n");
        _span_del(sp0, 13, 11);
        for (sp = _span_first(sp0); sp; sp = _span_next(sp))
          {
             if (sp) printf("  @ %i [%i], size %i\n", sp->pos, _span_real_pos_get(sp), sp->size);
          }

        printf("add @23, 29 spans, size 20\n");
        _span_insert(sp0, 23, 19, 20);
        for (sp = _span_first(sp0); sp; sp = _span_next(sp))
          {
             if (sp) printf("  @ %i [%i], size %i\n", sp->pos, _span_real_pos_get(sp), sp->size);
          }
     }
   return obj;
}

/**
 * XXX
 *
 * xxx
 *
 * @param obj The genlist object
 *
 * @ingroup Genscroller
 */
EAPI void
elm_genscroller_world_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->minw = w;
   wd->minh = h;
   evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
   _sizing_eval(wd->obj);
   evas_object_smart_changed(wd->pan_smart);
}

EAPI void
elm_genscroller_world_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
}

EAPI void
elm_genscroller_cell_size_set(Evas_Object *obj, int w, int h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->cells.w = w;
   wd->cells.h = h;
}

EAPI void
elm_genscroller_cell_size_get(Evas_Object *obj, int *w, int *h)
{
}

EAPI void
elm_genscroller_cell_all_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
}

EAPI void
elm_genscroller_cell_row_size_set(Evas_Object *obj, int y, Evas_Coord h)
{
}

EAPI void
elm_genscroller_cell_col_size_set(Evas_Object *obj, int x, Evas_Coord w)
{
}

EAPI void
elm_genscroller_cell_rows_insert(Evas_Object *obj, int y, int rows)
{
}

EAPI void
elm_genscroller_cell_rows_del(Evas_Object *obj, int y, int rows)
{
}

EAPI void
elm_genscroller_cell_cols_insert(Evas_Object *obj, int x, int cols)
{
}

EAPI void
elm_genscroller_cell_cols_del(Evas_Object *obj, int x, int cols)
{
}
