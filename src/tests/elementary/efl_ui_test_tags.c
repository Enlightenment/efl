#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"

static void
_expand_state_changed(void *data, const Efl_Event *ev)
{
   Eina_Bool *last = data;

   *last = *((Eina_Bool *)ev->info);
}

/* The event says whether the list is expanded now: it carries a bool, not the
 * inverse of the state it names. */
EFL_START_TEST(efl_ui_tags_expand_state_changed_payload)
{
   Eina_Bool last = EINA_TRUE;
   Eina_Array *items;
   Eo *win, *tags;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   tags = efl_add(EFL_UI_TAGS_CLASS, win);
   efl_event_callback_add(tags, EFL_UI_TAGS_EVENT_EXPAND_STATE_CHANGED,
                          _expand_state_changed, &last);
   efl_gfx_entity_size_set(tags, EINA_SIZE2D(200, 100));

   items = eina_array_new(2);
   eina_array_push(items, "one");
   eina_array_push(items, "two");
   efl_ui_tags_items_set(tags, items);
   eina_array_free(items);

   efl_ui_tags_expanded_set(tags, EINA_FALSE);
   ck_assert(!last);
   efl_ui_tags_expanded_set(tags, EINA_TRUE);
   ck_assert(last);
}
EFL_END_TEST

void efl_ui_test_tags(TCase *tc)
{
   tcase_add_test(tc, efl_ui_tags_expand_state_changed_payload);
}
