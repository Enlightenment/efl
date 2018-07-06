#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>

#include <Eina.h>
#include <Eet.h>

#include "eet_suite.h"
#include "eet_test_common.h"

EFL_START_TEST(eet_test_file_simple_write)
{
   const char *buffer = "Here is a string of data to save !";
   Eina_Iterator *it;
   Eina_File *f;
   Eet_Entry *entry;
   Eet_File *ef;
   char *test;
   char *file;
   void *m;
   int size;
   int tmpfd;

   file = strdup("/tmp/eet_suite_testXXXXXX");

   fail_if(-1 == (tmpfd = mkstemp(file)));
   fail_if(!!close(tmpfd));

   fail_if(eet_mode_get(NULL) != EET_FILE_MODE_INVALID);

   ef = eet_open(file, EET_FILE_MODE_WRITE);
   fail_if(!ef);

   fail_if(!eet_write(ef, "keys/tests", buffer, strlen(buffer) + 1, 1));
   fail_if(!eet_alias(ef, "keys/alias", "keys/tests", 0));
   fail_if(!eet_alias(ef, "keys/alias2", "keys/alias", 1));

   fail_if(eet_mode_get(ef) != EET_FILE_MODE_WRITE);

   fail_if(eet_list(ef, "*", &size) != NULL);
   fail_if(eet_num_entries(ef) != -1);

   eet_close(ef);

   /* Test read from buffer */
   f = eina_file_open(file, EINA_FALSE);
   fail_if(!f);

   m = eina_file_map_all(f, EINA_FILE_WILLNEED);
   fail_if(!m);

   ef = eet_memopen_read(m, eina_file_size_get(f));
   fail_if(!ef);

   test = eet_read(ef, "keys/tests", &size);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(memcmp(test, buffer, strlen(buffer) + 1) != 0);

   test = eet_read(ef, "keys/alias2", &size);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(eet_read_direct(ef, "key/alias2", &size));

   fail_if(eet_mode_get(ef) != EET_FILE_MODE_READ);
   fail_if(eet_num_entries(ef) != 3);

   it = eet_list_entries(ef);
   fail_if(!it);
   EINA_ITERATOR_FOREACH(it, entry)
     fail_if(strcmp(entry->name, "keys/tests") &&
             strcmp(entry->name, "keys/alias") &&
             strcmp(entry->name, "keys/alias2"));

   eet_close(ef);

   eina_file_map_free(f, m);
   eina_file_close(f);

   /* Test read of simple file */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   test = eet_read(ef, "keys/tests", &size);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(memcmp(test, buffer, strlen(buffer) + 1) != 0);

   fail_if(strcmp(eet_alias_get(ef, "keys/alias"), "keys/tests"));
   test = eet_read(ef, "keys/alias2", &size);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(eet_read_direct(ef, "key/alias2", &size));

   fail_if(eet_mode_get(ef) != EET_FILE_MODE_READ);
   fail_if(eet_num_entries(ef) != 3);

   eet_close(ef);

   /* Test eet cache system */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   test = eet_read(ef, "keys/tests", &size);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(memcmp(test, buffer, strlen(buffer) + 1) != 0);

   eet_close(ef);

   fail_if(unlink(file) != 0);

}
EFL_END_TEST

EFL_START_TEST(eet_test_file_data)
{
   Eet_Data_Descriptor *edd;
   Eet_Test_Ex_Type *result;
   Eet_Dictionary *ed;
   Eet_File *ef;
   char **list;
   char *file;
   Eet_Data_Descriptor_Class eddc;
   Eet_Test_Ex_Type etbt;
   int size;
   int test;
   int tmpfd;

   file = strdup("/tmp/eet_suite_testXXXXXX");

   eet_test_ex_set(&etbt, 0);
   etbt.list = eina_list_prepend(etbt.list, eet_test_ex_set(NULL, 1));
   etbt.list = eina_list_prepend(etbt.list, eet_test_ex_set(NULL, 1));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY1, eet_test_ex_set(NULL, 2));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY2, eet_test_ex_set(NULL, 2));
   etbt.ilist = eina_list_prepend(etbt.ilist, &i42);
   etbt.ilist = eina_list_prepend(etbt.ilist, &i42);
   etbt.ihash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.ihash, EET_TEST_KEY1, &i7);
   etbt.ihash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.ihash, EET_TEST_KEY2, &i7);
   etbt.slist = eina_list_prepend(NULL, "test");
   etbt.shash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.shash, EET_TEST_KEY1, "test");
   memset(&etbt.charray, 0, sizeof(etbt.charray));
   etbt.charray[0] = "test";

   eet_test_setup_eddc(&eddc);
   eddc.name = "Eet_Test_Ex_Type";
   eddc.size = sizeof(Eet_Test_Ex_Type);

   edd = eet_data_descriptor_file_new(&eddc);
   fail_if(!edd);

   eet_build_ex_descriptor(edd, EINA_FALSE);

   fail_if(-1 == (tmpfd = mkstemp(file)));
   fail_if(!!close(tmpfd));

   /* Insert an error in etbt. */
   etbt.i = 0;

   /* Save the encoded data in a file. */
   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   fail_if(!eet_data_write(ef, edd, EET_TEST_FILE_KEY1, &etbt, 0));

   result = eet_data_read(ef, edd, EET_TEST_FILE_KEY1);
   fail_if(!result);

   fail_if(eet_mode_get(ef) != EET_FILE_MODE_READ_WRITE);

   /* Test string space. */
   ed = eet_dictionary_get(ef);

   fail_if(!eet_dictionary_string_check(ed, result->str));
   fail_if(eet_dictionary_string_check(ed, result->istr));

   eet_close(ef);

   /* Attempt to replace etbt by the correct one. */
   etbt.i = EET_TEST_INT;

   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   fail_if(!eet_data_write(ef, edd, EET_TEST_FILE_KEY1, &etbt, 0));

   result = eet_data_read(ef, edd, EET_TEST_FILE_KEY1);
   fail_if(!result);

   /* Test the resulting data. */
   fail_if(eet_test_ex_check(result, 0, EINA_FALSE) != 0);

   eet_close(ef);

   /* Read back the data. */
   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   fail_if(!eet_data_write(ef, edd, EET_TEST_FILE_KEY2, &etbt, 0));

   result = eet_data_read(ef, edd, EET_TEST_FILE_KEY1);
   fail_if(!result);

   /* Test string space. */
   ed = eet_dictionary_get(ef);
   fail_if(!ed);

   fail_if(!eet_dictionary_string_check(ed, result->str));
   fail_if(eet_dictionary_string_check(ed, result->istr));

   /* Test the resulting data. */
   fail_if(eet_test_ex_check(result, 0, EINA_FALSE) != 0);
   fail_if(eet_test_ex_check(eina_list_data_get(result->list), 1, EINA_FALSE) != 0);
   fail_if(eina_list_data_get(result->ilist) == NULL);
   fail_if(*((int *)eina_list_data_get(result->ilist)) != 42);
   fail_if(eina_list_data_get(result->slist) == NULL);
   fail_if(strcmp(eina_list_data_get(result->slist), "test") != 0);
   fail_if(eina_hash_find(result->shash, EET_TEST_KEY1) == NULL);
   fail_if(strcmp(eina_hash_find(result->shash, EET_TEST_KEY1), "test") != 0);
   fail_if(strcmp(result->charray[0], "test") != 0);

   test = 0;
   if (result->hash)
     eina_hash_foreach(result->hash, func, &test);

   fail_if(test != 0);
   if (result->ihash)
     eina_hash_foreach(result->ihash, func7, &test);

   fail_if(test != 0);

   list = eet_list(ef, "keys/*", &size);
   fail_if(eet_num_entries(ef) != 2);
   fail_if(size != 2);
   fail_if(!(strcmp(list[0],
                    EET_TEST_FILE_KEY1) == 0 &&
             strcmp(list[1], EET_TEST_FILE_KEY2) == 0)
           && !(strcmp(list[0],
                       EET_TEST_FILE_KEY2) == 0 &&
                strcmp(list[1], EET_TEST_FILE_KEY1) == 0));
   free(list);

   fail_if(eet_delete(ef, NULL) != 0);
   fail_if(eet_delete(NULL, EET_TEST_FILE_KEY1) != 0);
   fail_if(eet_delete(ef, EET_TEST_FILE_KEY1) == 0);

   list = eet_list(ef, "keys/*", &size);
   fail_if(size != 1);
   fail_if(eet_num_entries(ef) != 1);

   /* Test some more wrong case */
   fail_if(eet_data_read(ef, edd, "plop") != NULL);
   fail_if(eet_data_read(ef, edd, EET_TEST_FILE_KEY1) != NULL);

   /* Reinsert and reread data */
   fail_if(!eet_data_write(ef, edd, EET_TEST_FILE_KEY1, &etbt, 0));
   fail_if(eet_data_read(ef, edd, EET_TEST_FILE_KEY1) == NULL);
   fail_if(eet_read_direct(ef, EET_TEST_FILE_KEY1, &size) == NULL);

   eet_close(ef);

   fail_if(unlink(file) != 0);

}
EFL_END_TEST

EFL_START_TEST(eet_test_file_data_dump)
{
   Eet_Data_Descriptor *edd;
   Eet_Test_Ex_Type *result;
   Eet_Data_Descriptor_Class eddc;
   Eet_Test_Ex_Type etbt;
   Eet_File *ef;
   char *string1;
   char *file;
   int test;
   int tmpfd;

   file = strdup("/tmp/eet_suite_testXXXXXX");

   eet_test_ex_set(&etbt, 0);
   etbt.list = eina_list_prepend(etbt.list, eet_test_ex_set(NULL, 1));
   etbt.list = eina_list_prepend(etbt.list, eet_test_ex_set(NULL, 1));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY1, eet_test_ex_set(NULL, 2));
   eina_hash_add(etbt.hash, EET_TEST_KEY2, eet_test_ex_set(NULL, 2));
   etbt.ilist = eina_list_prepend(etbt.ilist, &i42);
   etbt.ilist = eina_list_prepend(etbt.ilist, &i42);
   etbt.ihash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.ihash, EET_TEST_KEY1, &i7);
   eina_hash_add(etbt.ihash, EET_TEST_KEY2, &i7);
   etbt.slist = eina_list_prepend(NULL, "test");
   etbt.shash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.shash, EET_TEST_KEY1, "test");
   memset(&etbt.charray, 0, sizeof(etbt.charray));
   etbt.charray[0] = "test";

   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc),
                                           "Eet_Test_Ex_Type",
                                           sizeof(Eet_Test_Ex_Type));

   edd = eet_data_descriptor_file_new(&eddc);
   fail_if(!edd);

   eet_build_ex_descriptor(edd, EINA_FALSE);

   fail_if(-1 == (tmpfd = mkstemp(file)));
   fail_if(!!close(tmpfd));

   /* Save the encoded data in a file. */
   ef = eet_open(file, EET_FILE_MODE_WRITE);
   fail_if(!ef);

   fail_if(!eet_data_write(ef, edd, EET_TEST_FILE_KEY1, &etbt, 0));

   eet_close(ef);

   /* Use dump/undump in the middle */
   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   string1 = NULL;
   fail_if(eet_data_dump(ef, EET_TEST_FILE_KEY1, append_string, &string1) != 1);
   fail_if(eet_delete(ef, EET_TEST_FILE_KEY1) == 0);
   fail_if(!eet_data_undump(ef, EET_TEST_FILE_KEY1, string1, strlen(string1), 1));

   eet_close(ef);

   /* Test the correctness of the reinsertion. */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   result = eet_data_read(ef, edd, EET_TEST_FILE_KEY1);
   fail_if(!result);

   eet_close(ef);

   /* Test the resulting data. */
   fail_if(eet_test_ex_check(result, 0, EINA_TRUE) != 0);
   fail_if(eet_test_ex_check(eina_list_data_get(result->list), 1, EINA_TRUE) != 0);
   fail_if(eina_list_data_get(result->ilist) == NULL);
   fail_if(*((int *)eina_list_data_get(result->ilist)) != 42);
   fail_if(eina_list_data_get(result->slist) == NULL);
   fail_if(strcmp(eina_list_data_get(result->slist), "test") != 0);
   fail_if(eina_hash_find(result->shash, EET_TEST_KEY1) == NULL);
   fail_if(strcmp(eina_hash_find(result->shash, EET_TEST_KEY1), "test") != 0);
   fail_if(strcmp(result->charray[0], "test") != 0);

   test = 0;
   _dump_call = EINA_TRUE;
   if (result->hash)
     eina_hash_foreach(result->hash, func, &test);
   _dump_call = EINA_FALSE;

   fail_if(test != 0);
   if (result->ihash)
     eina_hash_foreach(result->ihash, func7, &test);

   fail_if(test != 0);

   fail_if(unlink(file) != 0);

}
EFL_END_TEST

EFL_START_TEST(eet_test_file_fp)
{
   char *file;
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *edd_5FP;
   Eet_Data_Descriptor *edd_5DBL;
   Eet_File *ef;
   Eet_5FP origin;
   Eet_5DBL *convert;
   Eet_5FP *build;
   int tmpfd;

   file = strdup("/tmp/eet_suite_testXXXXXX");

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Eet_5FP);
   edd_5FP = eet_data_descriptor_file_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "fp32", fp32, EET_T_F32P32);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "fp16", fp16, EET_T_F16P16);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "fp8", fp8, EET_T_F8P24);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "f1", f1, EET_T_F32P32);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "f0", f0, EET_T_F32P32);

   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc), "Eet_5FP", sizeof (Eet_5DBL));
   edd_5DBL = eet_data_descriptor_file_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5DBL, Eet_5DBL, "fp32", fp32, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5DBL, Eet_5DBL, "fp16", fp16, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5DBL, Eet_5DBL, "fp8", fp8, EET_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5DBL, Eet_5DBL, "f1", f1, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5DBL, Eet_5DBL, "f0", f0, EET_T_DOUBLE);

   origin.fp32 = eina_f32p32_double_from(1.125);
   origin.fp16 = eina_f16p16_int_from(2000);
   origin.fp8 = eina_f8p24_int_from(125);
   origin.f1 = eina_f32p32_int_from(1);
   origin.f0 = 0;

   fail_if(-1 == (tmpfd = mkstemp(file)));
   fail_if(!!close(tmpfd));

   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   fail_if(!eet_data_write(ef, edd_5FP, EET_TEST_FILE_KEY1, &origin, 1));

   build = eet_data_read(ef, edd_5FP, EET_TEST_FILE_KEY1);
   fail_if(!build);

   convert = eet_data_read(ef, edd_5DBL, EET_TEST_FILE_KEY1);
   fail_if(!convert);

   fail_if(build->fp32 != eina_f32p32_double_from(1.125));
   fail_if(build->fp16 != eina_f16p16_int_from(2000));
   fail_if(build->fp8 != eina_f8p24_int_from(125));
   fail_if(build->f1 != eina_f32p32_int_from(1));
   fail_if(build->f0 != 0);

   fail_if(convert->fp32 != 1.125);
   fail_if(convert->fp16 != 2000);
   fail_if(convert->fp8 != 125);
   fail_if(convert->f1 != 1);
   fail_if(convert->f0 != 0);

   eet_close(ef);

   fail_if(unlink(file) != 0);

}
EFL_END_TEST

void eet_test_file(TCase *tc)
{
   tcase_add_test(tc, eet_test_file_simple_write);
   tcase_add_test(tc, eet_test_file_data);
   tcase_add_test(tc, eet_test_file_data_dump);
   tcase_add_test(tc, eet_test_file_fp);
}
