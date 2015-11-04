#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_file_tests.h"
#include "ecordova_suite.h"

#include <Ecore_Con.h>
#include <Ecore_File.h>

#include <stdbool.h>

static void
_setup(void)
{
   int ret = ecordova_init();
   ck_assert_int_eq(ret, 1);
   ecore_con_init();
}

static void
_teardown(void)
{
   ecore_con_shutdown();
   int ret = ecordova_shutdown();
   ck_assert_int_eq(ret, 0);
}

static Ecordova_Device *
_filetransfer_new(void)
{
   return eo_add(ECORDOVA_FILETRANSFER_CLASS, NULL);
}

START_TEST(smoke)
{
   Ecordova_Device *filetransfer = _filetransfer_new();
   eo_unref(filetransfer);
}
END_TEST

static unsigned char *expected_content = NULL;
static size_t expected_size = 0;

static Eina_Bool
_client_add_cb(void *data EINA_UNUSED,
               int type EINA_UNUSED,
               Ecore_Con_Event_Client_Add *event EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_client_del_cb(void *data EINA_UNUSED,
               int type EINA_UNUSED,
               Ecore_Con_Event_Client_Del *event)
{
   ecore_con_client_del(event->client);
   return EINA_TRUE;
}

static Eina_Bool
_client_data_cb(void *data EINA_UNUSED,
                int type EINA_UNUSED,
                Ecore_Con_Event_Client_Data *event)
{
   const char response_template[] =
     "HTTP/1.1 200 OK\r\n"
     "Server: filetransfer test httpserver\r\n"
     "Content-Length: %s\r\n"
     "Connection: close\r\n"
     "Content-Type: application/octet-stream\r\n\r\n";

   char content_length[15];
   snprintf(content_length, sizeof(content_length), "%ld", expected_size);

   size_t min_len = strlen(response_template) + strlen(content_length) + expected_size;
   char *buffer = malloc(min_len);
   snprintf(buffer, min_len, response_template, content_length);

   size_t len = strlen(buffer);
   memcpy(&buffer[len], expected_content, expected_size);
   len += expected_size;

   ecore_con_client_send(event->client, buffer, len);
   ecore_con_client_flush(event->client);
   free(buffer);

   return EINA_TRUE;
}

static Eina_Bool
_server_del_cb(void *data EINA_UNUSED,
               int type EINA_UNUSED,
               Ecore_Con_Event_Server_Del *event EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EINA_TRUE;
}

static Eina_Bool
_main_loop_quit_cb(void *data EINA_UNUSED,
                   Eo *obj EINA_UNUSED,
                   const Eo_Event_Description *desc EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_set_bool(void *data,
          Eo *obj EINA_UNUSED,
          const Eo_Event_Description *desc EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   fail_if(NULL == data);
   bool *value = data;
   *value = true;
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_on_progress_cb(void *data EINA_UNUSED,
                Eo *obj EINA_UNUSED,
                const Eo_Event_Description *desc EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   return EO_CALLBACK_CONTINUE;
}

START_TEST(download)
{
   const int server_port = 8181;
   const char *server_address = "127.0.0.1";
   Ecore_Con_Server *httpserver = ecore_con_server_add(ECORE_CON_REMOTE_TCP, server_address, server_port, NULL);
   fail_if(NULL == httpserver);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, (Ecore_Event_Handler_Cb)_client_add_cb, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, (Ecore_Event_Handler_Cb)_client_del_cb, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb)_client_data_cb, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, (Ecore_Event_Handler_Cb)_server_del_cb, NULL);

   Eina_Tmpstr *file_url = NULL;
   Ecordova_FileEntry *file_entry = create_tmpfile(&file_url, &expected_content, &expected_size);
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *tmpdir_entry = _create_tmpdir(&tmpdir);

   const char *filename = ecore_file_file_get(file_url);
   const char *protocol = "http://";
   size_t len = strlen(protocol) + strlen(server_address) + 1 + 15 + 1 + strlen(filename) + 1;
   char source[len];
   snprintf(source, len, "%s%s:%d/%s", protocol, server_address, server_port, filename);

   const char *download_extension = ".download";
   len = strlen(file_url) + strlen(download_extension) + 1;
   char target[len];
   snprintf(target, len, "%s%s", file_url, download_extension);

   Ecordova_Device *filetransfer = _filetransfer_new();

   bool error = false;
   bool timeout = false;

   eo_do(filetransfer, eo_event_callback_add(ECORDOVA_FILETRANSFER_EVENT_DOWNLOAD_SUCCESS, _main_loop_quit_cb, NULL),
                       eo_event_callback_add(ECORDOVA_FILETRANSFER_EVENT_ON_PROGRESS, _on_progress_cb, NULL),
                       eo_event_callback_add(ECORDOVA_FILETRANSFER_EVENT_ERROR, _set_bool, &error),
                       ecordova_filetransfer_download(source, target, EINA_FALSE, NULL));

   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));
   ecore_main_loop_begin();
   eo_unref(timeout_timer);

   fail_if(error);
   fail_if(timeout);

   eo_unref(filetransfer);



   Ecordova_FileEntry *downloaded_entry = NULL;
   error = fileentry_get(tmpdir_entry, target, 0, &downloaded_entry);
   fail_if(error);
   fail_unless(NULL != downloaded_entry);
   check_exists(file_url);


   Ecordova_File *downloaded_file = NULL;
   fail_if(error = fileentry_file_get(downloaded_entry, &downloaded_file));

   char *actual_content;
   size_t actual_size;
   fail_if(error = filereader_read(downloaded_file, &actual_content, &actual_size));
   ck_assert_int_eq(expected_size, actual_size);
   ck_assert_int_eq(0, memcmp(expected_content, actual_content, expected_size));
   free(actual_content);

   eo_unref(downloaded_file);

   fail_if(error = entry_remove(downloaded_entry));
   eo_unref(downloaded_entry);
   fail_if(error = entry_remove(file_entry));
   eo_unref(file_entry);
   fail_if(error = entry_remove(tmpdir_entry));
   eo_unref(tmpdir_entry);
   free(expected_content);
   eina_tmpstr_del(file_url);
   eina_tmpstr_del(tmpdir);

   ecore_con_server_del(httpserver);
   ecore_main_loop_begin();
}
END_TEST

void
ecordova_filetransfer_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, download);
}
