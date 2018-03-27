#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Eet.h>

#include "eet_suite.h"
#include "eet_test_common.h"

typedef struct _Eet_Connection_Data Eet_Connection_Data;
struct _Eet_Connection_Data
{
   Eet_Connection      *conn;
   Eet_Data_Descriptor *edd;
   Eina_Bool            test;
};

static Eina_Bool
_eet_connection_read(const void *eet_data,
                     size_t      size,
                     void       *user_data)
{
   Eet_Connection_Data *dt = user_data;
   Eet_Test_Ex_Type *result;
   Eet_Node *node;
   int test;

   result = eet_data_descriptor_decode(dt->edd, eet_data, size);
   node = eet_data_node_decode_cipher(eet_data, NULL, size);

   /* Test the resulting data. */
   fail_if(!node);
   fail_if(eet_test_ex_check(result, 0, _dump_call) != 0);
   fail_if(eet_test_ex_check(eina_list_data_get(result->list), 1, _dump_call) != 0);
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

   if (!dt->test)
     {
        dt->test = EINA_TRUE;
        _dump_call = EINA_TRUE;
        fail_if(!eet_connection_node_send(dt->conn, node, NULL));
        _dump_call = EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eet_connection_write(const void *data,
                      size_t      size,
                      void       *user_data)
{
   Eet_Connection_Data *dt = user_data;
   int still;

   if (!dt->test)
     {
        int step = size / 3;

        eet_connection_received(dt->conn, data, step);
        eet_connection_received(dt->conn, (char *)data + step, step);
        size -= 2 * step;
        still = eet_connection_received(dt->conn, (char *)data + 2 * step, size);
     }
   else
     still = eet_connection_received(dt->conn, data, size);

   fail_if(still);

   return EINA_TRUE;
}

EFL_START_TEST(eet_test_connection_check)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   Eet_Connection_Data ecd;
   Eet_Test_Ex_Type etbt;
   Eina_Bool on_going;

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

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc),
                                             "Eet_Test_Ex_Type",
                                             sizeof(Eet_Test_Ex_Type));

   edd = eet_data_descriptor_stream_new(&eddc);
   fail_if(!edd);

   eet_build_ex_descriptor(edd, EINA_TRUE);

   /* Init context. */
   ecd.test = EINA_FALSE;
   ecd.edd = edd;

   /* Create a connection. */
   ecd.conn = eet_connection_new(_eet_connection_read, _eet_connection_write, &ecd);
   fail_if(!ecd.conn);

   /* Test the connection. */
   fail_if(!eet_connection_send(ecd.conn, edd, &etbt, NULL));

   fail_if(!ecd.test);

   fail_if(!eet_connection_close(ecd.conn, &on_going));

   fail_if(on_going);
}
EFL_END_TEST

void eet_test_connection(TCase *tc)
{
   tcase_add_test(tc, eet_test_connection_check);
}
