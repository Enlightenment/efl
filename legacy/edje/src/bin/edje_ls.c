#include "Edje.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char      *file_out = NULL;
char      *progname = NULL;
Evas_List *collections = NULL;

static void
main_help(void)
{
   printf("Usage: "
      "%s [OPTIONS] input_file.edj ...\n"
      "\t-o outfile.txt    Output the listing of the collections to a file\n"
      , progname);
}

void
test_list(char *file)
{
   Evas_List *entries;

   entries = edje_file_collection_list(file);
   if (entries)
     {
	Evas_List *l;

	for (l = entries; l; l = l->next)
	  {
	     char *name;
	     char buf[1024];

	     name = l->data;
	     snprintf(buf, 1024, "%s: %s", file, name);
	     collections = evas_list_append(collections, strdup(buf));
	  }
	edje_file_collection_list_free(entries);
     }
}

int main(int argc, char **argv)
{
   int i;
   FILE *output;

   progname = argv[0];
   if (argc < 2)
     {
	main_help();
	exit(1);
     }

   edje_init();

   for (i = 1; i < argc; i++)
     {
	if (!strcmp(argv[i], "-h"))
	  {
	     main_help();
	     exit(0);
	  }
	else if ((!strcmp(argv[i], "-o")) && (i < (argc - 1)))
	  {
	     i++;	     
	     file_out = argv[i];
	  }
	else
	  {
	     test_list(argv[i]);
	  }
     }

   if (!collections)
     {
        main_help();
	exit(1);
     }

   if (file_out)
     {
	output = fopen(file_out, "w");
	if (!output)
	  {
	     perror("fopen");
	     main_help();
	     exit(1);
	  }
     }
   else
     output = stdout;

   while (collections)
     {
	char *name;

	name = collections->data;
	collections = evas_list_remove(collections, name);
	fprintf(output, "%s\n", name);
	free(name);
     }

   edje_shutdown();

   return 0;
}
