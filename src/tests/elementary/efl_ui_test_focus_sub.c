#include "efl_ui_test_focus_common.h"
#include "focus_test_sub_main.eo.h"

typedef struct {

} Focus_Test_Sub_Main_Data;

EOLIAN static Eina_Rect
_focus_test_sub_main_efl_gfx_entity_geometry_get(const Eo *obj EINA_UNUSED, Focus_Test_Sub_Main_Data *pd EINA_UNUSED)
{
   return EINA_RECT(-10, -10, 40, 40);
}

EOLIAN static Eina_Rect
_focus_test_sub_main_efl_ui_focus_object_focus_geometry_get(const Eo *obj EINA_UNUSED, Focus_Test_Sub_Main_Data *pd EINA_UNUSED)
{
   return EINA_RECT(0, 0, 20, 20);
}

EOLIAN static Efl_Ui_Focus_Manager*
_focus_test_sub_main_efl_ui_focus_object_focus_manager_get(const Eo *obj, Focus_Test_Sub_Main_Data *pd EINA_UNUSED)
{
   return efl_key_data_get(obj, "__user_manager");
}

EOLIAN static Efl_Ui_Focus_Object*
_focus_test_sub_main_efl_ui_focus_object_focus_parent_get(const Eo *obj, Focus_Test_Sub_Main_Data *pd EINA_UNUSED)
{
   return efl_key_data_get(obj, "__user_parent");
}


static Eina_List *registered;
static Eina_List *unregistered;

static Eina_Bool
_register(Eo *eo, void* data EINA_UNUSED, Efl_Ui_Focus_Object *child, Efl_Ui_Focus_Object *parent, Efl_Ui_Focus_Manager *manager)
{
   registered = eina_list_append(registered, child);
   printf("REGISTERED %p %s\n", child, efl_name_get(child));

   return efl_ui_focus_manager_calc_register(efl_super(eo, EFL_OBJECT_OVERRIDE_CLASS) , child, parent, manager);
}

static void
_unregister(Eo *eo, void* data EINA_UNUSED, Efl_Ui_Focus_Object *child)
{
   unregistered = eina_list_append(unregistered, child);
   printf("UNREGISTERED %p %s\n", child, efl_name_get(child));

   efl_ui_focus_manager_calc_unregister(efl_super(eo, EFL_OBJECT_OVERRIDE_CLASS) , child);
}

static Eina_Bool
_set_equal(Eina_List *a, Eina_List *b)
{
   Eina_List *n;
   void *d;

   ck_assert_int_eq(eina_list_count(a), eina_list_count(b));

   EINA_LIST_FOREACH(a, n, d)
     {
        ck_assert_ptr_ne(eina_list_data_find(b, d), NULL);
     }
   return EINA_TRUE;
}

#include "focus_test_sub_main.eo.c"

static void
_setup(Efl_Ui_Focus_Manager **m, Efl_Ui_Focus_Manager_Sub **sub, Efl_Ui_Focus_Object **r)
{

   TEST_OBJ_NEW(root, 10, 10, 10, 10);
   TEST_OBJ_NEW(root_manager, 0, 20, 20, 20);

   EFL_OPS_DEFINE(manager_tracker,
    EFL_OBJECT_OP_FUNC(efl_ui_focus_manager_calc_register, _register),
    EFL_OBJECT_OP_FUNC(efl_ui_focus_manager_calc_unregister, _unregister),
    );

   Efl_Ui_Focus_Manager *manager = efl_add_ref(EFL_UI_FOCUS_MANAGER_CALC_CLASS, NULL,
    efl_ui_focus_manager_root_set(efl_added, root_manager)
   );
   //flush now all changes
   efl_event_callback_call(manager, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, NULL);
   registered = NULL;
   unregistered = NULL;

   efl_object_override(manager, &manager_tracker);

   Focus_Test_Sub_Main *focus_main = efl_add_ref(FOCUS_TEST_SUB_MAIN_CLASS, NULL);

   efl_key_data_set(focus_main, "__user_manager", manager);
   efl_key_data_set(focus_main, "__user_parent", root_manager);

   Efl_Ui_Focus_Manager_Calc *subm = efl_add(EFL_UI_FOCUS_MANAGER_CALC_CLASS, focus_main,
    efl_ui_focus_manager_root_set(efl_added, root)
   );
   efl_event_callback_forwarder_add(subm, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, focus_main);
   efl_event_callback_forwarder_add(subm, EFL_UI_FOCUS_MANAGER_EVENT_REDIRECT_CHANGED, focus_main);
   efl_event_callback_forwarder_add(subm, EFL_UI_FOCUS_MANAGER_EVENT_MANAGER_FOCUS_CHANGED , focus_main);
   efl_event_callback_forwarder_add(subm, EFL_UI_FOCUS_MANAGER_EVENT_COORDS_DIRTY, focus_main);
   efl_event_callback_forwarder_add(subm, EFL_UI_FOCUS_MANAGER_EVENT_DIRTY_LOGIC_FREEZE_CHANGED, focus_main);


   efl_composite_attach(focus_main, subm);

   efl_event_callback_call(focus_main, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_MANAGER_CHANGED, NULL);

   efl_ui_focus_manager_calc_register_logical(manager, focus_main, root_manager, subm);

   *sub = focus_main;
   *m = manager;
   *r = root;
}

EFL_START_TEST(correct_register)
{
   Eina_List *set1 = NULL;
   Efl_Ui_Focus_Object *root;
   Efl_Ui_Focus_Manager *manager, *sub;
   _setup(&manager, &sub, &root);

   TEST_OBJ_NEW(child1, 0, 0, 10, 10);
   TEST_OBJ_NEW(child2, 10, 0, 10, 10);
   TEST_OBJ_NEW(child3, 0, 10, 10, 10);

   //set1 = eina_list_append(set1, sub);
   //set1 = eina_list_append(set1, root);
   set1 = eina_list_append(set1, child1);
   set1 = eina_list_append(set1, child2);
   set1 = eina_list_append(set1, child3);

   //test register stuff
   efl_ui_focus_manager_calc_register(sub, child1, root, NULL);
   efl_ui_focus_manager_calc_register(sub, child2, root, NULL);
   efl_ui_focus_manager_calc_register(sub, child3, root, NULL);
   //now force submanager to flush things
   efl_event_callback_call(manager, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, NULL);
   ck_assert_ptr_eq(unregistered, NULL);
   fail_if(!_set_equal(registered, set1));

   efl_ui_focus_manager_calc_unregister(sub, child1);
   efl_ui_focus_manager_calc_unregister(sub, child2);
   efl_ui_focus_manager_calc_unregister(sub, child3);
   efl_unref(child1);
   efl_unref(child2);
   efl_unref(child3);
   efl_unref(sub);
   efl_unref(manager);
}
EFL_END_TEST

EFL_START_TEST(correct_unregister)
{
   Eina_List *set = NULL;
   Efl_Ui_Focus_Object *root;
   Efl_Ui_Focus_Manager *manager, *sub;
   _setup(&manager, &sub, &root);

   TEST_OBJ_NEW(child1, 0, 0, 10, 10);
   TEST_OBJ_NEW(child2, 10, 0, 10, 10);
   TEST_OBJ_NEW(child3, 0, 10, 10, 10);

   set = eina_list_append(set, child3);

   //test register stuff
   efl_ui_focus_manager_calc_register(sub, child1, root, NULL);
   efl_ui_focus_manager_calc_register(sub, child2, root, NULL);
   efl_ui_focus_manager_calc_register(sub, child3, root, NULL);
   efl_event_callback_call(manager, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, NULL);
   eina_list_free(unregistered);
   unregistered = NULL;
   eina_list_free(registered);
   registered = NULL;

   //test unregister stuff
   efl_ui_focus_manager_calc_unregister(sub, child3);
   efl_event_callback_call(manager, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, NULL);
   ck_assert_ptr_eq(registered, NULL);
   fail_if(!_set_equal(unregistered, set));
   eina_list_free(unregistered);
   unregistered = NULL;

   efl_unref(sub);
   efl_unref(manager);
   efl_unref(root);
   efl_unref(child1);
   efl_unref(child2);
   efl_unref(child3);
}
EFL_END_TEST

EFL_START_TEST(correct_un_register)
{
   Eina_List *set_add = NULL, *set_del = NULL;
   Efl_Ui_Focus_Object *root;
   Efl_Ui_Focus_Manager *manager, *sub;
   _setup(&manager, &sub, &root);

   TEST_OBJ_NEW(child1, 0, 0, 10, 10);
   TEST_OBJ_NEW(child2, 10, 0, 10, 10);
   TEST_OBJ_NEW(child3, 0, 10, 10, 10);

   set_add = eina_list_append(set_add, child2);
   set_del = eina_list_append(set_del, child3);
   //test register stuff
   efl_ui_focus_manager_calc_register(sub, child1, root, NULL);
   efl_ui_focus_manager_calc_register(sub, child3, root, NULL);
   efl_event_callback_call(manager, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, NULL);
   eina_list_free(unregistered);
   unregistered = NULL;
   eina_list_free(registered);
   registered = NULL;

   //test unregister stuff
   efl_ui_focus_manager_calc_unregister(sub, child3);
   efl_ui_focus_manager_calc_register(sub, child2, root, NULL);
   efl_event_callback_call(manager, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, NULL);
   fail_if(!_set_equal(registered, set_add));
   fail_if(!_set_equal(unregistered, set_del));

   efl_unref(sub);
   efl_unref(manager);
   efl_unref(root);
   efl_unref(child1);
   efl_unref(child2);
   efl_unref(child3);
}
EFL_END_TEST

void efl_ui_test_focus_sub(TCase *tc)
{
   tcase_add_test(tc, correct_register);
   tcase_add_test(tc, correct_unregister);
   tcase_add_test(tc, correct_un_register);
}
