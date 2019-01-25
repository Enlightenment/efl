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
   double delta = fabs(pos);

   if (pd->curr)
     {
        eina_value_set(pd->v, 0.0);
        efl_layout_signal_message_send(pd->curr, 1, *(pd->v));
        if (pd->adj) efl_layout_signal_message_send(pd->adj, 1, *(pd->v));
     }

   item = eina_list_nth(pd->items, page);
   eina_value_set(pd->v, (1.0 - delta));
   efl_layout_signal_message_send(item, 1, *(pd->v));
   pd->curr = item;

   if (pos < 0)
     item = eina_list_nth(pd->items, (page - 1 + spd->cnt) % spd->cnt);
   else
     item = eina_list_nth(pd->items, (page + 1 + spd->cnt) % spd->cnt);

   eina_value_set(pd->v, delta);
   efl_layout_signal_message_send(item, 1, *(pd->v));
   pd->adj = item;
}

EOLIAN static void
_efl_page_indicator_icon_pack(Eo *obj,
                              Efl_Page_Indicator_Icon_Data *pd,
                              int index)
{
   EFL_PAGE_INDICATOR_DATA_GET(obj, spd);
   Eo *item, *existing;

   item = efl_add(EFL_CANVAS_LAYOUT_CLASS, spd->idbox);
   elm_widget_theme_object_set(spd->idbox, item,
                               "pager", "indicator", "default");
   efl_gfx_size_hint_align_set(item, 0.5, 0.5);
   efl_gfx_size_hint_weight_set(item, 0, 0);

   if (index == spd->cnt)
     {
        pd->items = eina_list_append(pd->items, item);
        efl_pack_end(spd->idbox, item);
     }
   else
     {
        existing = eina_list_nth(pd->items, index);
        pd->items = eina_list_prepend_relative(pd->items, item, existing);
        efl_pack_before(spd->idbox, item, existing);
     }

   efl_page_indicator_pack(efl_super(obj, MY_CLASS), index);
}

EOLIAN static void
_efl_page_indicator_icon_unpack(Eo *obj,
                                Efl_Page_Indicator_Icon_Data *pd,
                                int index)
{
   EFL_PAGE_INDICATOR_DATA_GET(obj, spd);
   Eo *item;

   item = eina_list_nth(pd->items, index);
   pd->items = eina_list_remove(pd->items, item);
   efl_pack_unpack(spd->idbox, item);
   efl_del(item);

   if (index == spd->curr_idx) pd->curr = NULL;

   efl_page_indicator_unpack(efl_super(obj, MY_CLASS), index);

   if ((pd->curr == NULL) && (spd->curr_idx != -1))
     {
        pd->curr = eina_list_nth(pd->items, spd->curr_idx);
        eina_value_set(pd->v, 1.0);
        efl_layout_signal_message_send(pd->curr, 1, *(pd->v));
     }
}

EOLIAN static void
_efl_page_indicator_icon_efl_page_indicator_bind(Eo *obj,
                                                 Efl_Page_Indicator_Icon_Data *pd,
                                                 Eo *pager,
                                                 Efl_Canvas_Group *idbox)
{
   EFL_PAGE_INDICATOR_DATA_GET(obj, spd);
   Eo *item;
   int i, page;

   if (spd->pager.obj)
     {
        EINA_LIST_FREE(pd->items, item)
          {
             efl_del(item);
          }
     }

   efl_page_indicator_bind(efl_super(obj, MY_CLASS), pager, idbox);

   if (spd->pager.obj)
     {
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

             pd->curr = item;
          }
     }
}


#define EFL_PAGE_INDICATOR_ICON_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_update, \
                      _efl_page_indicator_icon_update), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_pack, \
                      _efl_page_indicator_icon_pack), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_unpack, \
                      _efl_page_indicator_icon_unpack)

#include "efl_page_indicator_icon.eo.c"
