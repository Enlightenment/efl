#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#define EFL_GFX_FILTER_BETA
#define EFL_CANVAS_LAYOUT_BETA

#include "edje_suite.h"

#define EVAS_DATA_DIR TESTS_SRC_DIR "/../../lib/evas"

static void
_message_signal_reply_cb(void *data, Evas_Object *obj EINA_UNUSED,
                         const char *emission, const char *source)
{
   int *id = data;

   fprintf(stderr, "source %s emit %s id %d\n", source, emission, *id);
   fflush(stderr);
   ck_assert_str_eq(source, "edc");
   ck_assert_ptr_ne(emission, NULL);

   if (!strncmp(emission, "int set", 7))
     ck_assert_str_eq(emission, "int set 7 12 42 255");
   else if (!strncmp(emission, "int", 3))
     ck_assert_str_eq(emission, "int 42");
   else if (!strncmp(emission, "float", 5))
     {
        char buf[64];
        sprintf(buf, "float %f", 0.12);
        ck_assert_str_eq(emission, buf);
     }
   else if (!strncmp(emission, "str", 3))
     ck_assert_str_eq(emission, "str hello world");
   else ck_abort_msg("Invalid emission!");

   (*id)++;
}

EFL_START_TEST(edje_test_message_send_legacy)
{
   Evas *evas;
   Evas_Object *obj;
   Edje_Message_Int msgi;
   Edje_Message_Float msgf;
   Edje_Message_String msgs;
   Edje_Message_Int_Set *msgis;
   int id = 0;

   /* Ugly calls to process:
    *
    * 1. Send edje message (async)
    * 2. Process edje message (sync)
    * 3. EDC program emits edje signal (async)
    * 4. Process edje signal (sync)
    * 5. Finally reached signal cb
    */

   evas = _setup_evas();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_messages.edj"), "test_group"));
   edje_object_signal_callback_add(obj, "*", "edc", _message_signal_reply_cb, &id);

   msgs.str = "hello world";
   edje_object_message_send(obj, EDJE_MESSAGE_STRING, 0, &msgs);
   edje_message_signal_process();
   ck_assert_int_eq(id, 1);

   msgi.val = 42;
   edje_object_message_send(obj, EDJE_MESSAGE_INT, 1, &msgi);
   edje_message_signal_process();
   ck_assert_int_eq(id, 2);

   msgf.val = 0.12;
   edje_object_message_send(obj, EDJE_MESSAGE_FLOAT, 2, &msgf);
   edje_message_signal_process();
   ck_assert_int_eq(id, 3);

   msgis = alloca(sizeof(*msgis) + 4 * sizeof(msgis->val));
   msgis->count = 4;
   msgis->val[0] = 7;
   msgis->val[1] = 12;
   msgis->val[2] = 42;
   msgis->val[3] = 255;
   edje_object_message_send(obj, EDJE_MESSAGE_INT_SET, 3, msgis);
   edje_message_signal_process();
   ck_assert_int_eq(id, 4);

   evas_object_del(obj);

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_message_send_eo)
{
   Evas *evas;
   Evas_Object *obj;
   Eina_Value v, *va;
   int id = 0;

   evas = _setup_evas();

   obj = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
                 efl_file_set(efl_added, test_layout_get("test_messages.edj"), "test_group"));

   // FIXME: EO API HERE
   edje_object_signal_callback_add(obj, "*", "edc", _message_signal_reply_cb, &id);

   // NOTE: edje_object_message_signal_process may or may not be in EO (TBD)

   eina_value_setup(&v, EINA_VALUE_TYPE_STRING);
   eina_value_set(&v, "hello world");
   efl_layout_signal_message_send(obj, 0, v);
   eina_value_flush(&v);
   edje_message_signal_process();
   edje_object_calc_force(obj);
   ck_assert_int_eq(id, 1);

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);
   eina_value_set(&v, 42);
   efl_layout_signal_message_send(obj, 1, v);
   eina_value_flush(&v);
   edje_message_signal_process();
   edje_object_calc_force(obj);
   ck_assert_int_eq(id, 2);

   eina_value_setup(&v, EINA_VALUE_TYPE_FLOAT);
   eina_value_set(&v, 0.12);
   efl_layout_signal_message_send(obj, 2, v);
   eina_value_flush(&v);
   edje_message_signal_process();
   edje_object_calc_force(obj);
   ck_assert_int_eq(id, 3);

   va = eina_value_array_new(EINA_VALUE_TYPE_INT, 4);
   eina_value_array_append(va, 7);
   eina_value_array_append(va, 12);
   eina_value_array_append(va, 42);
   eina_value_array_append(va, 255);
   efl_layout_signal_message_send(obj, 3, *va);
   eina_value_free(va);
   edje_message_signal_process();
   edje_object_calc_force(obj);
   ck_assert_int_eq(id, 4);

   efl_del(obj);

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_signals)
{
   Evas *evas;
   Evas_Object *obj;
   const char *state;

   evas = _setup_evas();

   obj = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
                 efl_file_set(efl_added, test_layout_get("test_signals.edj"), "level1"),
                 efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(320, 240)),
                 efl_gfx_entity_visible_set(efl_added, 1));

   edje_object_signal_emit(obj, "mouse,in", "text");

   edje_object_message_signal_process(obj);
   state = edje_object_part_state_get(obj, "group:group:text", NULL);
   ck_assert_str_eq(state, "default");

   edje_object_message_signal_process(obj);
   state = edje_object_part_state_get(obj, "group:group:text", NULL);
   ck_assert_str_eq(state, "default");

   edje_object_message_signal_recursive_process(obj);
   state = edje_object_part_state_get(obj, "group:group:text", NULL);
   ck_assert_str_eq(state, "over");

   efl_del(obj);

   evas_free(evas);
}
EFL_END_TEST

static int _signal_count;

static void
_signal_callback_count_cb(void *data, Evas_Object *obj EINA_UNUSED,
                         const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   int *_data = data;
   _signal_count += *_data;
}

EFL_START_TEST(edje_test_signal_callback_del_full)
{
   Evas *evas;
   Evas_Object *obj;
   int data[4] = { 1, 2, 4, 8 };

   evas = _setup_evas();

   obj = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
                 efl_file_set(efl_added,
                 test_layout_get("test_signal_callback_del_full.edj"), "test"),
                 efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(320, 240)),
                 efl_gfx_entity_visible_set(efl_added, 1));

   edje_object_signal_callback_add(obj, "some_signal", "event", _signal_callback_count_cb, &data[0]);
   edje_object_signal_callback_add(obj, "some_signal", "event", _signal_callback_count_cb, &data[1]);
   edje_object_signal_callback_add(obj, "some_signal", "event", _signal_callback_count_cb, &data[2]);
   edje_object_signal_callback_add(obj, "some_signal", "event", _signal_callback_count_cb, &data[3]);

   edje_object_signal_callback_del_full(obj, "some_signal", "event", _signal_callback_count_cb, &data[0]);
   edje_object_signal_callback_del_full(obj, "some_signal", "event", _signal_callback_count_cb, &data[3]);

   edje_object_signal_emit(obj, "some_signal", "event");

   edje_object_message_signal_process(obj);
   ck_assert_int_eq(_signal_count, (data[1] + data[2]));

   efl_del(obj);

   evas_free(evas);
}
EFL_END_TEST

void edje_test_signal(TCase *tc)
{
   tcase_add_test(tc, edje_test_message_send_legacy);
   tcase_add_test(tc, edje_test_message_send_eo);
   tcase_add_test(tc, edje_test_signals);
   tcase_add_test(tc, edje_test_signal_callback_del_full);

}
