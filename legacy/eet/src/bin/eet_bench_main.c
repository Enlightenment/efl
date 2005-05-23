/* strdup requires BSD source */
#define _BSD_SOURCE

#include "Eet.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <fnmatch.h>
#include <sys/time.h>
#include <time.h>

double
get_time(void)
{
   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

int
main(int argc, char **argv)
{
   char *file;
   Eet_File *ef;
   char **items;
   int items_num;
   int i;
   double t1, t2;

   if (argc < 2)
     exit(-1);

   file = argv[1];
   ef = eet_open(file, EET_FILE_MODE_READ);
   if (ef)
     {
	items = eet_list(ef, "images/*", &items_num);
	for (i = 0; i < items_num; i++)
	  items[i] = strdup(items[i]);
	eet_close(ef);
     }
   else
     {
	printf("EEK. cannot load %s\n", file);
	exit(-1);
     }

   t1 = get_time();
   ef = eet_open(file, EET_FILE_MODE_READ);
   if (ef)
     {
	for (i = 0; i < items_num; i++)
	  {
	     int alpha, compress, quality, lossy;
	     unsigned int w, h;
	     void *data;

	     if (eet_data_image_header_read(ef, items[i], &w, &h, &alpha, &compress, &quality, &lossy))
	       {
		  data = eet_data_image_read(ef, items[i], &w, &h, &alpha, &compress, &quality, &lossy);
		  if (data) free(data);
		  else
		    {
		       printf("failure reading %s (%i/%i\n", items[i], i, items_num);
		       exit(-1);
		    }
	       }
	     else
	       {
		  printf("failure reading header %s (%i/%i\n", items[i], i, items_num);
		  exit(-1);
	       }
	  }
     }
   eet_close(ef);
   t2 = get_time();
   printf("1 open/close: %i img headers + data in %3.3f sec (%3.3f img/sec)\n",
	  items_num,
	  t2 - t1,
	  (double)items_num / (t2 - t1));
   t1 = get_time();
   for (i = 0; i < items_num; i++)
     {
	int alpha, compress, quality, lossy;
	unsigned int w, h;
	void *data;

	ef = eet_open(file, EET_FILE_MODE_READ);
	if (ef)
	  {
	     if (eet_data_image_header_read(ef, items[i], &w, &h, &alpha, &compress, &quality, &lossy))
	       {
		  data = eet_data_image_read(ef, items[i], &w, &h, &alpha, &compress, &quality, &lossy);
		  if (data) free(data);
		  else
		    {
		       printf("failure reading %s (%i/%i\n", items[i], i, items_num);
		       exit(-1);
		    }
	       }
	     else
	       {
		  printf("failure reading header %s (%i/%i\n", items[i], i, items_num);
		  exit(-1);
	       }
	     eet_close(ef);
	  }
     }
   t2 = get_time();
   printf("many open/close: %i img headers + data in %3.3f sec (%3.3f img/sec)\n",
	  items_num,
	  t2 - t1,
	  (double)items_num / (t2 - t1));
   return 0;
}
