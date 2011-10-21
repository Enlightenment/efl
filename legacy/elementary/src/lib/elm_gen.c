#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "elm_gen.h"

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
   Evas_Coord        pan_x, pan_y;
   Eina_Bool         reorder_mode : 1;
   Eina_Bool         on_hold : 1;
   Eina_Bool         multi : 1;
   Eina_Bool         no_select : 1;
   Eina_Bool         wasselected : 1;
   Eina_Bool         always_select : 1;
};

static const char *
_item_label_hook(Elm_Gen_Item *it, const char *part)
{
   if (!it->itc->func.label_get) return NULL;
   return edje_object_part_text_get(VIEW(it), part);
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
/******************************************************************************/
void
elm_gen_item_unrealize(Elm_Gen_Item *it,
                       Eina_Bool     calc,
                       Ecore_Cb      extra_cb)
{
   Evas_Object *icon;

   if (!it->realized) return;
   if (it->wd->reorder_it == it) return;
   evas_event_freeze(evas_object_evas_get(WIDGET(it)));
   if (!calc)
     evas_object_smart_callback_call(WIDGET(it), "unrealized", it);
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }

   elm_widget_stringlist_free(it->labels);
   it->labels = NULL;
   elm_widget_stringlist_free(it->icons);
   it->icons = NULL;
   elm_widget_stringlist_free(it->states);
   it->states = NULL;

   EINA_LIST_FREE(it->icon_objs, icon)
     evas_object_del(icon);

   if (extra_cb) extra_cb(it);

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
elm_gen_item_del_serious(Elm_Gen_Item *it, Ecore_Cb job)
{
   elm_widget_item_pre_notify_del(it);
   if (it->selected) it->wd->selected = eina_list_remove(it->wd->selected, it);
   it->delete_me = EINA_TRUE;
   if ((!it->delete_me) && (it->itc->func.del))
     it->itc->func.del((void *)it->base.data, WIDGET(it));
   it->wd->items = eina_inlist_remove(it->wd->items, EINA_INLIST_GET(it));
   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   it->wd->walking -= it->walking;
   if (it->long_timer) ecore_timer_del(it->long_timer);
   if (it->group)
     it->wd->group_items = eina_list_remove(it->wd->group_items, it);

   if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
   it->wd->calc_job = ecore_job_add(job, it->wd);
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

   elm_widget_item_text_get_hook_set(it, _item_label_hook);
   return it;
}
/******************************************************************************/
