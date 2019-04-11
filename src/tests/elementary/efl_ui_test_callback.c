#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_suite.h"

static Eo *win;

static void
callback_setup()
{
   win = win_add();

   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 50));
}

static void
_btn_visibility_change(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Bool *flag = data;
   Eina_Bool *visible = ev->info;

   if (!(*visible))
     *flag = EINA_TRUE;
}

EFL_START_TEST(efl_ui_callback_visibility)
{
   Eina_Bool check = EINA_FALSE;
   Eo *box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_content_set(win, efl_added));
   efl_event_callback_add(box, EFL_GFX_ENTITY_EVENT_VISIBILITY_CHANGED, _btn_visibility_change, &check);
   efl_gfx_entity_visible_set(box, EINA_FALSE);

   ck_assert_int_eq(check, EINA_TRUE);
}
EFL_END_TEST

void efl_ui_test_callback(TCase *tc)
{
   tcase_add_checked_fixture(tc, callback_setup, NULL);
   tcase_add_test(tc, efl_ui_callback_visibility);
}
