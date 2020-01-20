#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"
#include "eo_internal.h"

Evas_Object *win = NULL;
Evas_Object *widget = NULL;
Eo *collection_grid = NULL;
const Efl_Class *test_content_klass = NULL;
const Efl_Class *widget_klass = NULL;
EFL_CLASS_SIMPLE_CLASS(efl_ui_widget, "Efl.Ui.Widget", EFL_UI_WIDGET_CLASS);
EFL_CLASS_SIMPLE_CLASS(efl_ui_item, "Efl.Ui.Item", EFL_UI_ITEM_CLASS);
#define EFL_UI_ITEM_REALIZED_CLASS efl_ui_item_realized_class_get()


#define NUM_TEST_WIDGETS 30
static Eina_Hash *test_widgets;

static void
_setup_window_and_widget(const Efl_Class *klass, const Efl_Class *content_klass)
{
   ck_assert(!win);
   ck_assert(!widget);

   test_content_klass = content_klass;
   widget_klass = klass;
   win = win_add();
   widget = efl_add(klass, win);
   ck_assert(efl_content_set(win, widget));

   ck_assert(win);
   ck_assert(widget);
   efl_wref_add(widget, &widget);
   efl_wref_add(win, &win);
}

Eo*
create_test_widget(void)
{
   static unsigned int i;

   if (is_forked())
     {
        Eo **widgets = eina_hash_find(test_widgets, &test_content_klass);
        ck_assert_int_lt(i, NUM_TEST_WIDGETS);
        ck_assert(widgets[i]);
        return widgets[i++];
     }
   Eo *ret = efl_add(test_content_klass, win);

   return ret;
}

static void
spec_suite_setup(Eo *global_win)
{
   const Efl_Class *test_classes[] =
   {
    EFL_UI_GRID_DEFAULT_ITEM_CLASS,
    EFL_UI_LIST_DEFAULT_ITEM_CLASS,
    EFL_UI_RADIO_CLASS,
    EFL_UI_TAB_BAR_DEFAULT_ITEM_CLASS,
    WIDGET_CLASS
   };
   collection_grid = efl_add(EFL_UI_GRID_CLASS, global_win);
   test_widgets = eina_hash_pointer_new(NULL);
   for (unsigned int t = 0; t < EINA_C_ARRAY_LENGTH(test_classes); t++)
     {
        Eo **widgets = malloc(NUM_TEST_WIDGETS * sizeof(void*));
        for (unsigned int i = 0; i < NUM_TEST_WIDGETS; i++)
          efl_wref_add(efl_add(test_classes[t], global_win), &widgets[i]);
        eina_hash_set(test_widgets, &test_classes[t], widgets);
     }
}

#include "efl_ui_spec_suite_gen.x"
int
main(int argc, char **argv)
{
   int failed_count;

#define LIST_DECL
#include "efl_ui_spec_suite_gen.x"

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   suite_setup_cb_set(spec_suite_setup);

   failed_count = suite_setup(EINA_FALSE);

   failed_count += _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Efl_Ui_Spec", etc, SUITE_INIT_FN(elm2), SUITE_SHUTDOWN_FN(elm));

   return (failed_count == 0) ? 0 : 255;
}
