#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eet.h>
#include <Ecore_Con_Eet.h>

#include "ecore_con_suite.h"

#define TEST_STREAM "test_stream"
#define SRV_MSG "Server Message"
#define CLI_MSG "Client Message"

static Eet_Data_Descriptor *_ece_test_descriptor;
static char eet_data[] = "test_data";

typedef struct
{
   int id;
   char *message;
}ECE_Test;

static void
_ece_test_descriptor_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, ECE_Test);
   _ece_test_descriptor = eet_data_descriptor_stream_new(&eddc);

#define ECE_TEST_ADD_BASIC(member, eet_type) \
  EET_DATA_DESCRIPTOR_ADD_BASIC             \
    (_ece_test_descriptor, ECE_Test, # member, member, eet_type)

   ECE_TEST_ADD_BASIC(id, EET_T_INT);
   ECE_TEST_ADD_BASIC(message, EET_T_STRING);
#undef ECE_TEST_ADD_BASIC
}

static void
_ece_test_descriptor_shutdown(void)
{
   eet_data_descriptor_free(_ece_test_descriptor);
}

static Eina_Bool
_eet_client_discnct_cb(void *data EINA_UNUSED, Ecore_Con_Reply *reply EINA_UNUSED, Ecore_Con_Client *conn EINA_UNUSED)
{
   ecore_main_loop_quit();

   return EINA_FALSE;
}

static Eina_Bool
_eet_client_cnct_cb(void *data EINA_UNUSED, Ecore_Con_Reply *reply, Ecore_Con_Client *conn EINA_UNUSED)
{
   const char *ec_eet_data;
   Ecore_Con_Eet *ec_eet = ecore_con_eet_reply(reply);
   fail_if(!ec_eet);

   ec_eet_data = ecore_con_eet_data_get(ec_eet);
   fail_if(strcmp(ec_eet_data, eet_data));

   return EINA_TRUE;
}

static void
_eet_data_cb(void *data EINA_UNUSED, Ecore_Con_Reply *reply, const char *protocol_name, void *value)
{
   char toSend[] = SRV_MSG;
   ECE_Test *received = value;

   fail_if(strcmp(received->message, CLI_MSG));
   fail_if(received->id != 1);

   ecore_con_eet_raw_send(reply, protocol_name, "message", toSend,
                          strlen(toSend) + 1);
}

static Eina_Bool
_eet_svr_cnct_cb(void *data, Ecore_Con_Reply *reply, Ecore_Con_Server *conn EINA_UNUSED)
{
   int *i = data;
   ECE_Test toSend = {1, CLI_MSG};
   char rawMsg[] = CLI_MSG;

   if (*i)
     ecore_con_eet_send(reply, TEST_STREAM, &toSend);
   else
     ecore_con_eet_raw_send(reply, TEST_STREAM, "message", rawMsg,
                            strlen(rawMsg) + 1);

   return EINA_TRUE;
}

static void
_eet_raw_data_cb(void *data, Ecore_Con_Reply *reply, const char *protocol_name EINA_UNUSED, const char *section, void *value, size_t length)
{
   Ecore_Con_Server *cl = data;
   Ecore_Con_Eet *ece_cl = ecore_con_eet_reply(reply);

   fprintf(stderr, "Section: %s\n", section);
   fprintf(stderr, "Value: %s\n", (char *) value);
   fail_if(strncmp((char *) value, SRV_MSG, length));

   ecore_con_eet_raw_data_callback_del(ece_cl, TEST_STREAM);
   ecore_con_eet_server_connect_callback_del(ece_cl, _eet_svr_cnct_cb, NULL);

   ecore_con_server_del(cl);
}

EFL_START_TEST(ecore_con_test_ecore_con_eet_svr_cl)
{
   Ecore_Con_Eet *ece_svr, *ece_cl;
   Ecore_Con_Server *svr, *cl;
   int *cb_data = (int *) malloc(sizeof(int));
   *cb_data = 1;

   eet_init();
   _ece_test_descriptor_init();

   svr = ecore_con_server_add(ECORE_CON_REMOTE_TCP, "127.0.0.1",
                              12346, NULL);
   fail_if(!svr);

   ece_svr = ecore_con_eet_server_new(svr);
   fail_if(!ece_svr);

   ecore_con_eet_data_set(ece_svr, eet_data);

   ecore_con_eet_register(ece_svr, TEST_STREAM, _ece_test_descriptor);
   ecore_con_eet_data_callback_add(ece_svr, TEST_STREAM, _eet_data_cb,
                                   NULL);
   ecore_con_eet_client_connect_callback_add(ece_svr, _eet_client_cnct_cb,
                                             NULL);
   ecore_con_eet_client_disconnect_callback_add(ece_svr,
                                                _eet_client_discnct_cb, NULL);

   cl = ecore_con_server_connect(ECORE_CON_REMOTE_TCP, "127.0.0.1",
                                 12346, NULL);
   fail_if(!cl);

   ece_cl = ecore_con_eet_client_new(cl);
   fail_if(!ece_cl);

   ecore_con_eet_register(ece_cl, TEST_STREAM, _ece_test_descriptor);
   ecore_con_eet_raw_data_callback_add(ece_cl, TEST_STREAM,
                                       _eet_raw_data_cb, cl);
   ecore_con_eet_server_connect_callback_add(ece_cl, _eet_svr_cnct_cb,
                                             cb_data);

   ecore_main_loop_begin();


   ecore_con_eet_data_callback_del(ece_svr, TEST_STREAM);
   ecore_con_eet_client_connect_callback_del(ece_svr, _eet_client_cnct_cb,
                                             NULL);
   ecore_con_eet_client_disconnect_callback_del(ece_svr,
                                                _eet_client_discnct_cb, NULL);

   free(cb_data);
   _ece_test_descriptor_shutdown();
   ecore_con_server_del(svr);
   eet_shutdown();
}
EFL_END_TEST

static Eina_Bool
_eet_svr_discnct_cb(void *data EINA_UNUSED, Ecore_Con_Reply *reply EINA_UNUSED, Ecore_Con_Server *conn EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EINA_FALSE;
}

static void
_eet_svr_raw_data_cb(void *data, Ecore_Con_Reply *reply, const char *protocol_name EINA_UNUSED, const char *section, void *value, size_t length)
{
   Ecore_Con_Server *svr = data;
   Ecore_Con_Eet *ece_svr = ecore_con_eet_reply(reply);

   fprintf(stderr, "Section: %s\n", section);
   fprintf(stderr, "Value: %s\n", (char *) value);
   fail_if(strncmp((char *) value, CLI_MSG, length));

   ecore_con_eet_raw_data_callback_del(ece_svr, TEST_STREAM);

   ecore_con_server_del(svr);
}

EFL_START_TEST(ecore_con_test_ecore_con_eet_svr_del)
{
   Ecore_Con_Eet *ece_svr, *ece_cl;
   Ecore_Con_Server *svr, *cl;
   int *cb_data = (int *) malloc(sizeof(int));
   *cb_data = 0;

   eet_init();
   _ece_test_descriptor_init();

   svr = ecore_con_server_add(ECORE_CON_REMOTE_TCP, "127.0.0.1",
                              12346, NULL);
   fail_if(!svr);

   ece_svr = ecore_con_eet_server_new(svr);
   fail_if(!ece_svr);

   ecore_con_eet_data_set(ece_svr, eet_data);

   ecore_con_eet_register(ece_svr, TEST_STREAM, _ece_test_descriptor);
   ecore_con_eet_raw_data_callback_add(ece_svr, TEST_STREAM,
                                  _eet_svr_raw_data_cb, svr);

   cl = ecore_con_server_connect(ECORE_CON_REMOTE_TCP, "127.0.0.1",
                                 12346, NULL);
   fail_if(!cl);

   ece_cl = ecore_con_eet_client_new(cl);
   fail_if(!ece_cl);

   ecore_con_eet_register(ece_cl, TEST_STREAM, _ece_test_descriptor);
   ecore_con_eet_server_connect_callback_add(ece_cl, _eet_svr_cnct_cb,
                                             cb_data);
   ecore_con_eet_server_disconnect_callback_add(ece_cl,
                                                _eet_svr_discnct_cb, NULL);

   ecore_main_loop_begin();

   free(cb_data);
   _ece_test_descriptor_shutdown();
   eet_shutdown();
}
EFL_END_TEST

void ecore_con_test_ecore_con_eet(TCase *tc)
{
   tcase_add_test(tc, ecore_con_test_ecore_con_eet_svr_cl);
   tcase_add_test(tc, ecore_con_test_ecore_con_eet_svr_del);
}
