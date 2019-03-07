#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>

static void
_bt_clicked(void *data EINA_UNUSED, const Efl_Event *ev)
{
   printf("click went through on %p\n", ev->object);
}

static void
_scroll_start_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Position2D pos = efl_ui_scrollable_content_pos_get(ev->object);
   printf("scroll start: %p x: %d y: %d\n", ev->object, pos.x, pos.y);
}

static void
_scroll_stop_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Position2D pos = efl_ui_scrollable_content_pos_get(ev->object);
   printf("scroll stop: %p x: %d y: %d\n", ev->object, pos.x, pos.y);
}

void
test_efl_ui_scroller(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *sc, *sc2, *sc3, *bx, *bx2, *gd, *gd2;
   int i, j;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl Ui Scroller"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 400));

   sc = efl_add(EFL_UI_SCROLLER_CLASS, win,
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
                efl_event_callback_add(efl_added, EFL_UI_EVENT_SCROLL_START, _scroll_start_cb, NULL),
                efl_event_callback_add(efl_added, EFL_UI_EVENT_SCROLL_STOP, _scroll_stop_cb, NULL),
                efl_content_set(win, efl_added));

   bx = efl_add(EFL_UI_BOX_CLASS, sc,
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN),
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0),
                efl_gfx_hint_align_set(efl_added, 0.5, 0),
                efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                efl_content_set(sc, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, bx,
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(160, 0)),
           efl_pack(bx, efl_added));

   for (i = 0; i < 3; i++)
      {
        efl_add(EFL_UI_BUTTON_CLASS, bx,
                efl_text_set(efl_added, "Vertical"),
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0.0),
                efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _bt_clicked, NULL),
                efl_pack(bx, efl_added));
      }

   sc2 = efl_add(EFL_UI_SCROLLER_CLASS, bx,
                 efl_ui_scrollable_match_content_set(efl_added, EINA_FALSE, EINA_TRUE),
                 efl_pack(bx, efl_added));

   bx2 = efl_add(EFL_UI_BOX_CLASS, sc2,
                 efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                 efl_content_set(sc2, efl_added));

   for (i = 0; i < 10; i++)
      {
        efl_add(EFL_UI_BUTTON_CLASS, bx2,
                efl_text_set(efl_added, "... Horizontal scrolling ..."),
                efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _bt_clicked, NULL),
                efl_pack(bx2, efl_added));
      }

   for (i = 0; i < 3; i++)
      {
        efl_add(EFL_UI_BUTTON_CLASS, bx,
                efl_text_set(efl_added, "Vertical"),
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0.0),
                efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _bt_clicked, NULL),
                efl_pack(bx, efl_added));
      }

   gd = efl_add(EFL_UI_TABLE_CLASS, bx,
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
                efl_gfx_hint_align_set(efl_added, 0.5, 0),
                efl_pack(bx, efl_added));

   efl_add(EFL_CANVAS_RECTANGLE_CLASS, win,
           efl_gfx_color_set(efl_added, 0, 0, 0, 0),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(200, 120)),
           efl_pack_table(gd, efl_added, 0, 0, 1, 1));

   sc3 = efl_add(EFL_UI_SCROLLER_CLASS, win,
                 efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
                 efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_TRUE),
                 efl_pack_table(gd, efl_added, 0, 0, 1, 1));

   gd2 = efl_add(EFL_UI_TABLE_CLASS, sc3,
                 efl_content_set(sc3, efl_added));

   for (j = 0; j < 16; j++)
     {
        for (i = 0; i < 16; i++)
          {
             efl_add(EFL_UI_BUTTON_CLASS, win,
                     efl_text_set(efl_added, "Both"),
                     efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _bt_clicked, NULL),
                     efl_pack_table(gd2, efl_added, i, j, 1, 1));
          }
     }

   for (i = 0; i < 200; i++)
      {
        efl_add(EFL_UI_BUTTON_CLASS, bx,
                efl_text_set(efl_added, "Vertical"),
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0.0),
                efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _bt_clicked, NULL),
                efl_pack(bx, efl_added));
      }
}
