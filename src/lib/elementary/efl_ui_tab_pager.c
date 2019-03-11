#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif


#include <Elementary.h>
#include "elm_priv.h"

#include "efl_ui_tab_pager_private.h"
#include "efl_ui_tab_page_private.h"
#include "efl_ui_widget_pager.h"

#define MY_CLASS EFL_UI_TAB_PAGER_CLASS

static void
_select(Eo *obj, int index)
{
   EFL_UI_TAB_PAGER_DATA_GET(obj, sd);
   if (sd->cnt > index) sd->cur = index;
   else sd->cur = 0;

   efl_ui_tab_bar_current_tab_set(sd->tab_bar, sd->cur);
   efl_ui_pager_current_page_set(efl_super(obj, MY_CLASS), sd->cur);
}

static void
_tab_select_cb(void *data, const Efl_Event *event)
{
   int index = (intptr_t)event->info;
   _select(data, index);
}

static void
_tab_changed_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Tab_Page_Tab_Changed_Event *ev = event->info;
   int index;

   EFL_UI_TAB_PAGER_DATA_GET(data, sd);
   EFL_UI_TAB_PAGE_DATA_GET(event->object, pd);
   index = eina_list_data_idx(sd->tab_pages, event->object);

   switch (ev->changed_info)
     {
        case EFL_UI_TAB_PAGE_TAB_CHANGED_LABEL:
          efl_ui_tab_bar_tab_label_set(sd->tab_bar, index, pd->tab_label);
          break;

        case EFL_UI_TAB_PAGE_TAB_CHANGED_ICON:
          efl_ui_tab_bar_tab_icon_set(sd->tab_bar, index, pd->tab_icon);
          break;

        default:
          break;
     }
}

EOLIAN static void
_efl_ui_tab_pager_tab_bar_set(Eo *obj, Efl_Ui_Tab_Pager_Data *sd, Efl_Canvas_Object *tab_bar)
{
   if (sd->tab_bar != NULL)
     {
        efl_event_callback_del(sd->tab_bar, EFL_UI_EVENT_SELECTABLE_SELECTED, _tab_select_cb, obj);
        efl_content_unset(efl_part(obj, "efl.tab_root"));
        efl_del(sd->tab_bar);
     }

   sd->tab_bar = tab_bar;
   efl_content_set(efl_part(obj, "efl.tab_root"), sd->tab_bar);

   efl_event_callback_add(sd->tab_bar, EFL_UI_EVENT_SELECTABLE_SELECTED, _tab_select_cb, obj);
}

EOLIAN static Efl_Canvas_Object *
_efl_ui_tab_pager_tab_bar_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *sd)
{
   return sd->tab_bar;
}

EOLIAN static void
_efl_ui_tab_pager_efl_ui_pager_current_page_set(Eo *obj, Efl_Ui_Tab_Pager_Data *sd EINA_UNUSED, int index)
{
   _select(obj, index);
}

EOLIAN static void
_efl_ui_tab_pager_efl_object_destructor(Eo *obj, Efl_Ui_Tab_Pager_Data *sd)
{
   if (sd->tab_bar != NULL)
     efl_event_callback_del(sd->tab_bar, EFL_UI_EVENT_SELECTABLE_SELECTED, _tab_select_cb, obj);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Object *
_efl_ui_tab_pager_efl_object_constructor(Eo *obj, Efl_Ui_Tab_Pager_Data *sd)
{
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "tab_pager");

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   efl_ui_widget_focus_allow_set(obj, EINA_TRUE);

   sd->tab_bar = NULL;
   sd->cur = 0;
   sd->cnt = 0;

   return obj;
}

EOLIAN static int
_efl_ui_tab_pager_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *sd)
{
   return sd->cnt;
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_pack_clear(Eo *obj, Efl_Ui_Tab_Pager_Data *sd)
{
   if (sd->tab_bar)
     {
        Eo *subobj;
        Eina_List *l, *l_next;
        int begin_index = 0;
        EINA_LIST_FOREACH_SAFE(sd->tab_pages, l, l_next, subobj)
          {
             efl_event_callback_del(subobj, EFL_UI_TAB_PAGE_EVENT_TAB_CHANGED, _tab_changed_cb, obj);

             sd->tab_pages = eina_list_remove(sd->tab_pages, subobj);
             sd->cnt--;

             efl_ui_tab_bar_tab_remove(sd->tab_bar, begin_index);
          }
        efl_pack_clear(efl_super(obj, MY_CLASS));

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_unpack_all(Eo *obj, Efl_Ui_Tab_Pager_Data *sd)
{
   if (sd->tab_bar)
     {
        Eo *subobj;
        Eina_List *l, *l_next;
        int begin_index = 0;
        EINA_LIST_FOREACH_SAFE(sd->tab_pages, l, l_next, subobj)
          {
             efl_event_callback_del(subobj, EFL_UI_TAB_PAGE_EVENT_TAB_CHANGED, _tab_changed_cb, obj);

             sd->tab_pages = eina_list_remove(sd->tab_pages, subobj);
             sd->cnt--;

             efl_ui_tab_bar_tab_remove(sd->tab_bar, begin_index);
          }
        efl_pack_unpack_all(efl_super(obj, MY_CLASS));

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_unpack(Eo *obj, Efl_Ui_Tab_Pager_Data *sd, Efl_Gfx_Entity *subobj)
{
   if (sd->tab_bar)
     {
        efl_event_callback_del(subobj, EFL_UI_TAB_PAGE_EVENT_TAB_CHANGED, _tab_changed_cb, obj);

        int index = eina_list_data_idx(sd->tab_pages, (void *)subobj);

        sd->tab_pages = eina_list_remove(sd->tab_pages, subobj);
        sd->cnt--;

        efl_ui_tab_bar_tab_remove(sd->tab_bar, index);
        efl_pack_unpack(efl_super(obj, MY_CLASS), subobj);


        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_begin(Eo *obj,
                                             Efl_Ui_Tab_Pager_Data *sd,
                                             Efl_Gfx_Entity *subobj)
{
   if (sd->tab_bar)
     {
        EFL_UI_TAB_PAGE_DATA_GET(subobj, pd);
        int begin_index = 0;

        sd->tab_pages = eina_list_prepend(sd->tab_pages, subobj);
        sd->cnt ++;

        efl_ui_tab_bar_tab_add(sd->tab_bar, begin_index, pd->tab_label, pd->tab_icon);
        efl_pack_begin(efl_super(obj, MY_CLASS), subobj);

        sd->cur ++;

        efl_event_callback_add(subobj, EFL_UI_TAB_PAGE_EVENT_TAB_CHANGED, _tab_changed_cb, obj);

        return EINA_TRUE;
     }
   else return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_end(Eo *obj,
                                           Efl_Ui_Tab_Pager_Data *sd,
                                           Efl_Gfx_Entity *subobj)
{
   if (sd->tab_bar)
     {
        EFL_UI_TAB_PAGE_DATA_GET(subobj, pd);
        int end_index = efl_ui_tab_bar_tab_count(sd->tab_bar);

        sd->tab_pages = eina_list_append(sd->tab_pages, subobj);
        sd->cnt ++;

        efl_ui_tab_bar_tab_add(sd->tab_bar, end_index, pd->tab_label, pd->tab_icon);
        efl_pack_end(efl_super(obj, MY_CLASS), subobj);

        efl_event_callback_add(subobj, EFL_UI_TAB_PAGE_EVENT_TAB_CHANGED, _tab_changed_cb, obj);

        return EINA_TRUE;
     }
   else return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_before(Eo *obj,
                                              Efl_Ui_Tab_Pager_Data *sd,
                                              Efl_Gfx_Entity *subobj,
                                              const Efl_Gfx_Entity *existing)
{
   if (sd->tab_bar)
     {
        EFL_UI_TAB_PAGE_DATA_GET(subobj, pd);
        int before_index = eina_list_data_idx(sd->tab_pages, (void *)existing);

        sd->tab_pages = eina_list_prepend_relative(sd->tab_pages, subobj, existing);
        sd->cnt ++;

        efl_ui_tab_bar_tab_add(sd->tab_bar, before_index, pd->tab_label, pd->tab_icon);
        efl_pack_before(efl_super(obj, MY_CLASS), subobj, existing);

        if (sd->cur >= before_index) sd->cur ++;

        efl_event_callback_add(subobj, EFL_UI_TAB_PAGE_EVENT_TAB_CHANGED, _tab_changed_cb, obj);

        return EINA_TRUE;
     }
   else return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_after(Eo *obj,
                                             Efl_Ui_Tab_Pager_Data *sd,
                                             Efl_Gfx_Entity *subobj,
                                             const Efl_Gfx_Entity *existing)
{
   if (sd->tab_bar)
     {
        EFL_UI_TAB_PAGE_DATA_GET(subobj, pd);
        int after_index = eina_list_data_idx(sd->tab_pages, (void *)existing) + 1;

        sd->tab_pages = eina_list_append_relative(sd->tab_pages, subobj, existing);
        sd->cnt ++;

        efl_ui_tab_bar_tab_add(sd->tab_bar, after_index, pd->tab_label, pd->tab_icon);
        efl_pack_after(efl_super(obj, MY_CLASS), subobj, existing);

        if (sd->cur > after_index) sd->cur ++;

        efl_event_callback_add(subobj, EFL_UI_TAB_PAGE_EVENT_TAB_CHANGED, _tab_changed_cb, obj);

        return EINA_TRUE;
     }
   else return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_at(Eo *obj,
                                          Efl_Ui_Tab_Pager_Data *sd,
                                          Efl_Gfx_Entity *subobj,
                                          int index)
{
   if (sd->tab_bar)
     {
        Efl_Gfx_Entity *existing = NULL;
        EFL_UI_TAB_PAGE_DATA_GET(subobj, pd);

        existing = eina_list_nth(sd->tab_pages, index);
        sd->tab_pages = eina_list_prepend_relative(sd->tab_pages, subobj, existing);
        sd->cnt ++;

        efl_ui_tab_bar_tab_add(sd->tab_bar, index, pd->tab_label, pd->tab_icon);
        efl_pack_at(efl_super(obj, MY_CLASS), subobj, index);

        if (sd->cur >= index) sd->cur ++;

        efl_event_callback_add(subobj, EFL_UI_TAB_PAGE_EVENT_TAB_CHANGED, _tab_changed_cb, obj);

        return EINA_TRUE;
     }
   else return EINA_FALSE;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_tab_pager_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Tab_Pager_Data *sd, int index)
{
   if (sd->tab_bar)
     {
        Efl_Gfx_Entity *existing = NULL;
        existing = eina_list_nth(sd->tab_pages, index);
        if (!existing) return NULL;

        efl_event_callback_del(existing, EFL_UI_TAB_PAGE_EVENT_TAB_CHANGED, _tab_changed_cb, obj);

        sd->tab_pages = eina_list_remove(sd->tab_pages, existing);
        sd->cnt--;

        efl_ui_tab_bar_tab_remove(sd->tab_bar, index);
        efl_pack_unpack_at(efl_super(obj, MY_CLASS), index);

        return existing;
     }
   return NULL;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_tab_pager_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED,
                                                   Efl_Ui_Tab_Pager_Data *sd,
                                                   int index)
{
   return eina_list_nth(sd->tab_pages, index);
}

EOLIAN static int
_efl_ui_tab_pager_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                                 Efl_Ui_Tab_Pager_Data *sd,
                                                 const Efl_Gfx_Entity *subobj)
{
   return eina_list_data_idx(sd->tab_pages, (void *)subobj);
}

#include "efl_ui_tab_pager.eo.c"
