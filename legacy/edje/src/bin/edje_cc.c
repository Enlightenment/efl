#include "edje_cc.h"

static void main_help(void);

Evas_List *img_dirs = NULL;
char      *file_in = NULL;
char      *file_out = NULL;
char      *progname = NULL;

static void
main_help(void)
{
   printf
     ("Usage:\n"
      "\t%s [OPTIONS] input_file.edc output_file.eet\n"
      "\n"
      "Where OPTIONS is one or more of:\n"
      "\n"
      "-id image/directory      Add a directory to look in for relative path images\n"
      ,progname);
}

int
main(int argc, char **argv)
{
   int i;

   progname = argv[0];
   for (i = 1; i < argc; i++)
     {
	if (!strcmp(argv[i], "-h"))
	  {
	     main_help();
	     exit(0);
	  }
	else if ((!strcmp(argv[i], "-id")) && (i < (argc - 1)))
	  {
	     i++;	     
	     img_dirs = evas_list_append(img_dirs, argv[i]);
	  }
	else if (!file_in)
	  file_in = argv[i];
	else if (!file_out)
	  file_out = argv[i];
     }
   if (!file_in)
     {
	fprintf(stderr, "%s: Error: no input file specified.\n", progname);
	main_help();
	exit(-1);
     }
   if (!file_out)
     {
	fprintf(stderr, "%s: Error: no output file specified.\n", progname);
	main_help();
	exit(-1);
     }
   
   edje_file = calloc(1, sizeof(Edje_File));
   if (!edje_file)
     {
	fprintf(stderr, "%s: Error. memory allocation of %i bytes failed. %s\n",
		progname, sizeof(Edje_File), strerror(errno));
	exit(-1);
     }
   
   data_setup();   
   compile();
   data_write();
   return 0;
}
