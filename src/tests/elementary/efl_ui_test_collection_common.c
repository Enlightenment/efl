#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "efl_ui_test_collection_common.h"

Eo *item_container;

void
fill_items(const Efl_Class *klass)
{
  for (int i = 0; i < 3; ++i)
    {
       char buf[PATH_MAX];
       Eo *it = efl_add(klass, item_container);

       snprintf(buf, sizeof(buf), "%d - Test %d", i, i%13);
       efl_text_set(it, buf);
       efl_pack_end(item_container, it);
    }
}

EFL_START_TEST(finalizer_group_middle_insert)
{
   Efl_Ui_Group_Item *group_item = efl_add(EFL_UI_GROUP_ITEM_CLASS, item_container);
   efl_pack_end(item_container, group_item);
   Efl_Ui_Group_Item *i1 = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
   efl_pack_end(group_item, i1);
   Efl_Ui_Group_Item *i2 = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
   efl_pack_end(group_item, i2);
   Efl_Ui_Group_Item *e = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);

   EXPECT_ERROR_START;
   efl_pack_after(item_container, e, group_item);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   efl_pack_after(item_container, e, i1);
   EXPECT_ERROR_END;

   EXPECT_ERROR_START;
   efl_pack_before(item_container, e, i1);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   efl_pack_before(item_container, e, i2);
   EXPECT_ERROR_END;

   EXPECT_ERROR_START;
   efl_pack_at(item_container, e, 1);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   efl_pack_at(item_container, e, 2);
   EXPECT_ERROR_END;

   efl_del(item_container);
}
EFL_END_TEST

void efl_ui_test_item_container_common_add(TCase *tc)
{
   tcase_add_test(tc, finalizer_group_middle_insert);
}
