#include <stdio.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define __UNUSED__
#endif

#define COOKIEJAR "cookies.jar"

static Eina_Bool
_url_data_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Con_Event_Url_Data *url_data = event_info;
   int i;

   printf("\nData received from server:\n>>>>>\n");
   for (i = 0; i < url_data->size; i++)
     printf("%c", url_data->data[i]);
   printf("\n>>>>>>\n\n");

   return EINA_TRUE;
}

static Eina_Bool
_url_complete_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Con_Event_Url_Complete *url_complete = event_info;
   const Eina_List *headers, *l;
   char *str;

   printf("\n");
   printf("download completed with status code: %d\n", url_complete->status);

   headers = ecore_con_url_response_headers_get(url_complete->url_con);

   printf("response headers:\n");
   EINA_LIST_FOREACH(headers, l, str)
      printf("header: %s", str);

   ecore_con_url_cookies_jar_write(url_complete->url_con);

   ecore_main_loop_quit();

   return EINA_TRUE;
}

int main(int argc, const char *argv[])
{
   Ecore_Con_Url *ec_url = NULL;
   char cmd = '\0';
   Eina_Bool r;

   if (argc < 2)
     {
	printf("need at least one parameter: <url> [command]\n");
	return -1;
     }

   if (argc > 2)
     cmd = argv[2][0];

   ecore_init();
   ecore_con_init();
   ecore_con_url_init();

   ec_url = ecore_con_url_new(argv[1]);
   if (!ec_url)
     {
	printf("error when creating ecore con url object.\n");
	goto end;
     }

   ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA, _url_data_cb, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_complete_cb, NULL);

   ecore_con_url_additional_header_add(ec_url, "User-Agent", "Ecore_Con client");

   ecore_con_url_cookies_init(ec_url);
   if (cmd != 'c' && cmd != 's')
     ecore_con_url_cookies_file_add(ec_url, COOKIEJAR);
   ecore_con_url_cookies_jar_file_set(ec_url, COOKIEJAR);

   switch (cmd)
     {
      case 'c': // clear
	 printf("Cleaning previously set cookies.\n");
	 ecore_con_url_cookies_clear(ec_url);
	 break;
      case 's': // clear session
	 printf("Cleaning previously set session cookies.\n");
	 ecore_con_url_cookies_session_clear(ec_url);
	 break;
      case 'i': // ignore session
	 printf("Ignoring old session cookies.\n");
	 ecore_con_url_cookies_ignore_old_session_set(ec_url, EINA_TRUE);
     }

   r = ecore_con_url_get(ec_url);
   if (!r)
     {
	printf("could not realize request.\n");
	goto free_ec_url;
     }

   ecore_main_loop_begin();

free_ec_url:
   ecore_con_url_free(ec_url);
end:
   ecore_con_url_shutdown();
   ecore_con_shutdown();
   ecore_shutdown();

   return 0;
}
