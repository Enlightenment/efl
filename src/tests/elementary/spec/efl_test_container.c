#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

void
efl_test_container_content_equal(Efl_Ui_Widget **wid, unsigned int len)
{
   Efl_Ui_Widget *c;
   Eina_Iterator *iter;
   unsigned int i;
   Eina_List *tmp = NULL;

   ck_assert_int_eq(efl_content_count(widget), len);
   iter = efl_content_iterate(widget);
   EINA_ITERATOR_FOREACH(iter, c)
     {
        ck_assert_ptr_eq(eina_list_data_find(tmp, c), NULL);
        tmp = eina_list_append(tmp, c);
     }
   eina_iterator_free(iter);
   for (i = 0; i < len; ++i)
     {
        ck_assert_ptr_ne(eina_list_data_find(tmp, wid[i]), NULL);
        tmp = eina_list_remove(tmp, wid[i]);
     }
   ck_assert_int_eq(eina_list_count(tmp), 0);
}

typedef struct {
   Eina_Bool *flag;
   void *expected_event_data;
} Efl_Container_Event_Content;

static void
_evt_called(void *data, const Efl_Event *ev)
{
   Efl_Container_Event_Content *content = data;

   *content->flag = EINA_TRUE;
   ck_assert_ptr_eq(content->expected_event_data, ev->info);
   efl_event_callback_del(ev->object, ev->desc, _evt_called, data);
   free(data);
}

void
efl_test_container_expect_evt_content_added(Efl_Ui_Widget *widget, const Efl_Event_Description *ev, Eina_Bool *flag, void *event_data)
{
   Efl_Container_Event_Content *c = calloc(1, sizeof(Efl_Container_Event_Content));
   c->flag = flag;
   c->expected_event_data = event_data;
   efl_event_callback_add(widget, ev, _evt_called, c);
}
