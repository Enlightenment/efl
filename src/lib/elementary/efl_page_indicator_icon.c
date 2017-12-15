#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_page_indicator_icon.h"
#include "efl_page_indicator.h"
#include "efl_ui_widget_pager.h"

#define MY_CLASS EFL_PAGE_INDICATOR_ICON_CLASS



EOLIAN static void
_efl_page_indicator_icon_update(Eo *obj,
                                Efl_Page_Indicator_Icon_Data *pd,
                                double pos)
{
   EFL_PAGE_INDICATOR_DATA_GET(obj, spd);
   Eo *item;
   int page = efl_ui_pager_current_page_get(spd->pager.obj);

   if (pos < 0)
     {
        item = eina_list_nth(pd->items, page);
        eina_value_set(pd->v, (1.0 + pos));
        efl_layout_signal_message_send(item, 1, *(pd->v));

        item = eina_list_nth(pd->items,
                             (page - 1 + spd->cnt) % spd->cnt);
        eina_value_set(pd->v, (pos * -1.0));
        efl_layout_signal_message_send(item, 1, *(pd->v));
     }
   else
     {
        item = eina_list_nth(pd->items, page);
        eina_value_set(pd->v, (1.0 - pos));
        efl_layout_signal_message_send(item, 1, *(pd->v));

        item = eina_list_nth(pd->items,
                             (page + 1 + spd->cnt) % spd->cnt);
        eina_value_set(pd->v, pos);
        efl_layout_signal_message_send(item, 1, *(pd->v));
     }
}

EOLIAN static void
_efl_page_indicator_icon_pack_begin(Eo *obj,
                                    Efl_Page_Indicator_Icon_Data *pd)
{
   EFL_PAGE_INDICATOR_DATA_GET(obj, spd);
   Eo *item;

   efl_page_indicator_pack_begin(efl_super(obj, MY_CLASS));

   item = efl_add(EFL_CANVAS_LAYOUT_CLASS, spd->idbox);
   pd->items = eina_list_prepend(pd->items, item);

   elm_widget_theme_object_set(spd->idbox, item,
                               "pager", "indicator", "default");
   efl_gfx_size_hint_align_set(item, 0.5, 0.5);
   efl_gfx_size_hint_weight_set(item, 0, 0);
   efl_pack_begin(spd->idbox, item);
}

EOLIAN static void
_efl_page_indicator_icon_pack_end(Eo *obj,
                                  Efl_Page_Indicator_Icon_Data *pd)
{
   EFL_PAGE_INDICATOR_DATA_GET(obj, spd);
   Eo *item;

   efl_page_indicator_pack_end(efl_super(obj, MY_CLASS));

   item = efl_add(EFL_CANVAS_LAYOUT_CLASS, spd->idbox);
   pd->items = eina_list_append(pd->items, item);

   elm_widget_theme_object_set(spd->idbox, item,
                               "pager", "indicator", "default");
   efl_gfx_size_hint_align_set(item, 0.5, 0.5);
   efl_gfx_size_hint_weight_set(item, 0, 0);
   efl_pack_end(spd->idbox, item);
}

EOLIAN static void
_efl_page_indicator_icon_pack_before(Eo *obj,
                                     Efl_Page_Indicator_Icon_Data *pd,
                                     int index)
{
   EFL_PAGE_INDICATOR_DATA_GET(obj, spd);
   Eo *item, *existing;

   efl_page_indicator_pack_before(efl_super(obj, MY_CLASS), index);

   existing = eina_list_nth(pd->items, index);

   item = efl_add(EFL_CANVAS_LAYOUT_CLASS, spd->idbox);
   pd->items = eina_list_prepend_relative(pd->items, item, existing);

   elm_widget_theme_object_set(spd->idbox, item,
                               "pager", "indicator", "default");
   efl_gfx_size_hint_align_set(item, 0.5, 0.5);
   efl_gfx_size_hint_weight_set(item, 0, 0);

   efl_pack_before(spd->idbox, item, existing);
}

EOLIAN static void
_efl_page_indicator_icon_pack_after(Eo *obj,
                                    Efl_Page_Indicator_Icon_Data *pd,
                                    int index)
{
   EFL_PAGE_INDICATOR_DATA_GET(obj, spd);
   Eo *item, *existing;

   efl_page_indicator_pack_after(efl_super(obj, MY_CLASS), index);

   existing = eina_list_nth(pd->items, index);

   item = efl_add(EFL_CANVAS_LAYOUT_CLASS, spd->idbox);
   pd->items = eina_list_append_relative(pd->items, item, existing);

   elm_widget_theme_object_set(spd->idbox, item,
                               "pager", "indicator", "default");
   efl_gfx_size_hint_align_set(item, 0.5, 0.5);
   efl_gfx_size_hint_weight_set(item, 0, 0);

   efl_pack_after(spd->idbox, item, existing);
}

EOLIAN static Eo *
_efl_page_indicator_icon_efl_object_constructor(Eo *obj,
                                                Efl_Page_Indicator_Icon_Data *pd)
{
   EFL_PAGE_INDICATOR_DATA_GET(obj, spd);
   Eo *item;
   int i, page;

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_ui_direction_set(spd->idbox, EFL_UI_DIR_HORIZONTAL);
   efl_pack_padding_set(spd->idbox, 15, 15, EINA_TRUE);

   pd->v = eina_value_float_new(0.0);

   if (spd->cnt != 0)
     {
        for (i = 0; i < spd->cnt; i++)
          {
             item = efl_add(EFL_CANVAS_LAYOUT_CLASS, spd->idbox);
             pd->items = eina_list_append(pd->items, item);

             elm_widget_theme_object_set(spd->idbox, item,
                                         "pager", "indicator", "default");
             efl_gfx_size_hint_align_set(item, 0.5, 0.5);
             efl_gfx_size_hint_weight_set(item, 0, 0);
             efl_pack_end(spd->idbox, item);
          }

        page = efl_ui_pager_current_page_get(spd->pager.obj);
        item = eina_list_nth(pd->items, page);
        eina_value_set(pd->v, 1.0);
        efl_layout_signal_message_send(item, 1, *(pd->v));
     }

   return obj;
}

EOLIAN static void
_efl_page_indicator_icon_efl_object_destructor(Eo *obj,
                                               Efl_Page_Indicator_Icon_Data *pd)
{
   Eo *item;

   efl_destructor(efl_super(obj, MY_CLASS));

   EINA_LIST_FREE(pd->items, item)
      efl_del(item);
}


#define EFL_PAGE_INDICATOR_ICON_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_update, \
                      _efl_page_indicator_icon_update), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_pack_begin, \
                      _efl_page_indicator_icon_pack_begin), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_pack_end, \
                      _efl_page_indicator_icon_pack_end), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_pack_before, \
                      _efl_page_indicator_icon_pack_before), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_pack_after, \
                      _efl_page_indicator_icon_pack_after)

#include "efl_page_indicator_icon.eo.c"
