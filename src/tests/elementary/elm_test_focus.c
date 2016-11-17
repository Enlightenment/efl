#include "elm_test_focus_common.h"

START_TEST(focus_unregister_twice)
{
   elm_init(1, NULL);
   Efl_Ui_Focus_Object *r1 = efl_add(FOCUS_TEST_CLASS, NULL);
   Efl_Ui_Focus_Object *r2 = efl_add(FOCUS_TEST_CLASS, NULL);

   Efl_Ui_Focus_Manager *m = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
    efl_ui_focus_manager_root_set(efl_added, r1)
   );

   fail_if(!efl_ui_focus_manager_register(m, r2, r1, NULL));

   efl_ui_focus_manager_unregister(m, r1);
   efl_ui_focus_manager_unregister(m, r1);
   efl_ui_focus_manager_unregister(m, r1);

   efl_del(r2);
   efl_del(r1);
   efl_del(m);

   elm_shutdown();
}
END_TEST

START_TEST(focus_register_twice)
{
   elm_init(1, NULL);

   Efl_Ui_Focus_Object *r1 = elm_focus_test_object_new("r1", 0, 0, 10, 10);
   Efl_Ui_Focus_Object *r2 = elm_focus_test_object_new("r2", 0, 10, 10, 10);

   Efl_Ui_Focus_Manager *m = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
    efl_ui_focus_manager_root_set(efl_added, r1)
   );

   fail_if(!efl_ui_focus_manager_register(m, r2, r1, NULL));
   fail_if(efl_ui_focus_manager_register(m, r2, r1, NULL));

   efl_del(r1);
   efl_del(m);

   elm_shutdown();
}
END_TEST

START_TEST(pos_check)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *middle, *east, *west, *north, *south;

   elm_init(1, NULL);

   elm_focus_test_setup_cross(&middle, &south, &north, &east, &west);

   m = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
    efl_ui_focus_manager_root_set(efl_added, middle)
   );

   efl_ui_focus_manager_register(m, north, middle, NULL);
   efl_ui_focus_manager_register(m, south, middle, NULL);
   efl_ui_focus_manager_register(m, west, middle, NULL);
   efl_ui_focus_manager_register(m, east, middle, NULL);

#define CHECK(obj, r,l,u,d) \
   efl_ui_focus_manager_focus(m, obj); \
   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_RIGHT), r); \
   efl_ui_focus_manager_focus(m, obj); \
   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_LEFT), l); \
   efl_ui_focus_manager_focus(m, obj); \
   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_UP), u); \
   efl_ui_focus_manager_focus(m, obj); \
   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_DOWN), d); \
   efl_ui_focus_manager_focus(m, obj);

   CHECK(middle, east, west, north, south)
   CHECK(east, NULL, middle, NULL, NULL)
   CHECK(west, middle, NULL, NULL, NULL)
   CHECK(north, NULL, NULL, NULL, middle)
   CHECK(south, NULL, NULL, middle, NULL)

   efl_del(middle);
   efl_del(south);
   efl_del(north);
   efl_del(east);
   efl_del(west);

   elm_shutdown();
}
END_TEST

START_TEST(redirect)
{
   elm_init(1, NULL);

   TEST_OBJ_NEW(root, 0, 0, 20, 20);
   TEST_OBJ_NEW(one, 0, 0, 20, 20);
   TEST_OBJ_NEW(two, 20, 0, 20, 20);

   Efl_Ui_Focus_Manager *m = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
    efl_ui_focus_manager_root_set(efl_added, root)
   );

   Efl_Ui_Focus_Manager *m2 = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
    efl_ui_focus_manager_root_set(efl_added, one)
   );

   efl_ui_focus_manager_register(m2, two, one, NULL);

   efl_ui_focus_manager_redirect_set(m, m2);
   efl_ui_focus_manager_focus(m2, one);

   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_RIGHT), two);

   elm_shutdown();
}
END_TEST

START_TEST(border_check)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *middle, *east, *west, *north, *south;
   Eina_List *list = NULL;
   Eina_Iterator *iter;
   Efl_Ui_Focus_Object *obj;

   elm_init(1, NULL);

   elm_focus_test_setup_cross(&middle, &south, &north, &east, &west);

   m = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
      efl_ui_focus_manager_root_set(efl_added, middle)
    );
   efl_ui_focus_manager_register(m, south, middle, NULL);
   efl_ui_focus_manager_register(m, north, middle, NULL);
   efl_ui_focus_manager_register(m, east, middle, NULL);
   efl_ui_focus_manager_register(m, west, middle, NULL);

   iter = efl_ui_focus_manager_border_elements_get(m);

   EINA_ITERATOR_FOREACH(iter, obj)
     {
        list = eina_list_append(list, obj);
     }

   eina_iterator_free(iter);

   ck_assert(eina_list_data_find(list, east) == east);
   ck_assert(eina_list_data_find(list, north) == north);
   ck_assert(eina_list_data_find(list, west) == west);
   ck_assert(eina_list_data_find(list, east) == east);
   ck_assert(eina_list_data_find(list, middle) == NULL);
   ck_assert(eina_list_count(list) == 4);

   elm_shutdown();
}
END_TEST

START_TEST(logical_chain)
{
   Efl_Ui_Focus_Manager *m;
   int i = 0;

   elm_init(1, NULL);

   TEST_OBJ_NEW(root, 0, 0, 20, 20);

   m = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
    efl_ui_focus_manager_root_set(efl_added, root)
   );
   fail_if(!m);

   efl_ui_focus_manager_focus(m, root);

   i++;
   TEST_OBJ_NEW(child1, 0, i*20, 20, 20);
   i++;
   TEST_OBJ_NEW(child2, 0, i*20, 20, 20);
   i++;
   TEST_OBJ_NEW(child3, 0, i*20, 20, 20);

   i++;
   TEST_OBJ_NEW(subchild11, 0, i*20, 20, 20);
   i++;
   TEST_OBJ_NEW(subchild12, 0, i*20, 20, 20);
   i++;
   TEST_OBJ_NEW(subchild13, 0, i*20, 20, 20);

   i++;
   TEST_OBJ_NEW(subchild21, 0, i*20, 20, 20);
   i++;
   TEST_OBJ_NEW(subchild22, 0, i*20, 20, 20);
   i++;
   TEST_OBJ_NEW(subchild23, 0, i*20, 20, 20);

   //register everything
   efl_ui_focus_manager_register(m, child1, root, NULL);
   efl_ui_focus_manager_register(m, child2, root, NULL);
   efl_ui_focus_manager_register_logical(m, child3, root);
   efl_ui_focus_manager_register(m, subchild11, child1, NULL);
   efl_ui_focus_manager_register(m, subchild12, child1, NULL);
   efl_ui_focus_manager_register(m, subchild13, child1, NULL);
   efl_ui_focus_manager_register(m, subchild21, child3, NULL);
   efl_ui_focus_manager_register(m, subchild22, child3, NULL);
   efl_ui_focus_manager_register(m, subchild23, child3, NULL);

   Efl_Object *logical_chain[] = {
    child1, subchild11, subchild12, subchild13,
    child2, subchild21, subchild22, subchild23, root, NULL
   };
   for (i = 0; logical_chain[i]; ++i)
     {
        ck_assert_ptr_eq(logical_chain[i], efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_NEXT));
     }
   i-= 2;
   for (; i > 0; --i)
     {
        ck_assert_ptr_eq(logical_chain[i], efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_PREV));
     }
   elm_shutdown();
}
END_TEST

START_TEST(finalize_check)
{
   Efl_Ui_Focus_Manager *m;

   elm_init(1, NULL);

   m = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL);
   fail_if(m);

   elm_shutdown();
}
END_TEST

START_TEST(redirect_param)
{
   Efl_Ui_Focus_Manager *m, *m2;

   elm_init(1, NULL);

   TEST_OBJ_NEW(root, 0, 20, 20, 20);
   TEST_OBJ_NEW(root2, 0, 20, 20, 20);
   TEST_OBJ_NEW(child, 0, 20, 20, 20);

   m = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
    efl_ui_focus_manager_root_set(efl_added, root)
   );

   m2 = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
    efl_ui_focus_manager_root_set(efl_added, root2)
   );

   efl_ui_focus_manager_register(m, child, root, m2);
   efl_ui_focus_manager_focus(m, child);

   ck_assert_ptr_eq(efl_ui_focus_manager_redirect_get(m), m2);

   elm_shutdown();
}
END_TEST

START_TEST(invalid_args_check)
{
   Efl_Ui_Focus_Manager *m;

   elm_init(1, NULL);

   TEST_OBJ_NEW(root, 0, 20, 20, 20);
   TEST_OBJ_NEW(child, 0, 20, 20, 20);
   TEST_OBJ_NEW(child2, 0, 20, 20, 20);

   m = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
    efl_ui_focus_manager_root_set(efl_added, root)
   );

   //no child and no parent
   ck_assert_int_eq(efl_ui_focus_manager_register(m, NULL, NULL, NULL), 0);
   ck_assert_int_eq(efl_ui_focus_manager_register(m, child, NULL, NULL), 0);
   ck_assert_int_eq(efl_ui_focus_manager_register(m, NULL, root, NULL), 0);

   ck_assert_int_eq(efl_ui_focus_manager_register(m, child, root, NULL), 1);

   ck_assert_int_eq(efl_ui_focus_manager_update_parent(m, child, NULL), 0);
   ck_assert_int_eq(efl_ui_focus_manager_update_parent(m, NULL, NULL), 0);
   ck_assert_int_eq(efl_ui_focus_manager_update_parent(m, child, child2), 0);

   ck_assert_int_eq(efl_ui_focus_manager_register(m, child2, root, NULL), 1);
   ck_assert_int_eq(efl_ui_focus_manager_update_parent(m, child, child2), 1);

   elm_shutdown();
}
END_TEST

START_TEST(order_check)
{
   Efl_Ui_Focus_Manager *m;
   Eina_List *order = NULL;

   elm_init(1, NULL);

   TEST_OBJ_NEW(root, 0, 20, 20, 20);
   TEST_OBJ_NEW(child1, 0, 20, 20, 20);
   TEST_OBJ_NEW(child2, 0, 20, 20, 20);
   TEST_OBJ_NEW(child3, 0, 20, 20, 20);

   m = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
    efl_ui_focus_manager_root_set(efl_added, root)
   );

   //no child and no parent
   efl_ui_focus_manager_register(m, child1, root, NULL);
   efl_ui_focus_manager_register(m, child2, root, NULL);
   efl_ui_focus_manager_register(m, child3, root, NULL);

   //positiv check
   order = eina_list_append(order, child2);
   order = eina_list_append(order, child3);
   order = eina_list_append(order, child1);
   ck_assert_int_eq(efl_ui_focus_manager_update_children(m, root, order), 1);

   eina_list_free(order);
   order = NULL;

   //negativ check
   order = eina_list_append(order, child1);
   order = eina_list_append(order, child2);
   ck_assert_int_eq(efl_ui_focus_manager_update_children(m, root, order), 0);

   elm_shutdown();
}
END_TEST
void elm_test_focus(TCase *tc)
{
    tcase_add_test(tc, focus_register_twice);
    tcase_add_test(tc, focus_unregister_twice);
    tcase_add_test(tc, pos_check);
    tcase_add_test(tc, redirect);
    tcase_add_test(tc, border_check);
    tcase_add_test(tc, finalize_check);
    tcase_add_test(tc, logical_chain);
    tcase_add_test(tc, redirect_param);
    tcase_add_test(tc, invalid_args_check);
    tcase_add_test(tc, order_check);
}
