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

void efl_ui_test_item_container_common_add(TCase *tc EINA_UNUSED)
{
}
