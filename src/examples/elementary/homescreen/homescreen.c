//Compile with:
//gcc -g efl_ui_scroller_example.c -o efl_ui_scroller_example `pkg-config --cflags --libs elementary`
#include <Efl_Ui.h>
#include <Efreet.h>

static Efl_Ui_Spotlight_Container *over_container;

#define SCALE 0.5

typedef struct
{
  Eina_Rect prefered;
  const char *name;
  const char *icon_path;
} Icon;


static Icon workspace1[] = {
    { EINA_RECT(0, 0, 0, 0), "bla", "ic"},
    { EINA_RECT(0, 0, 0, 0), "bla", "ic2"},
    { EINA_RECT(0, 0, 0, 0), NULL, NULL},
};

static Icon workspace2[] = {
    { EINA_RECT(0, 0, 0, 0), "bla", "ic"},
    { EINA_RECT(0, 0, 0, 0), "bla", "ic2"},
    { EINA_RECT(0, 0, 0, 0), NULL, NULL},
};

static Icon workspace3[] = {
    { EINA_RECT(0, 0, 0, 0), "bla", "ic"},
    { EINA_RECT(0, 0, 0, 0), "bla", "ic2"},
    { EINA_RECT(0, 0, 0, 0), NULL, NULL},
};

static Efl_Ui_Widget*
_create_icon(Icon *icon, Eo *parent)
{
   Eo *ret = efl_add(EFL_UI_BUTTON_CLASS, parent);
   efl_text_set(ret, icon->name);
   return ret;
}

static Efl_Ui_Table*
_hs_screen_new(Icon *icons, Eina_Size2D goal_size)
{
   Efl_Ui_Table *table;

   table = efl_add(EFL_UI_TABLE_CLASS, over_container);

   for (int y = 0; y < goal_size.h/(150*SCALE); ++y)
     {
        for (int x = 0; x < goal_size.w/(150*SCALE); ++x)
          {
             Eo *obj = efl_add(EFL_UI_BUTTON_CLASS, table,
               efl_text_set(efl_added, "Bla") );
             efl_pack_table(table, obj, x, y, 1, 1);
          }
     }

   return table;
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win;

   win = efl_new(EFL_UI_WIN_CLASS,
             efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   efl_gfx_entity_size_set(win, EINA_SIZE2D(720*SCALE, 1280*SCALE));


   Efl_Ui_Spotlight_Indicator *indicator = efl_new(EFL_UI_SPOTLIGHT_INDICATOR_ICON_CLASS);
   Efl_Ui_Spotlight_Manager *scroll = efl_new(EFL_UI_SPOTLIGHT_MANAGER_SCROLL_CLASS);

   over_container = efl_add(EFL_UI_SPOTLIGHT_CONTAINER_CLASS, win,
      efl_ui_spotlight_manager_set(efl_added, scroll),
      efl_ui_spotlight_indicator_set(efl_added, indicator)
   );
   efl_gfx_entity_size_set(over_container, EINA_SIZE2D(720*SCALE, 1280*SCALE));

   for (int i = 0; i < 3; ++i)
   {
      Eo *screen = _hs_screen_new(NULL, EINA_SIZE2D(720*SCALE, 1280*SCALE));

      efl_pack_end(over_container, screen);
   }
}
EFL_MAIN()
