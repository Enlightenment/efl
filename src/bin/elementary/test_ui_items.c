#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>

static Efl_Ui_Widget*
_item_add(Eo *box, const Efl_Class *c, int i)
{
   Eo *check, *rect, *il = efl_add(c, box);
   int r = 0, g = 0, b = 0;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%d - Test %d", i, i%13);
   efl_text_set(il, buf);

   rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS, il);
   switch (i % 5)
     {
      case 0:
         r = 255;
         break;
      case 1:
         g = 255;
         break;
      case 2:
         b = 255;
         break;
      case 3:
         r = g = b = 255;
         break;
      case 4:
         r = g = b = 0;
         break;
   }
   efl_gfx_color_set(rect, r, g, b, 255);
   efl_content_set(il, rect);

   check = efl_add(EFL_UI_CHECK_CLASS, il);
   efl_content_set(efl_part(il, "extra"), check);

   if (c == EFL_UI_GRID_DEFAULT_ITEM_CLASS)
     efl_gfx_hint_size_min_set(il, EINA_SIZE2D(100, 180));
   else
     efl_gfx_hint_size_min_set(il, EINA_SIZE2D(40, 40+(i%2)*40));

   return il;
}

void test_efl_ui_item(void *data EINA_UNUSED,
                      Efl_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   Eo *win, *box, *o;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Item examples"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE)
                );

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_content_set(win, efl_added));

   o = _item_add(box, EFL_UI_GRID_DEFAULT_ITEM_CLASS, 1);
   efl_pack_end(box, o);

   o = _item_add(box, EFL_UI_GRID_DEFAULT_ITEM_CLASS, 2);
   efl_ui_widget_disabled_set(o, EINA_TRUE);
   efl_pack_end(box, o);

   o = _item_add(box, EFL_UI_LIST_DEFAULT_ITEM_CLASS, 3);
   efl_pack_end(box, o);

   o = _item_add(box, EFL_UI_LIST_DEFAULT_ITEM_CLASS, 4);
   efl_ui_widget_disabled_set(o, EINA_TRUE);
   efl_pack_end(box, o);

   o = efl_add(EFL_UI_LIST_PLACEHOLDER_ITEM_CLASS, box);
   efl_gfx_hint_size_min_set(o, EINA_SIZE2D(40, 40+40));
   efl_pack_end(box, o);

   o = efl_add(EFL_UI_LIST_PLACEHOLDER_ITEM_CLASS, box);
   efl_gfx_hint_size_min_set(o, EINA_SIZE2D(40, 40));
   efl_ui_widget_disabled_set(o, EINA_TRUE);
   efl_pack_end(box, o);

   o = _item_add(box, EFL_UI_GROUP_ITEM_CLASS, 5);
   efl_gfx_hint_size_min_set(o, EINA_SIZE2D(40, 40+40));
   efl_pack_end(box, o);

   o = _item_add(box, EFL_UI_GROUP_ITEM_CLASS, 6);
   efl_gfx_hint_size_min_set(o, EINA_SIZE2D(40, 40));
   efl_ui_widget_disabled_set(o, EINA_TRUE);

   o = _item_add(box, EFL_UI_TAB_BAR_DEFAULT_ITEM_CLASS, 5);
   efl_gfx_hint_size_min_set(o, EINA_SIZE2D(40, 40+40));
   efl_pack_end(box, o);

   o = _item_add(box, EFL_UI_TAB_BAR_DEFAULT_ITEM_CLASS, 6);
   efl_gfx_hint_size_min_set(o, EINA_SIZE2D(40, 40));
   efl_ui_widget_disabled_set(o, EINA_TRUE);
   efl_pack_end(box, o);
   o = _item_add(box, EFL_UI_TAB_BAR_DEFAULT_ITEM_CLASS, 5);
   efl_ui_tab_bar_default_item_icon_set(o, "folder");
   efl_gfx_hint_size_min_set(o, EINA_SIZE2D(40, 40+40));
   efl_pack_end(box, o);
}
