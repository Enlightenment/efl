#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include <Ecore_Audio.h>
#include <Ecore_File.h>

#include "ecore_suite.h"

#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Audio.h>

static Eina_Bool _failed_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
  Eina_Bool *pulse_context_failed = data;

  if (pulse_context_failed)
    {
       int pa_check = system("pulseaudio --check");
       *pulse_context_failed = (pa_check == 0);
    }
  ecore_main_loop_quit();

  return EINA_TRUE;
}

static Eina_Bool _finished_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
  ecore_main_loop_quit();

  return EINA_TRUE;
}

static Eina_Bool _looped_cb(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
  eo_do(obj, ecore_audio_obj_in_looped_set(EINA_FALSE));

  return EINA_TRUE;
}

#ifdef HAVE_PULSE
static Eina_Bool
_seek_vol(void *data)
{
   double len;
   Eo *in = data;
   Eo *out;

   eo_do(in, ecore_audio_obj_in_output_get(&out));

   eo_do(out, ecore_audio_obj_volume_set(0.4));
   eo_do(in, ecore_audio_obj_in_seek(-0.3, SEEK_END, &len));
   fail_if(len < 0);

   return EINA_FALSE;
}

START_TEST(ecore_test_ecore_audio_obj_pulse)
{
   Eo *in, *out;
   Eina_Bool ret;
   Eina_Bool pulse_context_failed = EINA_FALSE;

   in = eo_add(ECORE_AUDIO_OBJ_IN_SNDFILE_CLASS, NULL);
   fail_if(!in);

   eo_do(in, ecore_audio_obj_name_set("modem.wav"));
   eo_do(in, ecore_audio_obj_source_set(TESTS_SRC_DIR"/modem.wav", &ret));
   fail_if(!ret);

   out = eo_add(ECORE_AUDIO_OBJ_OUT_PULSE_CLASS, NULL);
   fail_if(!out);

   ecore_timer_add(0.3, _seek_vol, in);

   eo_do(in, eo_event_callback_add(ECORE_AUDIO_IN_EVENT_IN_STOPPED, _finished_cb, NULL));
   eo_do(out, eo_event_callback_add(ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_FAIL, _failed_cb, &pulse_context_failed));

   eo_do(out, ecore_audio_obj_out_input_attach(in, &ret));
   fail_if(!ret);

   ecore_main_loop_begin();
   fail_if(pulse_context_failed);

   eo_del(out);
   eo_del(in);
}
END_TEST
#endif

static Eina_Bool _quit(void *data EINA_UNUSED)
{
  ecore_main_loop_quit();

  return EINA_FALSE;
}

static Eina_Bool
_idle_del(void *data)
{
   Eo *in = data;
   eo_del(in);
   ecore_idler_add(_quit, NULL);

   return EINA_FALSE;
}

START_TEST(ecore_test_ecore_audio_cleanup)
{
   Eo *in, *out;
   int freq = 1000;
   Eina_Bool ret;

   in = eo_add(ECORE_AUDIO_OBJ_IN_TONE_CLASS, NULL);
   fail_if(!in);
   eo_do(in, eo_base_data_set(ECORE_AUDIO_ATTR_TONE_FREQ, &freq, NULL));
   eo_do(in, ecore_audio_obj_in_length_set(2));

   out = eo_add(ECORE_AUDIO_OBJ_OUT_SNDFILE_CLASS, NULL);
   fail_if(!out);
   eo_do(out, ecore_audio_obj_format_set(ECORE_AUDIO_FORMAT_OGG, &ret));
   fail_if(!ret);
   eo_do(out, ecore_audio_obj_source_set(TESTS_BUILD_DIR"/tmp.ogg", &ret));
   fail_if(!ret);

   eo_do(out, ecore_audio_obj_out_input_attach(in, &ret));
   fail_if(!ret);

   ecore_idler_add(_idle_del, in);

   ecore_main_loop_begin();

   ecore_file_remove(TESTS_BUILD_DIR"/tmp.ogg");
}
END_TEST

START_TEST(ecore_test_ecore_audio_obj_tone)
{
   Eo *in, *out;
   double len;
   int channel, rate, freq;
   Eina_Bool ret;
   char *tmp;

   in = eo_add(ECORE_AUDIO_OBJ_IN_TONE_CLASS, NULL);
   fail_if(!in);

   eo_do(in, ecore_audio_obj_name_set("tone"));

   eo_do(in, ecore_audio_obj_in_channels_get(&channel));
   fail_if(channel != 1);
   eo_do(in, ecore_audio_obj_in_samplerate_get(&rate));
   fail_if(rate != 44100);
   eo_do(in, ecore_audio_obj_in_length_get(&len));
   fail_if(len != 1);

   eo_do(in, ecore_audio_obj_in_length_set(2.5));
   eo_do(in, ecore_audio_obj_in_length_get(&len));
   fail_if(len != 2.5);

   eo_do(in, ecore_audio_obj_in_looped_set(EINA_TRUE));

   eo_do(in, ecore_audio_obj_in_remaining_get(&len));
   fail_if(len != 2.5);

   eo_do(in, eo_base_data_get(ECORE_AUDIO_ATTR_TONE_FREQ, (void **)&freq));
   fail_if(freq != 1000);

   freq = 2000;
   eo_do(in, eo_base_data_set(ECORE_AUDIO_ATTR_TONE_FREQ, &freq, NULL));

   eo_do(in, eo_base_data_get(ECORE_AUDIO_ATTR_TONE_FREQ, (void **)&freq));
   fail_if(freq != 2000);

   eo_do(in, eo_base_data_set("foo", "bar", NULL));
   eo_do(in, eo_base_data_get("foo", (void **)&tmp));
   ck_assert_str_eq(tmp, "bar");

   eo_do(in, ecore_audio_obj_in_seek(5.0, SEEK_SET, &len));
   fail_if(len != -1);

   eo_do(in, ecore_audio_obj_in_seek(1.0, 42, &len));
   fail_if(len != -1);

   eo_do(in, ecore_audio_obj_in_seek(1.0, SEEK_SET, &len));
   fail_if(len != 1.0);

   eo_do(in, ecore_audio_obj_in_remaining_get(&len));
   fail_if(len != 1.5);

   eo_do(in, ecore_audio_obj_in_seek(1.0, SEEK_CUR, &len));
   fail_if(len != 2.0);

   eo_do(in, ecore_audio_obj_in_remaining_get(&len));
   fail_if(len != 0.5);

   eo_do(in, ecore_audio_obj_in_seek(-1.0, SEEK_END, &len));
   fail_if(len != 1.5);

   eo_do(in, ecore_audio_obj_in_remaining_get(&len));
   fail_if(len != 1.0);

   out = eo_add(ECORE_AUDIO_OBJ_OUT_SNDFILE_CLASS, NULL);
   fail_if(!out);

   eo_do(out, ecore_audio_obj_name_set("tmp.wav"));
   eo_do(out, ecore_audio_obj_format_set(ECORE_AUDIO_FORMAT_WAV, &ret));
   fail_if(!ret);
   eo_do(out, ecore_audio_obj_source_set(TESTS_BUILD_DIR"/tmp.wav", &ret));
   fail_if(!ret);

   eo_do(out, ecore_audio_obj_out_input_attach(in, &ret));
   fail_if(!ret);

   eo_do(in, eo_event_callback_add(ECORE_AUDIO_IN_EVENT_IN_LOOPED, _looped_cb, NULL));
   eo_do(in, eo_event_callback_add(ECORE_AUDIO_IN_EVENT_IN_STOPPED, _finished_cb, NULL));

   ecore_main_loop_begin();

   eo_del(in);
   eo_del(out);

   //TODO: Compare and fail
   ecore_file_remove(TESTS_BUILD_DIR"/tmp.wav");
}
END_TEST

START_TEST(ecore_test_ecore_audio_obj_sndfile)
{
   Eo *in, *out;
   double len, rem;
   int channel, rate;
   Eina_Bool ret;
   Ecore_Audio_Format fmt;
   const char *src;

   in = eo_add(ECORE_AUDIO_OBJ_IN_SNDFILE_CLASS, NULL);
   fail_if(!in);

   eo_do(in, ecore_audio_obj_format_get(&fmt));
   fail_if(fmt != ECORE_AUDIO_FORMAT_AUTO);

   eo_do(in, ecore_audio_obj_format_set(ECORE_AUDIO_FORMAT_FLAC, &ret));
   fail_if(!ret);

   eo_do(in, ecore_audio_obj_format_get(&fmt));
   fail_if(fmt != ECORE_AUDIO_FORMAT_FLAC);

   eo_do(in, ecore_audio_obj_format_set(ECORE_AUDIO_FORMAT_AUTO, &ret));
   fail_if(!ret);

   eo_do(in, ecore_audio_obj_name_set("sms.ogg"));
   eo_do(in, ecore_audio_obj_source_set(TESTS_SRC_DIR"/sms.ogg", &ret));
   fail_if(!ret);

   eo_do(in, ecore_audio_obj_source_get(&src));
   ck_assert_str_eq(src, TESTS_SRC_DIR"/sms.ogg");

   eo_do(in, ecore_audio_obj_format_get(&fmt));
   fail_if(fmt != ECORE_AUDIO_FORMAT_OGG);

   eo_do(in, ecore_audio_obj_in_channels_get(&channel));
   fail_if(channel != 2);
   eo_do(in, ecore_audio_obj_in_samplerate_get(&rate));
   fail_if(rate != 44100);
   eo_do(in, ecore_audio_obj_in_length_get(&len));
   fail_if(len == 0);
   eo_do(in, ecore_audio_obj_in_remaining_get(&rem));
   fail_if(len != rem);

   eo_do(in, ecore_audio_obj_format_get(&fmt));
   fail_if(fmt != ECORE_AUDIO_FORMAT_OGG);

   eo_do(in, ecore_audio_obj_in_seek(0.5, SEEK_SET, &len));
   fail_if(len != 0.5);

   eo_do(in, ecore_audio_obj_in_seek(0.5, SEEK_CUR, &len));
   fail_if(len != 1.0);

   eo_do(in, ecore_audio_obj_in_seek(-1.0, SEEK_END, &len));
   fail_if(fabs(rem - 1 - len) > 0.1);

   out = eo_add(ECORE_AUDIO_OBJ_OUT_SNDFILE_CLASS, NULL);
   fail_if(!out);

   eo_do(out, ecore_audio_obj_name_set("tmp.wav"));
   eo_do(out, ecore_audio_obj_format_set(ECORE_AUDIO_FORMAT_WAV, &ret));
   fail_if(!ret);

   eo_do(out, ecore_audio_obj_format_get(&fmt));
   fail_if(fmt != ECORE_AUDIO_FORMAT_WAV);


//   eo_do(out, ecore_audio_obj_source_set("/tmp/file/does/not/exist/hopefully.wav", &ret));
//   fail_if(ret);

   eo_do(out, ecore_audio_obj_source_set(TESTS_BUILD_DIR"/tmp.wav", &ret));
   fail_if(!ret);

   eo_do(out, ecore_audio_obj_source_get(&src));
   ck_assert_str_eq(src, TESTS_BUILD_DIR"/tmp.wav");

   eo_do(out, ecore_audio_obj_out_input_attach(in, &ret));
   fail_if(!ret);

   eo_do(in, eo_event_callback_add(ECORE_AUDIO_IN_EVENT_IN_STOPPED, _finished_cb, NULL));

   ecore_main_loop_begin();

   eo_del(in);
   eo_del(out);

   //TODO: Compare and fail
   ecore_file_remove(TESTS_BUILD_DIR"/tmp.wav");
}
END_TEST

START_TEST(ecore_test_ecore_audio_obj_in_out)
{
  Eo *out2;
  Eina_List *in3;
  Eina_Bool attached;

  Eo *in = eo_add(ECORE_AUDIO_OBJ_IN_CLASS, NULL);
  Eo *in2 = eo_add(ECORE_AUDIO_OBJ_IN_CLASS, NULL);
  Eo *out = eo_add(ECORE_AUDIO_OBJ_OUT_CLASS, NULL);

  fail_if(!in);
  fail_if(!in2);
  fail_if(!out);

  fail_if(!eo_do(in, ecore_audio_obj_in_output_get(&out2)));

  fail_if(out2);

  fail_if(!eo_do(out, ecore_audio_obj_out_inputs_get(&in3)));

  fail_if(eina_list_count(in3) != 0);

  fail_if(!eo_do(out, ecore_audio_obj_out_input_attach(in, &attached)));
  fail_if(!attached);

  fail_if(!eo_do(out, ecore_audio_obj_out_input_attach(in, &attached)));
  fail_if(attached);

  fail_if(!eo_do(in, ecore_audio_obj_in_output_get(&out2)));

  fail_if(out2 != out);

  fail_if(!eo_do(out, ecore_audio_obj_out_inputs_get(&in3)));

  fail_if(eina_list_count(in3) != 1);
  fail_if(eina_list_data_get(in3) != in);

  fail_if(!eo_do(out, ecore_audio_obj_out_input_attach(in2, &attached)));
  fail_if(!attached);

  fail_if(!eo_do(out, ecore_audio_obj_out_inputs_get(&in3)));

  fail_if(eina_list_count(in3) != 2);
  fail_if(eina_list_data_get(in3) != in);

  eo_del(in2);

  fail_if(!eo_do(out, ecore_audio_obj_out_inputs_get(&in3)));

  fail_if(eina_list_count(in3) != 1);
  fail_if(eina_list_data_get(in3) != in);

  eo_del(out);

  fail_if(!eo_do(in, ecore_audio_obj_in_output_get(&out2)));

  fail_if(out2);

  eo_del(in);
}
END_TEST

static int read_cb(void *data EINA_UNUSED, Eo *eo_obj EINA_UNUSED, void *buffer, int len)
{
  static int i = 0;
  int j;
  uint8_t *buf = buffer;

  for (j=0;j<len; j++) {
      buf[j] = i++ %256;
  }
  return len;
}

static int write_cb(void *data EINA_UNUSED, Eo *eo_obj EINA_UNUSED, const void *buffer, int len)
{
  static int i = 0;
  int j;
  const uint8_t *buf = buffer;

  for (j=0;j<len; j++) {
      if (buf[j] != i%256)
        fail_if(buf[j] != i%256);
      i++;
  }

  if (i > 100000)
    ecore_main_loop_quit();

  return len;
}

Ecore_Audio_Vio in_vio = {
    .read = read_cb,
};

Ecore_Audio_Vio out_vio = {
    .write = write_cb,
};

START_TEST(ecore_test_ecore_audio_obj_vio)
{
  Eo *in, *out;

  in = eo_add(ECORE_AUDIO_OBJ_IN_CLASS, NULL);
  fail_if(!in);

  out = eo_add(ECORE_AUDIO_OBJ_OUT_CLASS, NULL);
  fail_if(!out);

  eo_do(in, ecore_audio_obj_vio_set(&in_vio, NULL, NULL));
  eo_do(out, ecore_audio_obj_vio_set(&out_vio, NULL, NULL));

  eo_do(out, ecore_audio_obj_out_input_attach(in, NULL));

  ecore_main_loop_begin();

  eo_del(out);
  eo_del(in);
}
END_TEST

static void _myfree(void *data)
{
  Eina_Bool *freed = data;

  *freed = EINA_TRUE;
}

START_TEST(ecore_test_ecore_audio_obj_in)
{
  int i;
  double speed, length;
  int samplerate, channels;
  Eina_Bool looped;

  ssize_t read;
  uint8_t buf[10];

  Ecore_Audio_Vio vio;
  Eina_Bool freed = EINA_FALSE;

  Eo *in = eo_add(ECORE_AUDIO_OBJ_IN_CLASS, NULL);

  fail_if(!in);

  fail_if(!eo_do(in, ecore_audio_obj_vio_set(&vio, &freed, _myfree)));
  fail_if(freed);

  fail_if(!eo_do(in, ecore_audio_obj_vio_set(NULL, NULL, NULL)));
  fail_if(!freed);

  fail_if(!eo_do(in, ecore_audio_obj_in_speed_get(&speed)));
  fail_if(speed != 1.0);

  fail_if(!eo_do(in, ecore_audio_obj_in_speed_set(2.5)));

  fail_if(!eo_do(in, ecore_audio_obj_in_speed_get(&speed)));
  fail_if(speed != 2.5);

  fail_if(!eo_do(in, ecore_audio_obj_in_speed_set(0)));

  fail_if(!eo_do(in, ecore_audio_obj_in_speed_get(&speed)));
  fail_if(speed != 0.2);

  fail_if(!eo_do(in, ecore_audio_obj_in_speed_set(10)));

  fail_if(!eo_do(in, ecore_audio_obj_in_speed_get(&speed)));
  fail_if(speed != 5.0);

  fail_if(!eo_do(in, ecore_audio_obj_in_samplerate_get(&samplerate)));
  fail_if(samplerate != 0);

  fail_if(!eo_do(in, ecore_audio_obj_in_samplerate_set(1234)));

  fail_if(!eo_do(in, ecore_audio_obj_in_samplerate_get(&samplerate)));
  fail_if(samplerate != 1234);

  fail_if(!eo_do(in, ecore_audio_obj_in_channels_get(&channels)));
  fail_if(channels != 0);

  fail_if(!eo_do(in, ecore_audio_obj_in_channels_set(2)));

  fail_if(!eo_do(in, ecore_audio_obj_in_channels_get(&channels)));
  fail_if(channels != 2);

  fail_if(!eo_do(in, ecore_audio_obj_in_looped_get(&looped)));
  fail_if(looped);

  fail_if(!eo_do(in, ecore_audio_obj_in_looped_set(EINA_TRUE)));

  fail_if(!eo_do(in, ecore_audio_obj_in_looped_get(&looped)));
  fail_if(!looped);

  fail_if(!eo_do(in, ecore_audio_obj_in_length_get(&length)));
  fail_if(length != 0);

  fail_if(eo_do(in, ecore_audio_obj_in_length_set(10.0)));

  fail_if(!eo_do(in, ecore_audio_obj_in_remaining_get(&length)));
  fail_if(length != -1);

  memset(buf, 0xaa, 10);
  fail_if(!eo_do(in, ecore_audio_obj_in_read(buf, 10, &read)));
  fail_if(read != 0);

  for (i=0; i<10; i++) {
      fail_if(buf[i] != 0xaa);
  }

  fail_if(!eo_do(in, ecore_audio_obj_paused_set(EINA_TRUE)));

  fail_if(!eo_do(in, ecore_audio_obj_in_read(buf, 10, &read)));
  fail_if(read != 10);

  for (i=0; i<10; i++) {
      fail_if(buf[i] != 0x00);
  }

  eo_del(in);
}
END_TEST

START_TEST(ecore_test_ecore_audio_obj)
{
  int i;
  const char *name;
  Eina_Bool paused;
  double volume;
  Eo *objs[2], *obj;

  objs[0] = eo_add(ECORE_AUDIO_OBJ_IN_CLASS, NULL);
  fail_if(!objs[0]);

  objs[1] = eo_add(ECORE_AUDIO_OBJ_OUT_CLASS, NULL);
  fail_if(!objs[1]);

  for (i=0; i<2; i++) {
    obj = objs[i];

    fail_if(!obj);

    fail_if(!eo_do(obj, ecore_audio_obj_name_get(&name)));

    fail_if(name);

    fail_if(!eo_do(obj, ecore_audio_obj_name_set("In1")));
    fail_if(!eo_do(obj, ecore_audio_obj_name_get(&name)));

    ck_assert_str_eq(name, "In1");

    fail_if(!eo_do(obj, ecore_audio_obj_name_get(NULL)));

    fail_if(!eo_do(obj, ecore_audio_obj_paused_get(&paused)));
    fail_if(paused);

    fail_if(!eo_do(obj, ecore_audio_obj_paused_set(EINA_TRUE)));
    fail_if(!eo_do(obj, ecore_audio_obj_paused_get(&paused)));
    fail_if(!paused);

    fail_if(!eo_do(obj, ecore_audio_obj_volume_get(&volume)));
    fail_if(volume != 1.0);

    fail_if(!eo_do(obj, ecore_audio_obj_volume_set(0.5)));
    fail_if(!eo_do(obj, ecore_audio_obj_volume_get(&volume)));
    fail_if(volume != 0.5);

    eo_del(obj);
  }

}
END_TEST

START_TEST(ecore_test_ecore_audio_init)
{
   int ret;

   ret = ecore_audio_init();
   ck_assert_int_eq(ret, 2);

   ret = ecore_audio_shutdown();
   ck_assert_int_eq(ret, 1);

}
END_TEST

void setup(void)
{
   int ret;

   ret = eina_init();
   ck_assert_int_eq(ret, 1);

   ret = ecore_init();
   fail_if(ret < 1);

   ret = ecore_audio_init();
   ck_assert_int_eq(ret, 1);
}

void teardown(void)
{
   ecore_audio_shutdown();
   ecore_shutdown();
   eina_shutdown();
}

void
ecore_test_ecore_audio(TCase *tc)
{
   tcase_add_checked_fixture (tc, setup, teardown);

   tcase_add_test(tc, ecore_test_ecore_audio_init);

   tcase_add_test(tc, ecore_test_ecore_audio_obj);
   tcase_add_test(tc, ecore_test_ecore_audio_obj_in);
   tcase_add_test(tc, ecore_test_ecore_audio_obj_vio);
   tcase_add_test(tc, ecore_test_ecore_audio_obj_in_out);
   tcase_add_test(tc, ecore_test_ecore_audio_obj_tone);

#ifdef HAVE_SNDFILE
   tcase_add_test(tc, ecore_test_ecore_audio_obj_sndfile);
   tcase_add_test(tc, ecore_test_ecore_audio_cleanup);
#endif
#ifdef HAVE_PULSE
   tcase_add_test(tc, ecore_test_ecore_audio_obj_pulse);
#endif

/*
   tcase_add_test(tc, ecore_test_ecore_audio_default);
   tcase_add_test(tc, ecore_test_ecore_audio_sndfile_vio);
   tcase_add_test(tc, ecore_test_ecore_audio_custom);
*/
}

