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

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* just some sample code on how to use encoder/decoders */
#if 0
#include <Evas.h>

typedef struct _blah2
{
   char *string;
}
Blah2;

typedef struct _blah3
{
   char *string;
}
Blah3;

typedef struct _blah
{
   char character;
   short sixteen;
   int integer;
   long long lots;
   float floating;
   double floating_lots;
   char *string;
   Blah2 *blah2;
   Evas_List *blah3;
}
Blah;

void
encdectest(void)
{
   Blah blah;
   Blah2 blah2;
   Blah3 blah3;
   Eet_Data_Descriptor *edd, *edd2, *edd3;
   void *data;
   int size;
   FILE *f;
   Blah *blah_in;

   edd3 = eet_data_descriptor_new("blah3", sizeof(Blah3),
				  evas_list_next,
				  evas_list_append,
				  evas_list_data,
				  evas_list_free,
				  evas_hash_foreach,
				  evas_hash_add,
				  evas_hash_free);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd3, Blah3, "string3", string, EET_T_STRING);

   edd2 = eet_data_descriptor_new("blah2", sizeof(Blah2),
				  evas_list_next,
				  evas_list_append,
				  evas_list_data,
				  evas_list_free,
				  evas_hash_foreach,
				  evas_hash_add,
				  evas_hash_free);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd2, Blah2, "string2", string, EET_T_STRING);

   edd = eet_data_descriptor_new("blah", sizeof(Blah),
				  evas_list_next,
				  evas_list_append,
				  evas_list_data,
				  evas_list_free,
				  evas_hash_foreach,
				  evas_hash_add,
				  evas_hash_free);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "character", character, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "sixteen", sixteen, EET_T_SHORT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "integer", integer, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "lots", lots, EET_T_LONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "floating", floating, EET_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "floating_lots", floating_lots, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "string", string, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_SUB  (edd, Blah, "blah2", blah2, edd2);
   EET_DATA_DESCRIPTOR_ADD_LIST (edd, Blah, "blah3", blah3, edd3);

   blah3.string="PANTS";

   blah2.string="subtype string here!";

   blah.character='7';
   blah.sixteen=0x7777;
   blah.integer=0xc0def00d;
   blah.lots=0xdeadbeef31337777;
   blah.floating=3.141592654;
   blah.floating_lots=0.777777777777777;
   blah.string="bite me like a turnip";
   blah.blah2 = &blah2;
   blah.blah3 = evas_list_append(NULL, &blah3);
   blah.blah3 = evas_list_append(blah.blah3, &blah3);
   blah.blah3 = evas_list_append(blah.blah3, &blah3);
   blah.blah3 = evas_list_append(blah.blah3, &blah3);
   blah.blah3 = evas_list_append(blah.blah3, &blah3);
   blah.blah3 = evas_list_append(blah.blah3, &blah3);
   blah.blah3 = evas_list_append(blah.blah3, &blah3);

   data = eet_data_descriptor_encode(edd, &blah, &size);
   f = fopen("out", "wb");
   if (f)
     {
	fwrite(data, size, 1, f);
	fclose(f);
     }
   printf("-----DECODING\n");
   blah_in = eet_data_descriptor_decode(edd, data, size);
   printf("-----DECODED!\n");
   printf("%c\n", blah_in->character);
   printf("%x\n", (int)blah_in->sixteen);
   printf("%x\n", blah_in->integer);
   printf("%lx\n", blah_in->lots);
   printf("%f\n", (double)blah_in->floating);
   printf("%f\n", (double)blah_in->floating_lots);
   printf("%s\n", blah_in->string);
   printf("%p\n", blah_in->blah2);
   printf("  %s\n", blah_in->blah2->string);
     {
	Evas_List *l;

	for (l = blah_in->blah3; l; l = l->next)
	  {
	     Blah3 *blah3_in;

	     blah3_in = l->data;
	     printf("%p\n", blah3_in);
	     printf("  %s\n", blah3_in->string);
	  }
     }
   eet_data_descriptor_free(edd);
   eet_data_descriptor_free(edd2);
   eet_data_descriptor_free(edd3);

   exit(0);
}
#endif

int eet_mkdir(char *dir);
void eet_mkdirs(char *s);

void depak_file(Eet_File *ef, char *file);
void depack(char *pak_file);

void list(char *pak_file);

void pak_file(Eet_File *ef, char *file, char **noz, int noz_num);
void pak_dir(Eet_File *ef, char *dir, char **noz, int noz_num);
void pack(char *pak_file, char **files, int count, char **noz, int noz_num);

int
eet_mkdir(char *dir)
{
#ifdef __MINGW32__
   if (mkdir(dir) < 0) return 0;
#else
   mode_t default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP
                         | S_IROTH | S_IXOTH;

   if (mkdir(dir, default_mode) < 0) return 0;
#endif

   return 1;
}

void
eet_mkdirs(char *s)
{
   char ss[PATH_MAX];
   int  i, ii;

   i = 0;
   ii = 0;
   while (s[i])
     {
	ss[ii++] = s[i];
	ss[ii] = 0;
	if (s[i] == '/') eet_mkdir(ss);
	i++;
     }
}

void
depak_file(Eet_File *ef, char *file)
{
   void *data;
   int size;
   char *last;

   data = eet_read(ef, file, &size);
   if (data)
     {
	FILE *f;
	char buf[PATH_MAX];
	int len;

	strncpy(buf, file, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = 0;
	if (buf[0] == '/') return;
	if (!strcmp(buf, "..")) return;
	if (!strncmp(buf, "../", 3)) return;
	if (strstr(buf, "/../")) return;
	len = strlen(buf);
	if (len >= 3)
	  {
	     if (!strcmp(&(buf[len - 3]), "/..")) return;
	  }
	last = strrchr(buf, '/');
	if (last)
	  {
	     last[1] = 0;
	     eet_mkdirs(buf);
	  }

	f = fopen(file, "wb");
	if (f)
	  {
	     fwrite(data, 1, size, f);
	     fclose(f);
	     printf("exported: %s\n", file);
	  }
	else
	  printf("error exporting: %s\n", file);
	free(data);
     }
   else
     {
	printf("error reading: %s\n", file);
     }
}

void
depack(char *pak_file)
{
   int i, num;
   char **list;
   Eet_File *ef;

   ef = eet_open(pak_file, EET_FILE_MODE_READ);
   if (!ef)
     {
	printf("cannot open for reading: %s\n", pak_file);
	return;
     }
   list = eet_list(ef, "*", &num);
   if (list)
     {
	for (i = 0; i < num; i++)
	  depak_file(ef, list[i]);
	free(list);
     }
   eet_close(ef);
}

void
list(char *pak_file)
{
   int i, num;
   char **list;
   Eet_File *ef;

   ef = eet_open(pak_file, EET_FILE_MODE_READ);
   if (!ef)
     {
	printf("cannot open for reading: %s\n", pak_file);
	return;
     }
   list = eet_list(ef, "*", &num);
   if (list)
     {
	for (i = 0; i < num; i++)
	  printf("%s\n",list[i]);
	free(list);
     }
   eet_close(ef);
}

void
pak_file(Eet_File *ef, char *file, char **noz, int noz_num)
{
   struct stat st;

   if (stat(file, &st) >= 0)
     {
	void *buf;

	buf = malloc(st.st_size);
	if (buf)
	  {
	     FILE *f;

	     f = fopen(file, "rb");
	     if (f)
	       {
		  int compress = 1;
		  int i;

		  for (i = 0; i < noz_num; i++)
		    {
		       if (!fnmatch(noz[i], file, 0))
			 {
			    compress = 0;
			    break;
			 }
		    }
		  fread(buf, 1, st.st_size, f);
		  if (!eet_write(ef, file, buf, st.st_size, compress))
		    printf("error importing: %s\n", file);
		  else
		    {
		       if (compress)
			 printf("compress: %s\n", file);
		       else
			 printf("imported: %s\n", file);
		    }
		  fclose(f);
	       }
	     free(buf);
	  }
     }
}

void
pak_dir(Eet_File *ef, char *dir, char **noz, int noz_num)
{
   DIR *dirp;
   struct dirent *dp;

   dirp = opendir(dir);
   if (!dirp)
     pak_file(ef, dir, noz, noz_num);
   else
     {
	while ((dp = readdir(dirp)))
	  {
	     char buf[PATH_MAX];

	     if ((!strcmp(".", dp->d_name)) || (!strcmp("..", dp->d_name)))
	       {
	       }
	     else
	       {
		  snprintf(buf, sizeof(buf), "%s/%s", dir, dp->d_name);
		  pak_dir(ef, buf, noz, noz_num);
	       }
	  }
     }
}

void
pack(char *pak_file, char **files, int count, char **noz, int noz_num)
{
   Eet_File *ef;
   int i;

   ef = eet_open(pak_file, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	printf("cannot open for writing: %s\n", pak_file);
	return;
     }
   for (i = 0; i < count; i++) pak_dir(ef, files[i], noz, noz_num);
   printf("done.\n");
   eet_close(ef);
}

int
main(int argc, char **argv)
{
   if (argc == 3)
     {
	if (!strcmp(argv[1], "-d"))
	  {
	     depack(argv[2]);
	     return 0;
	  }
	else if (!strcmp(argv[1], "-l"))
	  {
	     list(argv[2]);
	     return 0;
	  }
     }
   else if (argc > 3)
     {
	char **noz     = NULL;
	int    noz_num = 0;

	if (!strcmp(argv[1], "-c"))
	  {
	     int i;

	     for (i = 3; i < argc; i++)
	       {
		  if (!strcmp(argv[i], "-nz"))
		    {
		       if (i < (argc - 1))
			 {
			    i++;
			    noz_num++;
			    noz = realloc(noz, noz_num * sizeof(char *));
			    noz[noz_num - 1] = argv[i];
			 }
		    }
		  else
		    break;
	       }
	     pack(argv[2], &(argv[i]), argc - i, noz, noz_num);
	     return 0;
	  }
     }
   printf("usage:\n"
	  "  %s -l in_file\n"
	  "  %s -d in_file\n"
	  "  %s -c out_file [-nz glob [-nz glob ...]] dir_file1 [dir_file2 ...]\n"
	  "\n"
	  "where:\n"
	  "  -l in_file     list contents of eet file\n"
	  "  -d in_file     unpack eet file\n"
	  "  -c out_file    pack up eet file\n"
	  "  -nz match      don't compress files matching match glob\n"
	  "\n"
	  "example:\n"
	  "  %s -c out.eet -nz \"*.jpg\" things/\n"
	  "  %s -l out.eet\n"
	  "  %s -d out.eet\n",
	  argv[0], argv[0], argv[0],
	  argv[0], argv[0], argv[0]);
   return -1;
}
