#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>

#include "ecore_con_suite.h"

#define COOKIEJAR "testcookieXXXXXX.jar"
#define DEFAULT_LINK "www.google.com"

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
   int status;

   printf("Total downloaded bytes = %d\n",
           ecore_con_url_received_bytes_get(ev->url_con));

   if (info && info->_tmpfd)
     {
        status = ecore_con_url_status_code_get(ev->url_con);
        fail_if(status != 220);
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

#if defined(ECORE_CON_HTTP_TEST_URL) || defined(ECORE_CON_FTP_TEST_URL)
static Eina_Bool
_parse_url(char *link, char *url, char **user, char **passwd, char **file, char **dir)
{
   int is_http;
   char *temp, *temp1, *link_cpy;

   if (!strncmp(link, "http://", 7))
     is_http = EINA_TRUE;
#ifdef ECORE_CON_FTP_TEST_URL
   else if (!strncmp(link, "ftp://", 6))
     is_http = EINA_FALSE;
#endif
   else
     return EINA_FALSE;

   link_cpy = strdup(link);
   temp = link + 6;
   if (is_http)
     temp ++;

   temp1 = strtok(temp, ":");
   if (temp1)
     *user = strdup(temp1);
   else
     goto error_user;

   temp1 = strtok(NULL, "@");
   if (temp1)
     *passwd = strdup(temp1);
   else
     goto error_passwd;

   if (is_http)
     {
        strcpy(url, "http://");
        temp1 = strrchr(link_cpy, '@') + 1;
        strcat(url, temp1);

        free(link_cpy);
        return EINA_TRUE;
     }

   strcpy(url, "ftp://");
   temp1 = strtok(NULL, "?");
   if (temp1)
     strcat(url, temp1);
   else
     goto error_passwd;

   if (strchr (link_cpy, '&'))
     {
        temp1 = strtok(NULL, "=");
        if (temp1)
          *file = strdup(strtok(NULL, "&"));
        else
          goto error_passwd;

        temp1 = strrchr(link_cpy, '=') + 1;
        if (temp1)
          *dir = strdup(temp1);
        else
          goto error_file;
     }
   else
     {
        temp1 = strrchr(link_cpy,'=') + 1;
        if (temp1)
          *file = strdup(temp1);
        else
          goto error_passwd;
     }

   free(link_cpy);
   return EINA_TRUE;

error_file:
   free(*file);

error_passwd:
   free(*user);

error_user:
   free(link_cpy);
   fprintf(stderr, "Wrong URL format\n");
   return EINA_FALSE;
}
#endif

#ifdef ECORE_CON_FTP_TEST_URL
EFL_START_TEST(ecore_con_test_ecore_con_url_ftp_upload)
{
   Ecore_Con_Url *ec_url;
   url_test *info = NULL;
   int ret;
   char link[] = ECORE_CON_FTP_TEST_URL;
   char url[4096], *username, *password, *file = NULL, *dir = NULL;

   ret = ecore_con_url_init();
   fail_if(ret != 1);

   fail_unless(_parse_url(link, url, &username, &password, &file, &dir));

   fprintf(stderr, "FTP: \n url = %s \n username = %s \n password = %s \n file = %s \n", url, username, password, file);
   if (dir)
      fprintf(stderr, "directory = %s\n", dir);

   ec_url = ecore_con_url_new(link);
   fail_if(!ec_url);

   ecore_con_url_verbose_set(ec_url, EINA_TRUE);
   ecore_con_url_ftp_use_epsv_set(ec_url, EINA_TRUE);

   fail_unless(ecore_con_url_ftp_upload(ec_url, file, username, password, dir));

   ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE,
                           _url_compl_cb, info);

   ret = ecore_con_url_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST
#endif

#ifdef ECORE_CON_HTTP_TEST_URL
EFL_START_TEST(ecore_con_test_ecore_con_url_post)
{
   Ecore_Con_Url *ec_url;
   url_test *info = NULL;
   int ret;
   char link[] = ECORE_CON_HTTP_TEST_URL;
   char url_data[] = "test";
   char *username = NULL, *password = NULL;
   char url[4096];

   ret = ecore_con_url_init();
   fail_if(ret != 1);

   fail_unless(_parse_url(link, url, &username, &password, NULL, NULL));

   fprintf (stderr, "HTTP: \n url = %s \n username = %s \n password = %s \n", url, username, password);

   ecore_con_url_pipeline_set(EINA_TRUE);
   fail_unless (ecore_con_url_pipeline_get());

   ec_url = ecore_con_url_custom_new(url, "POST");
   fail_unless (ec_url);

   ecore_con_url_additional_header_add(ec_url, "User-Agent", "blablabla");
   ecore_con_url_verbose_set(ec_url, EINA_TRUE);

   ecore_con_url_httpauth_set(ec_url, username, password, EINA_FALSE);
   ecore_con_url_time(ec_url, ECORE_CON_URL_TIME_IFMODSINCE, 0);

   fail_unless(ecore_con_url_post(ec_url, url_data, 4, NULL));

   ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE,
                           _url_compl_cb, info);

   ret = ecore_con_url_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST
#endif

EFL_START_TEST(ecore_con_test_ecore_con_url_download)
{
   Ecore_Con_Url *url;
   url_test *info;
   int ret;
#ifdef ECORE_CON_HTTP_TEST_URL
   const char link[] = ECORE_CON_HTTP_TEST_URL;
#else
   const char link[] = DEFAULT_LINK;
#endif
   char url_data[] = "test";

   ret = ecore_con_url_init();
   fail_if(ret != 1);

   url = ecore_con_url_new(link);
   fail_if(!url);

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
}
EFL_END_TEST

EFL_START_TEST(ecore_con_test_ecore_con_url_create)
{
   Ecore_Con_Url *url;
   int ret;

   ret = ecore_con_url_init();
   fail_if(ret != 1);

   url = ecore_con_url_new("http://google.com");
   fail_if(!url);

   ecore_con_url_free(url);

   ret = ecore_con_url_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST

EFL_START_TEST(ecore_con_test_ecore_con_url_init)
{
   int ret;

   ret = ecore_con_url_init();
   fail_if(ret != 1);

   ret = ecore_con_url_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST

static Eina_Bool
_url_cookies_compl_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event_info)
{
   Ecore_Con_Event_Url_Complete *url_complete = event_info;
   const Eina_List *headers, *l;
   char *str;

   headers = ecore_con_url_response_headers_get(url_complete->url_con);

   fail_if(!headers);

   printf("response headers:\n");
   EINA_LIST_FOREACH(headers, l, str)
     printf("header: %s", str);

   ecore_con_url_cookies_jar_write(url_complete->url_con);

   ecore_main_loop_quit();

   return EINA_TRUE;
}

static Ecore_Con_Url *
_ecore_con_url_cookies_test_init()
{
   Ecore_Con_Url *ec_url = NULL;
#ifdef ECORE_CON_HTTP_TEST_URL
   const char link[] = ECORE_CON_HTTP_TEST_URL;
#else
   const char link[] = DEFAULT_LINK;
#endif

   ecore_con_url_init();

   ec_url = ecore_con_url_new(link);
   fail_if(!ec_url);

   ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE,
                           _url_cookies_compl_cb, NULL);

   ecore_con_url_additional_header_add(ec_url, "User-Agent",
                                       "Ecore_Con client");

   ecore_con_url_cookies_init(ec_url);

   return ec_url;
}

static void
_ecore_con_url_cookies_test_shutdown(Ecore_Con_Url *ec_url, int tmpfd, Eina_Tmpstr **path)
{
   close(tmpfd);
   unlink(*path);
   eina_tmpstr_del(*path);
   ecore_con_url_free(ec_url);
   ecore_con_url_shutdown();
}

EFL_START_TEST(ecore_con_test_ecore_con_url_cookies_clear)
{
   Ecore_Con_Url *ec_url = _ecore_con_url_cookies_test_init();
   Eina_Tmpstr *path;
   int tmpfd;

   tmpfd = eina_file_mkstemp(COOKIEJAR, &path);

   ecore_con_url_cookies_file_add(ec_url, path);
   fail_unless(ecore_con_url_cookies_jar_file_set(ec_url, path));
   ecore_con_url_cookies_clear(ec_url);

   fail_unless(ecore_con_url_get(ec_url));

   ecore_main_loop_begin();

   _ecore_con_url_cookies_test_shutdown(ec_url, tmpfd, &path);
}
EFL_END_TEST

EFL_START_TEST(ecore_con_test_ecore_con_url_cookies_clear_session)
{
   Ecore_Con_Url *ec_url = _ecore_con_url_cookies_test_init();
   Eina_Tmpstr *path;
   int tmpfd;

   tmpfd = eina_file_mkstemp(COOKIEJAR, &path);

   ecore_con_url_cookies_file_add(ec_url, path);
   fail_unless(ecore_con_url_cookies_jar_file_set(ec_url, path));
   ecore_con_url_cookies_session_clear(ec_url);

   fail_unless(ecore_con_url_get(ec_url));

   ecore_main_loop_begin();

   _ecore_con_url_cookies_test_shutdown(ec_url, tmpfd, &path);
}
EFL_END_TEST

EFL_START_TEST(ecore_con_test_ecore_con_url_cookies_ignore_session)
{
   Ecore_Con_Url *ec_url = _ecore_con_url_cookies_test_init();
   Eina_Tmpstr *path;
   int tmpfd;

   tmpfd = eina_file_mkstemp(COOKIEJAR, &path);

   fail_unless(ecore_con_url_cookies_jar_file_set(ec_url, path));
   ecore_con_url_cookies_ignore_old_session_set(ec_url,EINA_TRUE);

   fail_unless(ecore_con_url_get(ec_url));

   ecore_main_loop_begin();

   _ecore_con_url_cookies_test_shutdown(ec_url, tmpfd, &path);
}
EFL_END_TEST

void ecore_con_test_ecore_con_url(TCase *tc)
{
   tcase_add_test(tc, ecore_con_test_ecore_con_url_init);
   tcase_add_test(tc, ecore_con_test_ecore_con_url_create);
   tcase_add_test(tc, ecore_con_test_ecore_con_url_download);
   tcase_add_test(tc, ecore_con_test_ecore_con_url_cookies_clear);
   tcase_add_test(tc, ecore_con_test_ecore_con_url_cookies_clear_session);
   tcase_add_test(tc, ecore_con_test_ecore_con_url_cookies_ignore_session);
#ifdef ECORE_CON_HTTP_TEST_URL
   tcase_add_test(tc, ecore_con_test_ecore_con_url_post);
#endif
#ifdef ECORE_CON_FTP_TEST_URL
   tcase_add_test(tc, ecore_con_test_ecore_con_url_ftp_upload);
#endif
}
