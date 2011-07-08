#include <stdio.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>

static Eina_Bool
_url_data_cb(void *data, int type, void *event_info)
{
   Ecore_Con_Event_Url_Data *url_data = event_info;
   int i;

   for (i = 0; i < url_data->size; i++)
     printf("%c", url_data->data[i]);

   return EINA_TRUE;
}

static Eina_Bool
_url_complete_cb(void *data, int type, void *event_info)
{
   Ecore_Con_Event_Url_Complete *url_complete = event_info;
   const Eina_List *headers, *l;
   char *str;

   printf("\n");
   printf("download completed with status code: %d\n", url_complete->status);

   headers = ecore_con_url_response_headers_get(url_complete->url_con);

   EINA_LIST_FOREACH(headers, l, str)
      printf("header: %s\n", str);

   ecore_main_loop_quit();

   return EINA_TRUE;
}

int main(int argc, const char *argv[])
{
   Ecore_Con_Url *ec_url = NULL;
   const char *type;
   Eina_Bool r;

   if (argc < 3)
     {
	printf("need at least two parameters: < POST|GET >  <url1>\n");
	return -1;
     }

   type = argv[1];

   if (strcmp(type, "POST") && (strcmp(type, "GET")))
     {
	printf("only POST or GET are supported by this example.\n");
	return -1;
     }

   ecore_init();
   ecore_con_init();
   ecore_con_url_init();

   // check if requests are being pipelined, and set them if not:
   if (!ecore_con_url_pipeline_get())
     ecore_con_url_pipeline_set(EINA_TRUE);

   ec_url = ecore_con_url_custom_new(argv[2], type);
   if (!ec_url)
     {
	printf("error when creating ecore con url object.\n");
	goto end;
     }

   ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA, _url_data_cb, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_complete_cb, NULL);

   ecore_con_url_additional_header_add(ec_url, "User-Agent", "blablabla");
   ecore_con_url_verbose_set(ec_url, EINA_TRUE);

   ecore_con_url_httpauth_set(ec_url, "user", "password", EINA_FALSE);

   ecore_con_url_time(ec_url, ECORE_CON_URL_TIME_IFMODSINCE, 0);

   if (!strcmp(type, "GET"))
     r = ecore_con_url_get(ec_url);
   else
     r = ecore_con_url_post(ec_url, NULL, 0, NULL);

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
