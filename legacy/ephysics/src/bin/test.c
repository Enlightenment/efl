#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

#define EPHYSICS_TEST_LOG_COLOR EINA_COLOR_GREEN

int _ephysics_test_log_dom = -1;

/* examples prototypes */
void test_bouncing_ball(void *data, Evas_Object *obj, void *event_info);
void test_bouncing_text(void *data, Evas_Object *obj, void *event_info);
void test_colliding_balls(void *data, Evas_Object *obj, void *event_info);
void test_collision(void *data, Evas_Object *obj, void *event_info);
void test_constraint(void *data, Evas_Object *obj, void *event_info);
void test_falling_letters(void *data, Evas_Object *obj, void *event_info);
void test_jumping_balls(void *data, Evas_Object *obj, void *event_info);
void test_rotate(void *data, Evas_Object *obj, void *event_info);

static void
_win_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_exit();
}

static void
_subwin_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   test_data_del(data);
   ephysics_shutdown();
}

void
update_object_cb(void *data, EPhysics_Body *body, void *event_info)
{
   Evas_Object *sphere = event_info;
   Evas_Object *shadow = data;
   Edje_Message_Float msg;
   Evas_Object *edje;
   int x, y, w, h;

   ephysics_body_evas_object_update(body);
   evas_object_geometry_get(sphere, &x, &y, &w, &h);

   if (y > SH_THRESHOLD)
     {
        int sh_w;

        msg.val = (double) (y - SH_THRESHOLD) / (FLOOR_Y - SH_THRESHOLD - h);
        sh_w = (1 + 2 * msg.val) * w / 3;
        evas_object_resize(shadow, sh_w, 3);
        evas_object_move(shadow, x + (w - sh_w) / 2, FLOOR_Y);
     }
   else
        msg.val = 0;

   edje = elm_layout_edje_get(shadow);
   edje_object_message_send(edje, EDJE_MESSAGE_FLOAT, SHADOW_ALPHA_ID, &msg);
}


void
test_clean(Test_Data *test_data)
{
   EPhysics_Constraint *constraint;
   Evas_Object *evas_obj;
   EPhysics_Body *body;

   EINA_LIST_FREE(test_data->constraints, constraint)
      ephysics_constraint_del(constraint);

   EINA_LIST_FREE(test_data->bodies, body)
      ephysics_body_del(body);

   EINA_LIST_FREE(test_data->evas_objs, evas_obj)
      evas_object_del(evas_obj);
}

void
test_data_del(Test_Data *test_data)
{
   test_clean(test_data);
   evas_object_del(test_data->layout);
   ephysics_world_del(test_data->world);
   free(test_data);
}

Test_Data *
test_data_new(void)
{
   Test_Data *test_data;

   test_data = calloc(1, sizeof(Test_Data));
   if (!test_data)
     ERR("Failed to create test data");

   return test_data;
}

void
test_win_add(Test_Data *test_data, const char *title, Eina_Bool autodel)
{
   Evas_Object *win, *ly;

   win = elm_win_add(NULL, title, ELM_WIN_BASIC);
   elm_win_title_set(win, title);
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_screen_constrain_set(win, EINA_TRUE);
   evas_object_show(win);
   test_data->win = win;
   if (autodel)
     evas_object_smart_callback_add(win, "delete,request", _subwin_del,
                                    test_data);

   ly = elm_layout_add(win);
   elm_win_resize_object_add(win, ly);
   evas_object_size_hint_min_set(ly, WIDTH, HEIGHT);
   evas_object_size_hint_max_set(ly, WIDTH, HEIGHT);
   elm_layout_file_set(ly, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
                       "frame");
   evas_object_show(ly);
   test_data->layout = ly;
}

#define ADD_TEST(_name, _func) \
   elm_list_item_append(list, _name, NULL, NULL, _func, NULL);

/* FIXME add autorun and test_win_only features */
static void
_main_win_add(char *autorun __UNUSED__, Eina_Bool test_win_only __UNUSED__)
{
   Evas_Object *win, *list;

   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   elm_win_title_set(win, "EPhysics Tests");
   evas_object_smart_callback_add(win, "delete,request", _win_del, NULL);
   evas_object_resize(win, 460, 560);
   evas_object_show(win);

   list = elm_list_add(win);
   elm_object_style_set(list, "ephysics-test");
   elm_list_select_mode_set(list, ELM_OBJECT_SELECT_MODE_ALWAYS);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, list);
   evas_object_show(list);

   ADD_TEST("BOUNCING BALL", test_bouncing_ball);
   ADD_TEST("BOUNCING TEXT", test_bouncing_text);
   ADD_TEST("COLLIDING BALLS", test_colliding_balls);
   ADD_TEST("COLLISION DETECTION", test_collision);
   ADD_TEST("CONSTRAINT", test_constraint);
   ADD_TEST("FALLING LETTERS", test_falling_letters);
   ADD_TEST("JUMPING BALLS", test_jumping_balls);
   ADD_TEST("ROTATE", test_rotate);

   elm_list_go(list);
}

#undef ADD_TEST

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Eina_Bool test_win_only = EINA_FALSE;
   char *autorun = NULL;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);

   /* if called with a single argument try to autorun a test with
    * the same name as the given param
    * ex:  elementary_test "Box Vert 2" */
   if (argc == 2)
     autorun = argv[1];
   else if (argc == 3)
     {
        if ((!strcmp(argv[1], "--test-win-only")) ||
            (!strcmp(argv[1], "-to")))
          {
             test_win_only = EINA_TRUE;
             autorun = argv[2];
          }
     }

   _ephysics_test_log_dom = eina_log_domain_register(
      "ephysics-test", EPHYSICS_TEST_LOG_COLOR);

   elm_theme_extension_add(NULL,
                           PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj");
   _main_win_add(autorun, test_win_only);
   elm_run();
   elm_shutdown();
   return 0;
}
ELM_MAIN()
