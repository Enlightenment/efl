#include "efl_ui_test_focus_common.h"
#include "suite_helpers.h"

EFL_START_TEST(focus_unregister_twice)
{
   Efl_Ui_Focus_Object *r1, *r2;

   Efl_Ui_Focus_Manager *m = focus_test_manager_new(&r1, EINA_TRUE);

   r2 = efl_add_ref(FOCUS_TEST_CLASS, NULL);
   fail_if(!efl_ui_focus_manager_calc_register(m, r2, r1, NULL));

   efl_ui_focus_manager_calc_unregister(m, r1);
   efl_ui_focus_manager_calc_unregister(m, r1);
   efl_ui_focus_manager_calc_unregister(m, r1);

   efl_unref(r2);
   efl_unref(m);

}
EFL_END_TEST

EFL_START_TEST(focus_register_twice)
{
   Efl_Ui_Focus_Object *r1, *r2, *r3;

   Efl_Ui_Focus_Manager *m = focus_test_manager_new(&r1, EINA_TRUE);
   Efl_Ui_Focus_Manager *m2 = focus_test_manager_new(&r3, EINA_TRUE);

   r2 = efl_add_ref(FOCUS_TEST_CLASS, NULL);

   fail_if(!efl_ui_focus_manager_calc_register(m, r2, r1, NULL));
   //same confguration don't error out
   fail_if(!efl_ui_focus_manager_calc_register(m, r2, r1, NULL));

   EXPECT_ERROR_START;
   //different confidurations error out
   fail_if(efl_ui_focus_manager_calc_register(m, r2, r1, m2));
   EXPECT_ERROR_END;

   efl_unref(m);

}
EFL_END_TEST

EFL_START_TEST(pos_check)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *middle, *east, *west, *north, *south, *root;

   focus_test_setup_cross(&middle, &south, &north, &east, &west);

   m = focus_test_manager_new(&root, EINA_TRUE);
   efl_ui_focus_manager_calc_register(m, middle, root, NULL);
   efl_ui_focus_manager_calc_register(m, north, root, NULL);
   efl_ui_focus_manager_calc_register(m, south, root, NULL);
   efl_ui_focus_manager_calc_register(m, west, root, NULL);
   efl_ui_focus_manager_calc_register(m, east, root, NULL);

#define CHECK(obj, r,l,u,d) \
   efl_ui_focus_manager_focus_set(m, obj); \
   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_RIGHT), r); \
   efl_ui_focus_manager_focus_set(m, obj); \
   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_LEFT), l); \
   efl_ui_focus_manager_focus_set(m, obj); \
   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_UP), u); \
   efl_ui_focus_manager_focus_set(m, obj); \
   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_DOWN), d); \
   efl_ui_focus_manager_focus_set(m, obj);

   CHECK(middle, east, west, north, south)
   CHECK(east, NULL, middle, north, south)
   CHECK(west, middle, NULL, north, south)
   CHECK(north, east, west, NULL, middle)
   CHECK(south, east, west, middle, NULL)

   efl_unref(middle);
   efl_unref(south);
   efl_unref(north);
   efl_unref(east);
   efl_unref(west);
   efl_unref(m);
}
EFL_END_TEST

static Eina_Bool
_equal_set(Eina_List *elems, Efl_Ui_Focus_Object *lst[])
{
   unsigned int i = 0;

   for (i = 0; lst[i]; ++i)
     {
        Eina_Bool found = EINA_FALSE;
        Eina_List *n;
        Efl_Ui_Focus_Object *elem;

        EINA_LIST_FOREACH(elems, n, elem)
          {
            if (lst[i] != elem) continue;

            found = EINA_TRUE;
            break;
          }

        if (!found) return EINA_FALSE;
     }

   if (eina_list_count(elems) != i) return EINA_FALSE;
   return EINA_TRUE;
}

EFL_START_TEST(pos_check2)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Relations *rel;
   Efl_Ui_Focus_Object *root, *middle, *north_east, *north_west, *south_east, *south_west;

   middle = focus_test_object_new("middle", 40, 40, 5, 5);

   north_east = focus_test_object_new("north_east", 60, 20, 5, 5);
   north_west = focus_test_object_new("north_west", 20, 20, 5, 5);
   south_east = focus_test_object_new("south_east", 60, 60, 5, 5);
   south_west = focus_test_object_new("south_west", 20, 60, 5, 5);

   m = focus_test_manager_new(&root, EINA_TRUE);
   efl_ui_focus_manager_calc_register(m, middle, root, NULL);
   efl_ui_focus_manager_calc_register(m, north_east, root, NULL);
   efl_ui_focus_manager_calc_register(m, north_west, root, NULL);
   efl_ui_focus_manager_calc_register(m, south_east, root, NULL);
   efl_ui_focus_manager_calc_register(m, south_west, root, NULL);

   rel = efl_ui_focus_manager_fetch(m, middle);

#define ck_assert_set_eq(set, ...) \
   { \
      Efl_Ui_Focus_Object *tmp[] = { __VA_ARGS__ }; \
      ck_assert_int_eq(_equal_set(set, tmp), EINA_TRUE); \
   }

   ck_assert_set_eq(rel->left, north_west, south_west, NULL);
   ck_assert_set_eq(rel->right, north_east, south_east, NULL);
   ck_assert_set_eq(rel->top, north_west, north_east, NULL);
   ck_assert_set_eq(rel->down, south_west, south_east, NULL);

#undef ck_assert_set_eq

   efl_ui_focus_relation_free(rel);

   efl_unref(middle);
   efl_unref(north_east);
   efl_unref(north_west);
   efl_unref(south_east);
   efl_unref(south_west);

}
EFL_END_TEST
EFL_START_TEST(redirect)
{
   Efl_Ui_Focus_Object *root, *root2;
   TEST_OBJ_NEW(one, 0, 0, 20, 20);
   TEST_OBJ_NEW(two, 20, 0, 20, 20);

   Efl_Ui_Focus_Manager *m = focus_test_manager_new(&root, EINA_TRUE);
   Efl_Ui_Focus_Manager *m2 = focus_test_manager_new(&root2, EINA_TRUE);

   efl_ui_focus_manager_calc_register(m2, one, root2, NULL);
   efl_ui_focus_manager_calc_register(m2, two, root2, NULL);

   efl_ui_focus_manager_redirect_set(m, m2);
   efl_ui_focus_manager_focus_set(m2, one);

   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_RIGHT), two);

   efl_unref(m);
   efl_unref(m2);
}
EFL_END_TEST

EFL_START_TEST(border_check)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *middle, *east, *west, *north, *south, *root;
   Eina_List *list = NULL;
   Eina_Iterator *iter;
   Efl_Ui_Focus_Object *obj;

   focus_test_setup_cross(&middle, &south, &north, &east, &west);

   m = focus_test_manager_new(&root, EINA_TRUE);
   efl_ui_focus_manager_calc_register(m, middle, root, NULL);
   efl_ui_focus_manager_calc_register(m, south, root, NULL);
   efl_ui_focus_manager_calc_register(m, north, root, NULL);
   efl_ui_focus_manager_calc_register(m, east, root, NULL);
   efl_ui_focus_manager_calc_register(m, west, root, NULL);

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

}
EFL_END_TEST

EFL_START_TEST(logical_chain)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *lroot;
   int i = 0;

   TEST_OBJ_NEW(root, 0, 0, 20, 20);

   m = focus_test_manager_new(&lroot, EINA_TRUE);

   fail_if(!m);

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
   efl_ui_focus_manager_calc_register(m, root, lroot, NULL);
   efl_ui_focus_manager_calc_register(m, child1, root, NULL);
   efl_ui_focus_manager_calc_register(m, child2, root, NULL);
   efl_ui_focus_manager_calc_register_logical(m, child3, root, NULL);
   efl_ui_focus_manager_calc_register(m, subchild11, child1, NULL);
   efl_ui_focus_manager_calc_register(m, subchild12, child1, NULL);
   efl_ui_focus_manager_calc_register(m, subchild13, child1, NULL);
   efl_ui_focus_manager_calc_register(m, subchild21, child3, NULL);
   efl_ui_focus_manager_calc_register(m, subchild22, child3, NULL);
   efl_ui_focus_manager_calc_register(m, subchild23, child3, NULL);

   efl_ui_focus_manager_focus_set(m, root);

   Efl_Object *logical_chain[] = {
    child1, subchild11, subchild12, subchild13,
    child2, subchild21, subchild22, subchild23, NULL
   };
   for (i = 0; logical_chain[i]; ++i)
     {
        ck_assert_ptr_eq(logical_chain[i], efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_NEXT));
     }
   i-= 2;
   for (; i > 0; --i)
     {
        ck_assert_ptr_eq(logical_chain[i], efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_PREVIOUS));
     }
}
EFL_END_TEST

static void
_check_chain(Efl_Ui_Focus_Manager *m, Efl_Ui_Focus_Object *objects[])
{
   int i;
   for (i = 0; objects[i]; ++i)
     {
        Efl_Ui_Focus_Object *next;

        if (i > 0)
          efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_NEXT);
        else
          {
             Eo *root = efl_ui_focus_manager_root_get(m);
             efl_ui_focus_manager_reset_history(m);
             efl_ui_focus_manager_setup_on_first_touch(m, EFL_UI_FOCUS_DIRECTION_NEXT, root);
          }
        next = efl_ui_focus_manager_focus_get(efl_ui_focus_util_active_manager(m));

        ck_assert_ptr_eq(next, objects[i]);
     }

   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_NEXT), NULL);

   for (; i > 0; i--)
     {
        Efl_Ui_Focus_Object *prev;

        if (!!objects[i])
          efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_PREVIOUS);
        else
          {
             Eo *root = efl_ui_focus_manager_root_get(m);
             efl_ui_focus_manager_reset_history(m);
             efl_ui_focus_manager_setup_on_first_touch(m, EFL_UI_FOCUS_DIRECTION_PREVIOUS, root);
          }

        prev = efl_ui_focus_manager_focus_get(efl_ui_focus_util_active_manager(m));
        ck_assert_ptr_eq(prev, objects[i - 1]);
     }
   ck_assert_ptr_eq(efl_ui_focus_manager_move(m, EFL_UI_FOCUS_DIRECTION_PREVIOUS), NULL);
}

EFL_START_TEST(logical_chain_multi_redirect)
{
   Efl_Ui_Focus_Manager *m, *m2, *m3, *m4;
   Efl_Ui_Focus_Object *root,*root2, *root3, *root4, *c2, *c3, *c4;

   m = focus_test_manager_new(&root, EINA_TRUE);

   //Create 3 linked focus managers
   m2 = focus_test_manager_new(&root2, EINA_FALSE);
   efl_ui_focus_manager_calc_register_logical(m, root2, root, m2);
   focus_test_manager_set(root2, m);

   m3 = focus_test_manager_new(&root3, EINA_FALSE);
   efl_ui_focus_manager_calc_register_logical(m, root3, root, m3);
   focus_test_manager_set(root3, m);

   m4 = focus_test_manager_new(&root4, EINA_FALSE);
   efl_ui_focus_manager_calc_register_logical(m, root4, root, m4);
   focus_test_manager_set(root4, m);

   c2 = focus_test_object_new("child1", 0, 0, 20, 20);
   c3 = focus_test_object_new("child2", 0, 0, 20, 20);
   c4 = focus_test_object_new("child3", 0, 0, 20, 20);
   efl_ui_focus_manager_calc_register(m2, c2, root2, NULL);
   efl_ui_focus_manager_calc_register(m3, c3, root3, NULL);
   efl_ui_focus_manager_calc_register(m4, c4, root4, NULL);

   Efl_Ui_Focus_Object *objects[] = {c2, c3, c4, NULL};
   _check_chain(m, objects);

   efl_unref(m2);
   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(logical_chain_single_redirect)
{
   Efl_Ui_Focus_Manager *m, *m2;
   Efl_Ui_Focus_Object *root,*root2, *c1_1, *c1_2, *c1_3, *c2_1, *c2_2, *c2_3;

   m = focus_test_manager_new(&root, EINA_TRUE);
   m2 = focus_test_manager_new(&root2, EINA_FALSE);
   focus_test_manager_set(root2, m);
   c1_1 = focus_test_object_new("child1", 0, 0, 20, 20);
   c1_2 = root2;
   c1_3 = focus_test_object_new("child3", 0, 0, 20, 20);
   c2_1 = focus_test_object_new("child1", 0, 0, 20, 20);
   c2_2 = focus_test_object_new("child2", 0, 0, 20, 20);
   c2_3 = focus_test_object_new("child3", 0, 0, 20, 20);

   Efl_Ui_Focus_Object *objects[] = {c1_1, c2_1, c2_2, c2_3, c1_3, NULL};

   efl_ui_focus_manager_calc_register(m, c1_1, root, NULL);
   efl_ui_focus_manager_calc_register(m, c1_2, root, m2);
   efl_ui_focus_manager_calc_register(m, c1_3, root, NULL);

   efl_ui_focus_manager_calc_register(m2, c2_1, root2, NULL);
   efl_ui_focus_manager_calc_register(m2, c2_2, root2, NULL);
   efl_ui_focus_manager_calc_register(m2, c2_3, root2, NULL);

   _check_chain(m, objects);

   efl_unref(m2);
   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(finalize_check)
{
   Efl_Ui_Focus_Manager *m;

   EXPECT_ERROR_START;
   m = efl_add_ref(EFL_UI_FOCUS_MANAGER_CALC_CLASS, NULL);
   EXPECT_ERROR_END;
   fail_if(m);

   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(redirect_param)
{
   Efl_Ui_Focus_Manager *m, *m2;
   Efl_Ui_Focus_Object *root,*root2;

   TEST_OBJ_NEW(child, 0, 20, 20, 20);
   TEST_OBJ_NEW(child2, 0, 20, 20, 20);

   m = focus_test_manager_new(&root, EINA_TRUE);
   m2 = focus_test_manager_new(&root2, EINA_FALSE);
   efl_ui_focus_manager_calc_register_logical(m, root2, root, m2);
   focus_test_manager_set(root2, m);

   efl_ui_focus_manager_calc_register(m, child, root, m2);
   efl_ui_focus_manager_calc_register(m2, child2, root2, NULL);
   efl_ui_focus_manager_focus_set(m, root2);

   ck_assert_ptr_eq(efl_ui_focus_manager_redirect_get(m), m2);

   efl_unref(m);
   efl_unref(m2);
}
EFL_END_TEST

EFL_START_TEST(invalid_args_check)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root;

   TEST_OBJ_NEW(child, 0, 20, 20, 20);
   TEST_OBJ_NEW(child2, 0, 20, 20, 20);

   m = focus_test_manager_new(&root, EINA_TRUE);

   //no child and no parent
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_ui_focus_manager_calc_register(m, NULL, NULL, NULL), 0);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_ui_focus_manager_calc_register(m, child, NULL, NULL), 0);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_ui_focus_manager_calc_register(m, NULL, root, NULL), 0);
   EXPECT_ERROR_END;
   ck_assert_int_eq(efl_ui_focus_manager_calc_register(m, child, root, NULL), 1);
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_ui_focus_manager_calc_update_parent(m, child, NULL), 0);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_ui_focus_manager_calc_update_parent(m, NULL, NULL), 0);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_ui_focus_manager_calc_update_parent(m, child, child2), 0);
   EXPECT_ERROR_END;
   ck_assert_int_eq(efl_ui_focus_manager_calc_register(m, child2, root, NULL), 1);
   ck_assert_int_eq(efl_ui_focus_manager_calc_update_parent(m, child, child2), 1);
   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(order_check)
{
   Efl_Ui_Focus_Manager *m;
   Eina_List *order = NULL;
   Efl_Ui_Focus_Object *root;

   TEST_OBJ_NEW(child1, 0, 20, 20, 20);
   TEST_OBJ_NEW(child2, 0, 20, 20, 20);
   TEST_OBJ_NEW(child3, 0, 20, 20, 20);

   m = focus_test_manager_new(&root, EINA_TRUE);

   //no child and no parent
   efl_ui_focus_manager_calc_register(m, child1, root, NULL);
   efl_ui_focus_manager_calc_register(m, child2, root, NULL);
   efl_ui_focus_manager_calc_register(m, child3, root, NULL);

   //positiv check
   order = eina_list_append(order, child2);
   order = eina_list_append(order, child3);
   order = eina_list_append(order, child1);
   ck_assert_int_eq(efl_ui_focus_manager_calc_update_children(m, root, order), 1);

   order = NULL;

   //negativ check
   order = eina_list_append(order, child1);
   order = eina_list_append(order, child2);
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_ui_focus_manager_calc_update_children(m, root, order), 0);
   EXPECT_ERROR_END;
   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(logical_shift)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root;

   TEST_OBJ_NEW(child, 0, 0, 10, 10);
   TEST_OBJ_NEW(sub, 0, 0, 10, 10);
   TEST_OBJ_NEW(sub_sub, 0, 0, 10, 10);
   TEST_OBJ_NEW(sub_child, 0, 0, 10, 10);

   m = focus_test_manager_new(&root, EINA_TRUE);

   efl_ui_focus_manager_calc_register(m, child, root, NULL);
   efl_ui_focus_manager_calc_register_logical(m, sub, root, NULL);
   efl_ui_focus_manager_calc_register_logical(m, sub_sub, sub, NULL);
   efl_ui_focus_manager_calc_register(m, sub_child, sub, NULL);

   efl_ui_focus_manager_focus_set(m, root);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), child);

   efl_ui_focus_manager_focus_set(m, sub);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), sub_child);

   EXPECT_ERROR_START;
   efl_ui_focus_manager_focus_set(m, sub_sub);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), sub_child);
   EXPECT_ERROR_END;
   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(root_redirect_chain)
{
   Efl_Ui_Focus_Manager *m, *m2;
   Efl_Ui_Focus_Object *root, *root2;

   TEST_OBJ_NEW(child, 0, 20, 20, 20);

   m = focus_test_manager_new(&root, EINA_TRUE);
   m2 = focus_test_manager_new(&root2, EINA_TRUE);
   focus_test_manager_set(root2, m);

   efl_ui_focus_manager_calc_register(m, root2, root, m2);
   efl_ui_focus_manager_calc_register(m2, child, root2, NULL);
   efl_ui_focus_manager_focus_set(m2, child);

   ck_assert_ptr_eq(efl_ui_focus_manager_redirect_get(m), m2);

   efl_unref(m);
   efl_unref(m2);
}
EFL_END_TEST

EFL_START_TEST(root_redirect_chain_unset)
{
   Efl_Ui_Focus_Manager *m, *m2, *m3, *m4;
   Efl_Ui_Focus_Object *root, *root2, *root3, *root4;

   TEST_OBJ_NEW(child, 0, 20, 20, 20);
   TEST_OBJ_NEW(child2, 0, 20, 20, 20);
   TEST_OBJ_NEW(child3, 0, 20, 20, 20);

   m = focus_test_manager_new(&root, EINA_TRUE);
   m2 = focus_test_manager_new(&root2, EINA_FALSE);
   m3 = focus_test_manager_new(&root3, EINA_FALSE);
   m4 = focus_test_manager_new(&root4, EINA_FALSE);
   focus_test_manager_set(root2, m);
   focus_test_manager_set(root3, m);
   focus_test_manager_set(root4, m3);

   efl_ui_focus_manager_calc_register(m, root2, root, m2);
   efl_ui_focus_manager_calc_register(m, root3, root, m3);
   efl_ui_focus_manager_calc_register(m2, child, root2, NULL);
   efl_ui_focus_manager_calc_register(m3, child2, root3, NULL);
   efl_ui_focus_manager_calc_register(m3, root4, root3, NULL);
   efl_ui_focus_manager_calc_register(m4, child3, root4, NULL);

   efl_ui_focus_manager_redirect_set(m, m3);
   efl_ui_focus_manager_redirect_set(m3, m4);

   efl_ui_focus_manager_focus_set(m2, child);

   ck_assert_ptr_eq(efl_ui_focus_manager_redirect_get(m), m2);
   ck_assert_ptr_eq(efl_ui_focus_manager_redirect_get(m2), NULL);
   ck_assert_ptr_eq(efl_ui_focus_manager_redirect_get(m3), NULL);
   ck_assert_ptr_eq(efl_ui_focus_manager_redirect_get(m4), NULL);

   efl_unref(m);
   efl_unref(m2);
   efl_unref(m3);
   efl_unref(m4);
}
EFL_END_TEST

static Efl_Ui_Focus_Manager_Calc*
_recursive_triangle_manager(int recusion_depth, Efl_Ui_Focus_Object **most_right, Efl_Ui_Focus_Object **most_left, Eina_List **managers)
{
   Efl_Ui_Focus_Manager *m, *m_child1 = NULL, *m_child3 = NULL;
   Efl_Ui_Focus_Object *child1, *child3, *root;

   TEST_OBJ_NEW(child2, 0, 20, 20, 20);

   m = focus_test_manager_new(&root, recusion_depth == 0);

   if (recusion_depth < 3)
     {
        m_child1 = _recursive_triangle_manager(recusion_depth + 1, NULL, most_left, managers);
        m_child3 = _recursive_triangle_manager(recusion_depth + 1, most_right, NULL, managers);
        child1 = efl_ui_focus_manager_root_get(m_child1);
        child3 = efl_ui_focus_manager_root_get(m_child3);
        focus_test_manager_set(child1, m);
        focus_test_manager_set(child3, m);
     }
   else
     {
        TEST_OBJ_NEW(tmp_child1, 0, 20, 20, 20);
        TEST_OBJ_NEW(tmp_child3, 0, 20, 20, 20);

        child1 = tmp_child1;
        child3 = tmp_child3;

        if (most_left)
          {
             *most_left = child1;
          }

        if (most_right)
          {
             *most_right = child3;
          }
     }

   efl_ui_focus_manager_calc_register(m, child1, root, m_child1);
   efl_ui_focus_manager_calc_register(m, child2, root, NULL);
   efl_ui_focus_manager_calc_register(m, child3, root, m_child3);

   *managers = eina_list_append(*managers , m);

   return m;
}

static Efl_Ui_Focus_Object*
_get_highest_redirect(Efl_Ui_Focus_Manager *manager)
{
   Efl_Ui_Focus_Manager *tmp = manager;

   do {
     manager = tmp;
     tmp = efl_ui_focus_manager_redirect_get(tmp);
   } while (tmp);

   return manager;
}

EFL_START_TEST(first_touch_check)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *most_left, *most_right;
   Eina_List *managers = NULL;

   m = _recursive_triangle_manager(0, &most_right, &most_left, &managers);

   efl_ui_focus_manager_setup_on_first_touch(m, EFL_UI_FOCUS_DIRECTION_NEXT, efl_ui_focus_manager_root_get(m));

   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(_get_highest_redirect(m)), most_left);

   efl_ui_focus_manager_reset_history(m);

   efl_ui_focus_manager_setup_on_first_touch(m, EFL_UI_FOCUS_DIRECTION_PREVIOUS, efl_ui_focus_manager_root_get(m));
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(_get_highest_redirect(m)), most_right);

   EINA_LIST_FREE(managers, m)
     efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(test_request_subchild_empty)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root;

   m = focus_test_manager_new(&root, EINA_TRUE);

   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, root), NULL);

   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(test_request_subchild_one_element)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root, *c1;

   m = focus_test_manager_new(&root, EINA_TRUE);
   c1 = focus_test_object_new("child1", 0, 0, 20, 20);
   efl_ui_focus_manager_calc_register(m, c1, root, NULL);

   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, root), c1);
   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, c1), NULL);

   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(test_request_subchild_child_alongside)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root, *c1, *c2;

   m = focus_test_manager_new(&root, EINA_TRUE);
   c1 = focus_test_object_new("child1", 0, 0, 20, 20);
   c2 = focus_test_object_new("child2", 0, 0, 20, 20);
   efl_ui_focus_manager_calc_register(m, c1, root, NULL);
   efl_ui_focus_manager_calc_register(m, c2, root, NULL);

   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, root), c1);
   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, c1), NULL);
   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, c2), NULL);

   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(test_request_subchild_child_logical_regular)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root, *c1, *c2;

   m = focus_test_manager_new(&root, EINA_TRUE);
   c1 = focus_test_object_new("child1", 0, 0, 20, 20);
   c2 = focus_test_object_new("child2", 0, 0, 20, 20);
   efl_ui_focus_manager_calc_register_logical(m, c1, root, NULL);
   efl_ui_focus_manager_calc_register(m, c2, c1, NULL);

   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, root), c2);
   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, c1), c2);
   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, c2), NULL);

   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(test_request_subchild_child_regular_regular)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root, *c1, *c2;

   m = focus_test_manager_new(&root, EINA_TRUE);
   c1 = focus_test_object_new("child1", 0, 0, 20, 20);
   c2 = focus_test_object_new("child2", 0, 0, 20, 20);
   efl_ui_focus_manager_calc_register(m, c1, root, NULL);
   efl_ui_focus_manager_calc_register(m, c2, c1, NULL);

   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, root), c1);
   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, c1), c2);
   ck_assert_ptr_eq(efl_ui_focus_manager_request_subchild(m, c2), NULL);

   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(test_unregister_last_focused_no_history)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root, *c1, *c2, *c3;

   m = focus_test_manager_new(&root, EINA_TRUE);
   c1 = focus_test_object_new("child1", 0, 0, 20, 20);
   c2 = focus_test_object_new("child2", 0, 0, 20, 20);
   c3 = focus_test_object_new("child3", 0, 0, 20, 20);
   efl_ui_focus_manager_calc_register(m, c1, root, NULL);
   efl_ui_focus_manager_calc_register(m, c2, root, NULL);
   efl_ui_focus_manager_calc_register(m, c3, root, NULL);

   efl_ui_focus_manager_focus_set(m, c2);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), c2);

   efl_ui_focus_manager_calc_unregister(m, c2);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), c1);

   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(test_unregister_last_focused)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root, *c1, *c2, *c3;

   m = focus_test_manager_new(&root, EINA_TRUE);
   c1 = focus_test_object_new("child1", 0, 0, 20, 20);
   c2 = focus_test_object_new("child2", 0, 0, 20, 20);
   c3 = focus_test_object_new("child3", 0, 0, 20, 20);
   efl_ui_focus_manager_calc_register(m, c1, root, NULL);
   efl_ui_focus_manager_calc_register(m, c2, root, NULL);
   efl_ui_focus_manager_calc_register(m, c3, root, NULL);

   efl_ui_focus_manager_focus_set(m, c2);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), c2);
   efl_ui_focus_manager_focus_set(m, c3);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), c3);

   efl_ui_focus_manager_calc_unregister(m, c3);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), c2);

   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(test_unregister_last_focused_no_child)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root, *c1;

   m = focus_test_manager_new(&root, EINA_TRUE);
   c1 = focus_test_object_new("child1", 0, 0, 20, 20);
   efl_ui_focus_manager_calc_register(m, c1, root, NULL);

   efl_ui_focus_manager_focus_set(m, c1);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), c1);
   efl_ui_focus_manager_calc_unregister(m, c1);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), NULL);

   efl_unref(m);
}
EFL_END_TEST

EFL_START_TEST(test_pop_history_element)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root, *c1, *c2, *c3;

   m = focus_test_manager_new(&root, EINA_TRUE);
   c1 = focus_test_object_new("child1", 0, 0, 20, 20);
   c2 = focus_test_object_new("child2", 0, 0, 20, 20);
   c3 = focus_test_object_new("child3", 0, 0, 20, 20);
   efl_ui_focus_manager_calc_register(m, c1, root, NULL);
   efl_ui_focus_manager_calc_register(m, c2, root, NULL);
   efl_ui_focus_manager_calc_register(m, c3, root, NULL);

   efl_ui_focus_manager_focus_set(m, c2);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), c2);
   ck_assert_int_eq(efl_ui_focus_object_focus_get(c2), EINA_TRUE);

   efl_ui_focus_manager_pop_history_stack(m);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), c1);
   ck_assert_int_eq(efl_ui_focus_object_focus_get(c2), EINA_FALSE);

   efl_ui_focus_manager_focus_set(m, c1);
   efl_ui_focus_manager_focus_set(m, c2);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), c2);
   ck_assert_int_eq(efl_ui_focus_object_focus_get(c2), EINA_TRUE);
   efl_ui_focus_manager_pop_history_stack(m);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), c1);
   ck_assert_int_eq(efl_ui_focus_object_focus_get(c2), EINA_FALSE);
   ck_assert_int_eq(efl_ui_focus_object_focus_get(c1), EINA_TRUE);

   efl_unref(m);
}
EFL_END_TEST

static void
_focus_request_tree_check(Efl_Ui_Focus_Manager *m, Eina_Bool accept_logicals, Efl_Ui_Focus_Object *objs[])
{
   int i = 0;
   for (i = 1; objs[i]; ++i)
   {
      ck_assert_ptr_eq(efl_ui_focus_manager_request_move(m, EFL_UI_FOCUS_DIRECTION_NEXT, objs[i], accept_logicals), objs[i + 1]);
   }
   i--;
   for (; i > 0; --i)
   {
      ck_assert_ptr_eq(efl_ui_focus_manager_request_move(m, EFL_UI_FOCUS_DIRECTION_PREVIOUS, objs[i], accept_logicals), objs[i - 1]);
   }
}

EFL_START_TEST(test_request_move)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root, *c1, *c2, *c3, *c4, *c5, *c6;

   m = focus_test_manager_new(&root, EINA_TRUE);
   c1 = focus_test_object_new("child1", 0, 0, 20, 20);
   c2 = focus_test_object_new("child2", 0, 0, 20, 20);
   c3 = focus_test_object_new("child3", 0, 0, 20, 20);
   c4 = focus_test_object_new("child4", 0, 0, 20, 20);
   c5 = focus_test_object_new("child5", 0, 0, 20, 20);
   c6 = focus_test_object_new("child6", 0, 0, 20, 20);

   efl_ui_focus_manager_calc_register(m, c1, root, NULL);
   efl_ui_focus_manager_calc_register_logical(m, c2, root, NULL);
   efl_ui_focus_manager_calc_register(m, c3, c2, NULL);
   efl_ui_focus_manager_calc_register(m, c4, c2, NULL);
   efl_ui_focus_manager_calc_register(m, c5, c2, NULL);
   efl_ui_focus_manager_calc_register(m, c6, root, NULL);

   //   c3  c4  c5
   //    \  |  /
   //     \ | /
   //   c1  c2  c6
   //    \  |  /
   //     rooot

   Efl_Ui_Focus_Object *objs_logical[] = {NULL, root, c1, c2, c3, c4, c5, c6, NULL};
   _focus_request_tree_check(m, EINA_TRUE, objs_logical);

   Efl_Ui_Focus_Object *objs_regular[] = {NULL, c1, c3, c4, c5, c6, NULL};
   _focus_request_tree_check(m, EINA_FALSE, objs_regular);

}
EFL_END_TEST

EFL_START_TEST(redirect_unregister_entrypoint)
{
   Efl_Ui_Focus_Manager *m, *m2;
   Efl_Ui_Focus_Object *root, *root2;

   TEST_OBJ_NEW(child, 0, 20, 20, 20);
   TEST_OBJ_NEW(child2, 0, 20, 20, 20);
   TEST_OBJ_NEW(child3, 0, 20, 20, 20);

   m = focus_test_manager_new(&root, EINA_TRUE);
   m2 = focus_test_manager_new(&root2, EINA_TRUE);

   efl_ui_focus_manager_calc_register(m2, child3, root2, m2);

   efl_ui_focus_manager_calc_register(m, child, root, m2);
   efl_ui_focus_manager_calc_register(m, child2, root, NULL);
   efl_ui_focus_manager_focus_set(m, child);

   ck_assert_ptr_eq(efl_ui_focus_manager_redirect_get(m), m2);
   efl_ui_focus_manager_calc_unregister(m, child);
   ck_assert_ptr_eq(efl_ui_focus_manager_redirect_get(m), m2);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(m), NULL);

   efl_unref(m);
   efl_unref(m2);
}
EFL_END_TEST

static void
_child_focus_changed(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eina_Bool *flag = data;

   *flag = EINA_TRUE;
}

EFL_START_TEST(test_events_child_focus)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *root, *c1, *c2, *c3, *c4, *c5, *c6;
   Eina_Bool froot = EINA_FALSE, fc1 = EINA_FALSE, fc2 = EINA_FALSE,
             fc3 = EINA_FALSE, fc4 = EINA_FALSE, fc5 = EINA_FALSE, fc6 = EINA_FALSE;

   m = focus_test_manager_new(&root, EINA_TRUE);
   c1 = focus_test_object_new("child1", 0, 0, 20, 20);
   efl_parent_set(c1, root);
   c2 = focus_test_object_new("child2", 0, 0, 20, 20);
   efl_parent_set(c2, root);
   c3 = focus_test_object_new("child3", 0, 0, 20, 20);
   efl_parent_set(c3, c2);
   c4 = focus_test_object_new("child4", 0, 0, 20, 20);
   efl_parent_set(c4, c2);
   c5 = focus_test_object_new("child5", 0, 0, 20, 20);
   efl_parent_set(c5, root);
   c6 = focus_test_object_new("child6", 0, 0, 20, 20);
   efl_parent_set(c6, c5);

   efl_ui_focus_manager_calc_register(m, c1, root, NULL);
   efl_ui_focus_manager_calc_register_logical(m, c2, root, NULL);
   efl_ui_focus_manager_calc_register(m, c3, c2, NULL);
   efl_ui_focus_manager_calc_register(m, c4, c2, NULL);
   efl_ui_focus_manager_calc_register_logical(m, c5, root, NULL);
   efl_ui_focus_manager_calc_register(m, c6, c5, NULL);

   //   c3  c4  c6
   //    \  |   |
   //     \ |   |
   //   c1  c2  c5
   //    \  |  /
   //     root

   efl_ui_focus_manager_focus_set(m, c4);

   ck_assert_int_eq(froot, EINA_FALSE);
   ck_assert_int_eq(fc1, EINA_FALSE);
   ck_assert_int_eq(fc2, EINA_FALSE);
   ck_assert_int_eq(fc3, EINA_FALSE);
   ck_assert_int_eq(fc4, EINA_FALSE);
   ck_assert_int_eq(fc5, EINA_FALSE);
   ck_assert_int_eq(fc6, EINA_FALSE);

   efl_event_callback_add(root, EFL_UI_FOCUS_OBJECT_EVENT_CHILD_FOCUS_CHANGED, _child_focus_changed, &froot);
   efl_event_callback_add(c1, EFL_UI_FOCUS_OBJECT_EVENT_CHILD_FOCUS_CHANGED, _child_focus_changed, &fc1);
   efl_event_callback_add(c2, EFL_UI_FOCUS_OBJECT_EVENT_CHILD_FOCUS_CHANGED, _child_focus_changed, &fc2);
   efl_event_callback_add(c3, EFL_UI_FOCUS_OBJECT_EVENT_CHILD_FOCUS_CHANGED, _child_focus_changed, &fc3);
   efl_event_callback_add(c4, EFL_UI_FOCUS_OBJECT_EVENT_CHILD_FOCUS_CHANGED, _child_focus_changed, &fc4);
   efl_event_callback_add(c5, EFL_UI_FOCUS_OBJECT_EVENT_CHILD_FOCUS_CHANGED, _child_focus_changed, &fc5);
   efl_event_callback_add(c6, EFL_UI_FOCUS_OBJECT_EVENT_CHILD_FOCUS_CHANGED, _child_focus_changed, &fc6);
   efl_ui_focus_manager_focus_set(m, c1);

   ck_assert_int_eq(froot, EINA_FALSE);
   ck_assert_int_eq(fc1, EINA_FALSE);
   ck_assert_int_eq(fc2, EINA_TRUE);
   ck_assert_int_eq(fc3, EINA_FALSE);
   ck_assert_int_eq(fc4, EINA_FALSE);
   ck_assert_int_eq(fc5, EINA_FALSE);
   ck_assert_int_eq(fc6, EINA_FALSE);
   froot = EINA_FALSE;
   fc1 = EINA_FALSE;
   fc2 = EINA_FALSE;
   fc3 = EINA_FALSE;
   fc4 = EINA_FALSE;
   fc5 = EINA_FALSE;
   fc6 = EINA_FALSE;

   efl_ui_focus_manager_focus_set(m, c6);

   ck_assert_int_eq(froot, EINA_FALSE);
   ck_assert_int_eq(fc1, EINA_FALSE);
   ck_assert_int_eq(fc2, EINA_FALSE);
   ck_assert_int_eq(fc3, EINA_FALSE);
   ck_assert_int_eq(fc4, EINA_FALSE);
   ck_assert_int_eq(fc5, EINA_TRUE);
   ck_assert_int_eq(fc6, EINA_FALSE);
   froot = EINA_FALSE;
   fc1 = EINA_FALSE;
   fc2 = EINA_FALSE;
   fc3 = EINA_FALSE;
   fc4 = EINA_FALSE;
   fc5 = EINA_FALSE;
   fc6 = EINA_FALSE;

   efl_ui_focus_manager_focus_set(m, c3);

   ck_assert_int_eq(froot, EINA_FALSE);
   ck_assert_int_eq(fc1, EINA_FALSE);
   ck_assert_int_eq(fc2, EINA_TRUE);
   ck_assert_int_eq(fc3, EINA_FALSE);
   ck_assert_int_eq(fc4, EINA_FALSE);
   ck_assert_int_eq(fc5, EINA_TRUE);
   ck_assert_int_eq(fc6, EINA_FALSE);
}
EFL_END_TEST


EFL_START_TEST(viewport_check)
{
   Efl_Ui_Focus_Manager *m;
   Efl_Ui_Focus_Object *middle, *east, *west, *north, *south, *root;
   Eina_List *list = NULL;
   Eina_Iterator *iter;
   Efl_Ui_Focus_Object *obj;

   focus_test_setup_cross(&middle, &south, &north, &east, &west);

   m = focus_test_manager_new(&root, EINA_TRUE);
   efl_ui_focus_manager_calc_register(m, middle, root, NULL);
   efl_ui_focus_manager_calc_register(m, south, root, NULL);
   efl_ui_focus_manager_calc_register(m, north, root, NULL);
   efl_ui_focus_manager_calc_register(m, east, root, NULL);
   efl_ui_focus_manager_calc_register(m, west, root, NULL);

   iter = efl_ui_focus_manager_viewport_elements_get(m, EINA_RECT(80, 0, 100, 100));

   EINA_ITERATOR_FOREACH(iter, obj)
     {
        list = eina_list_append(list, obj);
     }

   eina_iterator_free(iter);

   ck_assert(eina_list_count(list) == 1);
   ck_assert_ptr_eq(eina_list_data_get(list), east);
}
EFL_END_TEST

void efl_ui_test_focus(TCase *tc)
{
    tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
    tcase_add_test(tc, focus_register_twice);
    tcase_add_test(tc, focus_unregister_twice);
    tcase_add_test(tc, pos_check);
    tcase_add_test(tc, pos_check2);
    tcase_add_test(tc, redirect);
    tcase_add_test(tc, border_check);
    tcase_add_test(tc, finalize_check);
    tcase_add_test(tc, logical_chain);
    tcase_add_test(tc, logical_chain_multi_redirect);
    tcase_add_test(tc, logical_chain_single_redirect);
    tcase_add_test(tc, redirect_param);
    tcase_add_test(tc, invalid_args_check);
    tcase_add_test(tc, order_check);
    tcase_add_test(tc, logical_shift);
    tcase_add_test(tc, root_redirect_chain);
    tcase_add_test(tc, root_redirect_chain_unset);
    tcase_add_test(tc, first_touch_check);
    tcase_add_test(tc, test_request_subchild_empty);
    tcase_add_test(tc, test_request_subchild_one_element);
    tcase_add_test(tc, test_request_subchild_child_alongside);
    tcase_add_test(tc, test_request_subchild_child_logical_regular);
    tcase_add_test(tc, test_request_subchild_child_regular_regular);
    tcase_add_test(tc, test_unregister_last_focused_no_history);
    tcase_add_test(tc, test_unregister_last_focused);
    tcase_add_test(tc, test_unregister_last_focused_no_child);
    tcase_add_test(tc, test_pop_history_element);
    tcase_add_test(tc, test_request_move);
    tcase_add_test(tc, redirect_unregister_entrypoint);
    tcase_add_test(tc, test_events_child_focus);
    tcase_add_test(tc, viewport_check);
}
