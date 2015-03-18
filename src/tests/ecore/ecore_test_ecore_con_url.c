#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_suite.h"

#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Eina.h>
#include <unistd.h>

typedef struct _url_test
{
   const char *_test_file;
   int _tmpfd;
} url_test;

static void
_free_url_test(url_test *info)
{
   unlink(info->_test_file);
   eina_tmpstr_del(info->_test_file);
   close(info->_tmpfd);
   free(info);
}

static Eina_Bool
_url_compl_cb(void *data, int type EINA_UNUSED, void *event_info)
{
   url_test *info = data;
   Ecore_Con_Event_Url_Complete *ev = event_info;

   printf("Total downloaded bytes = %d\n",
           ecore_con_url_received_bytes_get(ev->url_con));

   if (info->_tmpfd)
     {
        _free_url_test(info);
        ecore_con_url_free(ev->url_con);
     }
   else
     {
        fail_unless(ecore_con_url_url_set(ev->url_con,
                    "ftp://ftp.kernel.org/pub/linux/kernel/README"));

        ecore_con_url_verbose_set (ev->url_con, EINA_FALSE);

        info->_tmpfd = eina_file_mkstemp("ecore_con_test_XXXXXX.html",
                                         &(info->_test_file));
        if (info->_tmpfd < 0)
          {
             free(info);
             ecore_con_url_free(ev->url_con);
             fail();
          }

        ecore_con_url_fd_set(ev->url_con, info->_tmpfd);
        if (!ecore_con_url_get(ev->url_con))
          {
             _free_url_test(info);
             ecore_con_url_free(ev->url_con);
             fail();
          }
     }

   return EINA_FALSE;
}

START_TEST(ecore_test_ecore_con_url_download)
{
   Ecore_Con_Url *url;
   url_test *info;
   int ret;
   const char link[] = "www.google.com";
   char url_data[] = "test";

   ret = eina_init();
   fail_if(ret != 1);
   ret = ecore_con_url_init();
   fail_if(ret != 1);

   url = ecore_con_url_new(link);
   fail_unless (url);

   ecore_con_url_verbose_set(url, EINA_TRUE);

   fail_if (strcmp(ecore_con_url_url_get(url), link));

   ecore_con_url_data_set(url, url_data);
   fail_if (strcmp(ecore_con_url_data_get(url), url_data));

   info = (url_test *) malloc(sizeof(url_test));
   info->_tmpfd = 0;

   if (!ecore_con_url_get(url))
     {
        close(info->_tmpfd);
        free(info);
        ecore_con_url_free(url);
        fail();
     }
    else
     {
        ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE,
                                _url_compl_cb, info);
     }

   ret = ecore_con_url_shutdown();
   fail_if(ret != 0);
   ret = eina_shutdown();
}
END_TEST

START_TEST(ecore_test_ecore_con_url_create)
{
   Ecore_Con_Url *url;
   int ret;

   ret = eina_init();
   fail_if(ret != 1);
   ret = ecore_con_url_init();
   fail_if(ret != 1);

   url = ecore_con_url_new("http://google.com");
   fail_if(!url);

   ecore_con_url_free(url);

   ret = ecore_con_url_shutdown();
   fail_if(ret != 0);
   ret = eina_shutdown();
}
END_TEST

START_TEST(ecore_test_ecore_con_url_init)
{
   int ret;

   ret = ecore_con_url_init();
   fail_if(ret != 1);

   ret = ecore_con_url_shutdown();
   fail_if(ret != 0);
}
END_TEST

void ecore_test_ecore_con_url(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_con_url_init);
   tcase_add_test(tc, ecore_test_ecore_con_url_create);
   tcase_add_test(tc, ecore_test_ecore_con_url_download);
}
