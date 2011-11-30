#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "els_scroller.h"
#include "elm_gen.h"
#include "elm_genlist.h"

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

/******************************************************************************/

EAPI void
elm_gen_clear(Evas_Object *obj)
{
   elm_genlist_clear(obj);
}

EAPI void
elm_gen_item_selected_set(Elm_Gen_Item *it,
                          Eina_Bool     selected)
{
   elm_genlist_item_selected_set(it, selected);
}

EAPI Eina_Bool
elm_gen_item_selected_get(const Elm_Gen_Item *it)
{
   return elm_genlist_item_selected_get(it);
}

EAPI void
elm_gen_always_select_mode_set(Evas_Object *obj,
                               Eina_Bool    always_select)
{
   elm_genlist_always_select_mode_set(obj, always_select);
}

EAPI Eina_Bool
elm_gen_always_select_mode_get(const Evas_Object *obj)
{
   return elm_genlist_always_select_mode_get(obj);
}

EAPI void
elm_gen_no_select_mode_set(Evas_Object *obj,
                           Eina_Bool    no_select)
{
   elm_genlist_no_select_mode_set(obj, no_select);
}

EAPI Eina_Bool
elm_gen_no_select_mode_get(const Evas_Object *obj)
{
   return elm_genlist_no_select_mode_get(obj);
}

EAPI void
elm_gen_bounce_set(Evas_Object *obj,
                   Eina_Bool    h_bounce,
                   Eina_Bool    v_bounce)
{
   elm_genlist_bounce_set(obj, h_bounce, v_bounce);
}

EAPI void
elm_gen_bounce_get(const Evas_Object *obj,
                   Eina_Bool         *h_bounce,
                   Eina_Bool         *v_bounce)
{
   elm_genlist_bounce_get(obj, h_bounce, v_bounce);
}

EAPI void
elm_gen_page_relative_set(Evas_Object *obj,
                              double   h_pagerel,
                              double   v_pagerel)
{
   _elm_genlist_page_relative_set(obj, h_pagerel, v_pagerel);
}

EAPI void
elm_gen_page_relative_get(const Evas_Object *obj, double *h_pagerel, double *v_pagerel)
{
   _elm_genlist_page_relative_get(obj, h_pagerel, v_pagerel);
}

EAPI void
elm_gen_page_size_set(Evas_Object *obj,
                      Evas_Coord   h_pagesize,
                      Evas_Coord   v_pagesize)
{
   _elm_genlist_page_size_set(obj, h_pagesize, v_pagesize);
}

EAPI void
elm_gen_current_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber)
{
   _elm_genlist_current_page_get(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gen_last_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber)
{
   _elm_genlist_last_page_get(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gen_page_show(const Evas_Object *obj, int h_pagenumber, int v_pagenumber)
{
   _elm_genlist_page_show(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gen_page_bring_in(const Evas_Object *obj, int h_pagenumber, int v_pagenumber)
{
   _elm_genlist_page_bring_in(obj, h_pagenumber, v_pagenumber);
}

EAPI Elm_Gen_Item *
elm_gen_first_item_get(const Evas_Object *obj)
{
   return (Elm_Gen_Item *)elm_genlist_first_item_get(obj);
}

EAPI Elm_Gen_Item *
elm_gen_last_item_get(const Evas_Object *obj)
{
   return (Elm_Gen_Item *)elm_genlist_last_item_get(obj);
}

EAPI Elm_Gen_Item *
elm_gen_item_next_get(const Elm_Gen_Item *it)
{
   return (Elm_Gen_Item *)elm_genlist_item_next_get(it);
}

EAPI Elm_Gen_Item *
elm_gen_item_prev_get(const Elm_Gen_Item *it)
{
   return (Elm_Gen_Item *)elm_genlist_item_prev_get(it);
}

EAPI Evas_Object *
elm_gen_item_widget_get(const Elm_Gen_Item *it)
{
   return _elm_genlist_item_widget_get(it);
}
