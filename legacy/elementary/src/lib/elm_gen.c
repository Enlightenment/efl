#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "els_scroller.h"
#include "elm_gen.h"

/* TEMPORARY */
#undef ELM_CHECK_WIDTYPE
#define ELM_CHECK_WIDTYPE(obj, widtype) \
   if ((!obj) || (!elm_gen_type_check((obj), __func__))) return
#undef ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN
#define ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, ...)                \
   ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
   ELM_CHECK_WIDTYPE(WIDGET((it)), widtype) __VA_ARGS__;

static const char *_gengrid = NULL;
static const char *_genlist = NULL;

struct _Widget_Data
{
   Eina_Inlist_Sorted_State *state;
   Evas_Object      *obj;
   Evas_Object      *scr; /* a smart scroller object which is used internally in genlist */
   Evas_Object      *pan_smart; /* "elm_genlist_pan" evas smart object. this is an extern pan of smart scroller(scr). */
   Eina_List        *selected;
   Eina_List        *group_items;
   Eina_Inlist      *items; /* inlist of all items */
   Elm_Gen_Item     *reorder_it; /* item currently being repositioned */
   Elm_Gen_Item     *last_selected_item;
   Pan              *pan; /* pan_smart object's smart data */
   Ecore_Job        *calc_job;
   int               walking;
   int               item_width, item_height;
   int               group_item_width, group_item_height;
   int               minw, minh;
   long              count;
   Evas_Coord        pan_x, pan_y;
   Eina_Bool         reorder_mode : 1;
   Eina_Bool         on_hold : 1;
   Eina_Bool         multi : 1;
   Eina_Bool         no_select : 1;
   Eina_Bool         wasselected : 1;
   Eina_Bool         always_select : 1;
   Eina_Bool         clear_me : 1;
   Eina_Bool         h_bounce : 1;
   Eina_Bool         v_bounce : 1;
   Ecore_Cb          del_cb, calc_cb, sizing_cb;
   Ecore_Cb          clear_cb;
};

static const char SIG_ACTIVATED[] = "activated";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_SELECTED[] = "selected";
static const char SIG_UNSELECTED[] = "unselected";
static const char SIG_EXPANDED[] = "expanded";
static const char SIG_CONTRACTED[] = "contracted";
static const char SIG_EXPAND_REQUEST[] = "expand,request";
static const char SIG_CONTRACT_REQUEST[] = "contract,request";
static const char SIG_REALIZED[] = "realized";
static const char SIG_UNREALIZED[] = "unrealized";
static const char SIG_DRAG_START_UP[] = "drag,start,up";
static const char SIG_DRAG_START_DOWN[] = "drag,start,down";
static const char SIG_DRAG_START_LEFT[] = "drag,start,left";
static const char SIG_DRAG_START_RIGHT[] = "drag,start,right";
static const char SIG_DRAG_STOP[] = "drag,stop";
static const char SIG_DRAG[] = "drag";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const char SIG_SCROLL_EDGE_TOP[] = "scroll,edge,top"; // TODO : remove this
static const char SIG_SCROLL_EDGE_BOTTOM[] = "scroll,edge,bottom"; // TODO : remove this
static const char SIG_SCROLL_EDGE_LEFT[] = "scroll,edge,left"; // TODO : remove this
static const char SIG_SCROLL_EDGE_RIGHT[] = "scroll,edge,right"; // TODO : remove this
static const char SIG_EDGE_TOP[] = "edge,top";
static const char SIG_EDGE_BOTTOM[] = "edge,bottom";
static const char SIG_EDGE_LEFT[] = "edge,left";
static const char SIG_EDGE_RIGHT[] = "edge,right";
static const char SIG_MULTI_SWIPE_LEFT[] = "multi,swipe,left";
static const char SIG_MULTI_SWIPE_RIGHT[] = "multi,swipe,right";
static const char SIG_MULTI_SWIPE_UP[] = "multi,swipe,up";
static const char SIG_MULTI_SWIPE_DOWN[] = "multi,swipe,down";
static const char SIG_MULTI_PINCH_OUT[] = "multi,pinch,out";
static const char SIG_MULTI_PINCH_IN[] = "multi,pinch,in";
static const char SIG_SWIPE[] = "swipe";
static const char SIG_MOVED[] = "moved";

/* THIS FUNCTION IS HACKY AND TEMPORARY!!! */
Eina_Bool
elm_gen_type_check(const Evas_Object *obj,
                   const char        *func)
{
   const char *provided, *expected = "(unknown)";
   static int abort_on_warn = -1;
   provided = elm_widget_type_get(obj);
   if (!_genlist) _genlist = eina_stringshare_add("genlist");
   if (!_gengrid) _gengrid = eina_stringshare_add("gengrid");
   if (EINA_LIKELY(provided == _genlist) || EINA_LIKELY(provided == _gengrid))
     return EINA_TRUE;
   if ((!provided) || (!provided[0]))
     {
        provided = evas_object_type_get(obj);
        if ((!provided) || (!provided[0]))
          provided = "(unknown)";
     }
   ERR("Passing Object: %p in function: %s, of type: '%s' when expecting type: '%s'", obj, func, provided, expected);
   if (abort_on_warn == -1)
     {
        if (getenv("ELM_ERROR_ABORT")) abort_on_warn = 1;
        else abort_on_warn = 0;
     }
   if (abort_on_warn == 1) abort();
   return EINA_FALSE;
}

static const char *
_item_label_hook(Elm_Gen_Item *it, const char *part)
{
   if (!it->itc->func.label_get) return NULL;
   return edje_object_part_text_get(VIEW(it), part);
}

static Evas_Object *
_item_content_get_hook(Elm_Gen_Item *it, const char *part)
{
   return edje_object_part_swallow_get(VIEW(it), part);
}

static void
_item_content_set_hook(Elm_Gen_Item *it, const char *part, Evas_Object *content)
{
   edje_object_part_swallow(VIEW(it), part, content);
}

static Evas_Object *
_item_content_unset_hook(Elm_Gen_Item *it, const char *part)
{
   return edje_object_part_unswallow(VIEW(it), part);
}

#if 0
static Eina_Bool
_deselect_all_items(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;
   while (wd->selected)
     elm_gengrid_item_selected_set(wd->selected->data, EINA_FALSE);

   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_left(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;

   Elm_Gengrid_Item *prev = elm_gengrid_item_prev_get(wd->last_selected_item);
   if (!prev) return EINA_TRUE;
   if (elm_gengrid_item_selected_get(prev))
     {
        elm_gengrid_item_selected_set(wd->last_selected_item, EINA_FALSE);
        wd->last_selected_item = prev;
        elm_gengrid_item_show(wd->last_selected_item);
     }
   else
     {
        elm_gengrid_item_selected_set(prev, EINA_TRUE);
        elm_gengrid_item_show(prev);
     }

   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_right(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;

   Elm_Gengrid_Item *next = elm_gengrid_item_next_get(wd->last_selected_item);
   if (!next) return EINA_TRUE;
   if (elm_gengrid_item_selected_get(next))
     {
        elm_gengrid_item_selected_set(wd->last_selected_item, EINA_FALSE);
        wd->last_selected_item = next;
        elm_gengrid_item_show(wd->last_selected_item);
     }
   else
     {
        elm_gengrid_item_selected_set(next, EINA_TRUE);
        elm_gengrid_item_show(next);
     }

   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_up(Widget_Data *wd)
{
   unsigned int i;
   Eina_Bool r = EINA_TRUE;

   if (!wd->selected) return EINA_FALSE;

   for (i = 0; (r) && (i < wd->nmax); i++)
     r &= _item_multi_select_left(wd);

   return r;
}

static Eina_Bool
_item_multi_select_down(Widget_Data *wd)
{
   unsigned int i;
   Eina_Bool r = EINA_TRUE;

   if (!wd->selected) return EINA_FALSE;

   for (i = 0; (r) && (i < wd->nmax); i++)
     r &= _item_multi_select_right(wd);

   return r;
}

static Eina_Bool
_item_single_select_up(Widget_Data *wd)
{
   unsigned int i;

   Elm_Gengrid_Item *prev;

   if (!wd->selected)
     {
        prev = ELM_GENGRID_ITEM_FROM_INLIST(wd->items->last);
        while ((prev) && (prev->delete_me))
          prev = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
        elm_gengrid_item_selected_set(prev, EINA_TRUE);
        elm_gengrid_item_show(prev);
        return EINA_TRUE;
     }
   else prev = elm_gengrid_item_prev_get(wd->last_selected_item);

   if (!prev) return EINA_FALSE;

   for (i = 1; i < wd->nmax; i++)
     {
        Elm_Gengrid_Item *tmp = elm_gengrid_item_prev_get(prev);
        if (!tmp) return EINA_FALSE;
        prev = tmp;
     }

   _deselect_all_items(wd);

   elm_gengrid_item_selected_set(prev, EINA_TRUE);
   elm_gengrid_item_show(prev);
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_down(Widget_Data *wd)
{
   unsigned int i;

   Elm_Gengrid_Item *next;

   if (!wd->selected)
     {
        next = ELM_GENGRID_ITEM_FROM_INLIST(wd->items);
        while ((next) && (next->delete_me))
          next = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);
        elm_gengrid_item_selected_set(next, EINA_TRUE);
        elm_gengrid_item_show(next);
        return EINA_TRUE;
     }
   else next = elm_gengrid_item_next_get(wd->last_selected_item);

   if (!next) return EINA_FALSE;

   for (i = 1; i < wd->nmax; i++)
     {
        Elm_Gengrid_Item *tmp = elm_gengrid_item_next_get(next);
        if (!tmp) return EINA_FALSE;
        next = tmp;
     }

   _deselect_all_items(wd);

   elm_gengrid_item_selected_set(next, EINA_TRUE);
   elm_gengrid_item_show(next);
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_left(Widget_Data *wd)
{
   Elm_Gengrid_Item *prev;
   if (!wd->selected)
     {
        prev = ELM_GENGRID_ITEM_FROM_INLIST(wd->items->last);
        while ((prev) && (prev->delete_me))
          prev = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
     }
   else prev = elm_gengrid_item_prev_get(wd->last_selected_item);

   if (!prev) return EINA_FALSE;

   _deselect_all_items(wd);

   elm_gengrid_item_selected_set(prev, EINA_TRUE);
   elm_gengrid_item_show(prev);
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_right(Widget_Data *wd)
{
   Elm_Gengrid_Item *next;
   if (!wd->selected)
     {
        next = ELM_GENGRID_ITEM_FROM_INLIST(wd->items);
        while ((next) && (next->delete_me))
          next = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);
     }
   else next = elm_gengrid_item_next_get(wd->last_selected_item);

   if (!next) return EINA_FALSE;

   _deselect_all_items(wd);

   elm_gengrid_item_selected_set(next, EINA_TRUE);
   elm_gengrid_item_show(next);
   return EINA_TRUE;
}
#endif


static void
_item_select(Elm_Gen_Item *it)
{
   if ((it->wd->no_select) || (it->delete_me) || (it->mode_set)) return;
   if (!it->selected)
     {
        it->selected = EINA_TRUE;
        it->wd->selected = eina_list_append(it->wd->selected, it);
     }
   else if (!it->wd->always_select) return;

   evas_object_ref(WIDGET(it));
   it->walking++;
   it->wd->walking++;
   if (it->func.func) it->func.func((void *)it->func.data, WIDGET(it), it);
   if (!it->delete_me)
     evas_object_smart_callback_call(WIDGET(it), SIG_SELECTED, it);
   it->walking--;
   it->wd->walking--;
   evas_object_unref(WIDGET(it));
   if ((it->wd->clear_me) && (!it->wd->walking))
     elm_gen_clear(WIDGET(it));
   else
     {
        if ((!it->walking) && (it->delete_me))
          {
             if (!it->relcount) it->del_cb(it);
          }
        else
          it->wd->last_selected_item = it;
     }
}
/******************************************************************************/
void
elm_gen_item_unrealize(Elm_Gen_Item *it,
                       Eina_Bool     calc)
{
   Evas_Object *content;

   if (!it->realized) return;
   if (it->wd->reorder_it == it) return;
   evas_event_freeze(evas_object_evas_get(WIDGET(it)));
   if (!calc)
     evas_object_smart_callback_call(WIDGET(it), SIG_UNREALIZED, it);
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }

   elm_widget_stringlist_free(it->labels);
   it->labels = NULL;
   elm_widget_stringlist_free(it->contents);
   it->contents = NULL;
   elm_widget_stringlist_free(it->states);
   it->states = NULL;

   EINA_LIST_FREE(it->content_objs, content)
     evas_object_del(content);

   it->unrealize_cb(it);

   it->realized = EINA_FALSE;
   it->want_unrealize = EINA_FALSE;
   evas_event_thaw(evas_object_evas_get(WIDGET(it)));
   evas_event_thaw_eval(evas_object_evas_get(WIDGET(it)));
}

void
elm_gen_item_del_notserious(Elm_Gen_Item *it)
{
   elm_widget_item_pre_notify_del(it);
   it->delete_me = EINA_TRUE;
   if (it->selected) it->wd->selected = eina_list_remove(it->wd->selected, it);

   if (it->itc->func.del)
     it->itc->func.del((void *)it->base.data, WIDGET(it));
}

void
elm_gen_item_del_serious(Elm_Gen_Item *it)
{
   elm_gen_item_del_notserious(it);
   it->wd->items = eina_inlist_remove(it->wd->items, EINA_INLIST_GET(it));
   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   it->wd->walking -= it->walking;
   if (it->long_timer) ecore_timer_del(it->long_timer);
   if (it->group)
     it->wd->group_items = eina_list_remove(it->wd->group_items, it);

   if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
   it->wd->calc_job = ecore_job_add(it->wd->calc_cb, it->wd);
   free(it->item);
   it->item = NULL;
   elm_widget_item_del(it);
}

Elm_Gen_Item *
elm_gen_item_new(Widget_Data              *wd,
                 const Elm_Gen_Item_Class *itc,
                 const void               *data,
                 Elm_Gen_Item             *parent,
                 Evas_Smart_Cb             func,
                 const void               *func_data)
{
   Elm_Gen_Item *it;

   it = elm_widget_item_new(wd->obj, Elm_Gen_Item);
   if (!it) return NULL;
   it->wd = wd;
   it->itc = itc;
   it->base.data = data;
   it->parent = parent;
   it->func.func = func;
   it->func.data = func_data;
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_unset_hook_set(it, _item_content_unset_hook);
   /* TEMPORARY */
   it->sel_cb = (Ecore_Cb)_item_select;

   elm_widget_item_text_get_hook_set(it, _item_label_hook);
   return it;
}
/******************************************************************************/

EAPI void
elm_gen_item_selected_set(Elm_Gen_Item *it,
                          Eina_Bool     selected)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   Widget_Data *wd = it->wd;
   if (!wd) return;
   if ((it->delete_me) || (it->disabled)) return;
   selected = !!selected;
   if (it->selected == selected) return;

   if (selected)
     {
        if (!wd->multi)
          {
             while (wd->selected)
               {
                  if (it->unhighlight_cb) it->unhighlight_cb(wd->selected->data);
                  it->unsel_cb(wd->selected->data);
               }
          }
        it->highlight_cb(it);
        _item_select(it);
        return;
     }
   if (it->unhighlight_cb) it->unhighlight_cb(it);
   it->unsel_cb(it);
}

EAPI void
elm_gen_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->state)
     {
        eina_inlist_sorted_state_free(wd->state);
        wd->state = NULL;
     }

   if (wd->walking > 0)
     {
        Elm_Gen_Item *it;
        wd->clear_me = 1;
        EINA_INLIST_FOREACH(wd->items, it)
           it->delete_me = 1;
        return;
     }
   evas_event_freeze(evas_object_evas_get(wd->obj));
   while (wd->items)
     {
        Elm_Gen_Item *it = ELM_GEN_ITEM_FROM_INLIST(wd->items);
        it->del_cb(it);
     }
   wd->clear_me = 0;
   if (wd->calc_job)
     {
        ecore_job_del(wd->calc_job);
        wd->calc_job = NULL;
     }
   if (wd->selected) wd->selected = eina_list_free(wd->selected);
   if (wd->clear_cb) wd->clear_cb(wd);
   wd->pan_x = 0;
   wd->pan_y = 0;
   wd->minw = 0;
   wd->minh = 0;
   wd->count = 0;
   if (wd->pan_smart)
     {
        evas_object_size_hint_min_set(wd->pan_smart, wd->minw, wd->minh);
        evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
     }
   if (wd->sizing_cb) wd->sizing_cb(wd->obj);
   elm_smart_scroller_child_region_show(wd->scr, 0, 0, 0, 0);
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

EAPI Eina_Bool
elm_gen_item_selected_get(const Elm_Gen_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, EINA_FALSE);
   return it->selected;
}

EAPI void
elm_gen_always_select_mode_set(Evas_Object *obj,
                                   Eina_Bool    always_select)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->always_select = always_select;
}

EAPI Eina_Bool
elm_gen_always_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->always_select;
}

EAPI void
elm_gen_no_select_mode_set(Evas_Object *obj,
                               Eina_Bool    no_select)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->no_select = no_select;
}

EAPI Eina_Bool
elm_gen_no_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->no_select;
}

EAPI void
elm_gen_bounce_set(Evas_Object *obj,
                       Eina_Bool    h_bounce,
                       Eina_Bool    v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_set(wd->scr, h_bounce, v_bounce);
   wd->h_bounce = h_bounce;
   wd->v_bounce = v_bounce;
}

EAPI void
elm_gen_bounce_get(const Evas_Object *obj,
                       Eina_Bool         *h_bounce,
                       Eina_Bool         *v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (h_bounce) *h_bounce = wd->h_bounce;
   if (v_bounce) *v_bounce = wd->v_bounce;
}

EAPI void
elm_gen_page_relative_set(Evas_Object *obj,
                              double       h_pagerel,
                              double       v_pagerel)
{
   Evas_Coord pagesize_h;
   Evas_Coord pagesize_v;

   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_smart_scroller_paging_get(wd->scr, NULL, NULL, &pagesize_h, &pagesize_v);
   elm_smart_scroller_paging_set(wd->scr, h_pagerel, v_pagerel, pagesize_h,
                                 pagesize_v);
}

EAPI void
elm_gen_page_relative_get(const Evas_Object *obj, double *h_pagerel, double *v_pagerel)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_smart_scroller_paging_get(wd->scr, h_pagerel, v_pagerel, NULL, NULL);
}

EAPI void
elm_gen_page_size_set(Evas_Object *obj,
                          Evas_Coord   h_pagesize,
                          Evas_Coord   v_pagesize)
{
   double pagerel_h;
   double pagerel_v;

   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_paging_get(wd->scr, &pagerel_h, &pagerel_v, NULL, NULL);
   elm_smart_scroller_paging_set(wd->scr, pagerel_h, pagerel_v, h_pagesize,
                                 v_pagesize);
}

EAPI void
elm_gen_current_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_current_page_get(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gen_last_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_last_page_get(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gen_page_show(const Evas_Object *obj, int h_pagenumber, int v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_page_show(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gen_page_bring_in(const Evas_Object *obj, int h_pagenumber, int v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_page_bring_in(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI Elm_Gen_Item *
elm_gen_first_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->items) return NULL;
   Elm_Gen_Item *it = ELM_GEN_ITEM_FROM_INLIST(wd->items);
   while ((it) && (it->delete_me))
     it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
   return it;
}

EAPI Elm_Gen_Item *
elm_gen_last_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->items) return NULL;
   Elm_Gen_Item *it = ELM_GEN_ITEM_FROM_INLIST(wd->items->last);
   while ((it) && (it->delete_me))
     it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
   return it;
}

EAPI Elm_Gen_Item *
elm_gen_item_next_get(const Elm_Gen_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   while (it)
     {
        it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
        if ((it) && (!it->delete_me)) break;
     }
   return (Elm_Gen_Item *)it;
}

EAPI Elm_Gen_Item *
elm_gen_item_prev_get(const Elm_Gen_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   while (it)
     {
        it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
        if ((it) && (!it->delete_me)) break;
     }
   return (Elm_Gen_Item *)it;
}

EAPI Evas_Object *
elm_gen_item_widget_get(const Elm_Gen_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   return WIDGET(it);
}
