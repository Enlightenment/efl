#ifndef EFL_UI_TEST_ITEM_CONTAINER_COMMON_H
#define EFL_UI_TEST_ITEM_CONTAINER_COMMON_H 1

extern Eo *item_container;
extern Eo *position_manager;

void fill_items(const Efl_Class *klass);
void efl_ui_test_item_container_common_add(TCase *tc);
void efl_ui_test_position_manager_common_add(TCase *tc);

#endif
