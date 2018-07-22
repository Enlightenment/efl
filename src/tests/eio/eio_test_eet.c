#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <Ecore.h>
#include <Eio.h>
#include <Eet.h>

#include "eio_suite.h"

Eet_File *ee;

static void
_open_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, Eet_File *ef)
{
   ee = ef;
   ecore_main_loop_quit();
}

static void
_done_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_write_done_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int i)
{
   fail_if (i <= 0);
   ecore_main_loop_quit();
}

static void
_read_done_cb(void *data, Eio_File *handler EINA_UNUSED, void *read_data,
              unsigned int size)
{
   char *str = data;
   char *read_str = read_data;

   fail_if(!read_str);
   fail_if(size != strlen(str) + 1);
   fail_if(memcmp(str, read_str, strlen(str) + 1) != 0);

   ecore_main_loop_quit();
}

static void
_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)
{
   fprintf(stderr, "Error:%s\n", strerror(error));
   fail();
   ecore_main_loop_quit();
}

static void
_eet_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, Eet_Error err EINA_UNUSED)
{
   fail();
   ecore_main_loop_quit();
}

EFL_START_TEST(eio_test_eet_cipher_decipher)
{
   int ret;
   char *data = "This is the data to save in file";
   const char *key = "This is a secret key";
   Eio_File *ef;
   Eina_Tmpstr *file;

   eet_init();

   ret = eina_file_mkstemp("eio_eet_example_XXXXXX", &file);
   fail_if(ret == -1);

   ef = eio_eet_open(file, EET_FILE_MODE_WRITE, _open_cb, _error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_eet_write_cipher(ee, "keys/tests", data, strlen(data) + 1, 0,
                             key, _write_done_cb, _error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_eet_close(ee, _done_cb, _eet_error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_eet_open(file, EET_FILE_MODE_READ, _open_cb, _error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_eet_read_cipher(ee, "keys/tests", key, _read_done_cb,
                            _error_cb, data);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_eet_close(ee, _done_cb, _eet_error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   eina_tmpstr_del(file);
   eet_shutdown();
}
EFL_END_TEST

typedef struct
{
   unsigned int id;
   const char *name;
} Test_Struct;

static Eet_Data_Descriptor *_test_struct_descriptor;

static void
_test_struct_descriptor_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Test_Struct);
   _test_struct_descriptor = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_test_struct_descriptor, Test_Struct,
                                 "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_test_struct_descriptor, Test_Struct,
                                 "name", name, EET_T_STRING);
}

static Test_Struct *
_test_struct_new(void)
{
   Test_Struct *tc = calloc(1, sizeof(Test_Struct));
   fail_if(!tc);

   tc->id = 1234;
   tc->name = "eio_eet_test";

   return tc;
}

static void
_data_read_done_cb(void *data, Eio_File *handler EINA_UNUSED, void *decoded)
{
   Test_Struct *res = decoded;
   Test_Struct *orig = data;

   fail_if(!res);
   fail_if(res->id != orig->id);
   fail_if(strcmp(res->name, orig->name) != 0);
   ecore_main_loop_quit();
}

EFL_START_TEST(eio_test_eet_data_cipher_decipher)
{
   int ret;
   const char *key = "This is a secret key";
   Eio_File *ef;
   Test_Struct *tc;
   Eina_Tmpstr *file;

   eet_init();

   _test_struct_descriptor_init();
   tc = _test_struct_new();

   ret = eina_file_mkstemp("eio_eet_example_XXXXXX", &file);
   fail_if(ret == -1);

   ef = eio_eet_open(file, EET_FILE_MODE_WRITE, _open_cb, _error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_eet_data_write_cipher(ee, _test_struct_descriptor, "test",
                                  key, tc, 0, _write_done_cb, _error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_eet_close(ee, _done_cb, _eet_error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_eet_open(file, EET_FILE_MODE_READ, _open_cb, _error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_eet_data_read_cipher(ee, _test_struct_descriptor, "test",
                                 key, _data_read_done_cb, _error_cb, tc);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_eet_close(ee, _done_cb, _eet_error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   eina_tmpstr_del(file);
   eet_shutdown();
}
EFL_END_TEST

void
eio_test_eet(TCase *tc)
{
   tcase_add_test(tc, eio_test_eet_cipher_decipher);
   tcase_add_test(tc, eio_test_eet_data_cipher_decipher);
}
