#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <Ecore.h>
#include <Ecore_Con.h>

struct _request {
     long size;
};

static Eina_Bool
_url_progress_cb(void *data, int type, void *event_info)
{
   Ecore_Con_Event_Url_Progress *url_progress = event_info;
   float percent;

   if (url_progress->down.total > 0)
     {
	struct _request *req = ecore_con_url_data_get(url_progress->url_con);
	req->size = url_progress->down.now;

	percent = (url_progress->down.now / url_progress->down.total) * 100;
	printf("Total of download complete: %0.1f (%0.0f)%%\n",
	       percent, url_progress->down.now);
     }

   return EINA_TRUE;
}

static Eina_Bool
_url_complete_cb(void *data, int type, void *event_info)
{
   Ecore_Con_Event_Url_Complete *url_complete = event_info;

   struct _request *req = ecore_con_url_data_get(url_complete->url_con);
   int nbytes = ecore_con_url_received_bytes_get(url_complete->url_con);

   printf("\n");
   printf("download completed with status code: %d\n", url_complete->status);
   printf("Total size of downloaded file: %ld bytes\n", req->size);
   printf("Total size of downloaded file: %ld bytes "
	  "(from received_bytes_get)\n", nbytes);
   ecore_main_loop_quit();

   return EINA_TRUE;
}

int main(int argc, const char *argv[])
{
   Ecore_Con_Url *ec_url = NULL;
   struct _request *req;
   int fd;
   const char *filename = "downloadedfile.dat";

   if (argc < 2)
     {
	printf("need one parameter: <url>\n");
	return -1;
     }

   fd = open(filename, O_CREAT|O_WRONLY|O_TRUNC, 0644);

   if (fd == -1)
     {
	printf("error: could not open file for writing: \"%s\"\n",
	       filename);
	return -1;
     }

   ecore_init();
   ecore_con_init();
   ecore_con_url_init();

   ec_url = ecore_con_url_new(argv[1]);
   if (!ec_url)
     {
	printf("error when creating ecore con url object.\n");
	goto end;
     }

   req = malloc(sizeof(*req));
   req->size = 0;
   ecore_con_url_data_set(ec_url, req);

   ecore_con_url_fd_set(ec_url, fd);

   ecore_event_handler_add(ECORE_CON_EVENT_URL_PROGRESS, _url_progress_cb, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_complete_cb, NULL);

   if (!ecore_con_url_get(ec_url))
     {
	printf("could not realize request.\n");
	goto free_ec_url;
     }

   ecore_main_loop_begin();

free_ec_url:
   free(req);
   ecore_con_url_free(ec_url);
end:

   close(fd);
   ecore_con_url_shutdown();
   ecore_con_shutdown();
   ecore_shutdown();

   return 0;
}
