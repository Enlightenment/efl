#include "edje.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

Evas_List *img_dirs = NULL;
char      *file_in = NULL;
char      *file_out = NULL;
char      *progname = NULL;

int        line = 0;
Evas_List *stack = NULL;
Evas_List *params = NULL;

Edje_File *edje_file = NULL;

Eet_Data_Descriptor *edd_edje_file;
Eet_Data_Descriptor *edd_edje_image_directory;
Eet_Data_Descriptor *edd_edje_image_directory_entry;

void  data_setup(void);
void  data_write(void);
void  new_object(void);
void  new_statement(void);
int   isdelim(char c);
char *next_token(char *p, char *end, char **new_p, int *delim);
char *stack_id(void);
void  stack_chop_top(void);
void  parse(char *data, off_t size);
void  compile(void);
void  main_help(void);

typedef struct _New_Object_Handler    New_Object_Handler;
typedef struct _New_Statement_Handler New_Statement_Handler;

struct _New_Object_Handler
{
   char *type;
   void (*func)(void);
};

struct _New_Statement_Handler
{
   char *type;
   void (*func)(void);
};

void
ob_images(void)
{
   edje_file->image_dir = calloc(1, sizeof(Edje_Image_Directory));
   if (!edje_file->image_dir)
     {
	fprintf(stderr, "%s: Error. memory allocation of %i bytes failed. %s\n",
		progname, sizeof(Edje_Image_Directory), strerror(errno));
	exit(-1);
     }
}

void
ob_images_image(void)
{
   Edje_Image_Directory_Entry *img;
   
   img = calloc(1, sizeof(Edje_Image_Directory_Entry));
   edje_file->image_dir->entries = evas_list_append(edje_file->image_dir->entries, img);
   img->id = evas_list_count(edje_file->image_dir->entries) - 1;
}

void
st_images_image(void)
{
   Edje_Image_Directory_Entry *img;
   char *str;
   
   img = evas_list_data(evas_list_last(edje_file->image_dir->entries));
   str = evas_list_nth(params, 0);
   if (str)
     {
	img->entry = strdup(str);
	if (!str)
	  {
	     fprintf(stderr, "%s: Error. memory allocation of %i bytes failed. %s\n",
		     progname, strlen(str) + 1, strerror(errno));
	     exit(-1);
	  }
     }
   else
     {
	fprintf(stderr, "%s: Error. %s:%i: no filename for image as arg 1\n",
		progname, file_in, line);
	exit(-1);
     }
   str = evas_list_nth(params, 1);
   if (str)
     {
	if (!strcasecmp(str, "RAW"))
	  {
	     img->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT;
	     img->source_param = 0;
	  }
	else if (!strcasecmp(str, "COMP"))
	  {
	     img->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT;
	     img->source_param = 1;
	  }
	else if (!strcasecmp(str, "LOSSY"))
	  {
	     img->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY;
	     img->source_param = 0;
	  }
	else if (!strcasecmp(str, "USER"))
	  {
	     img->source_type = EDJE_IMAGE_SOURCE_TYPE_EXTERNAL;
	     img->source_param = 0;
	  }
	else
	  {
	     fprintf(stderr, "%s: Error. %s:%i: invalid encoding \"%s\" for image as arg 2\n",
		     progname, file_in, line, str);
	     exit(-1);
	  }
     }
   else
     {
	fprintf(stderr, "%s: Error. %s:%i: no encoding type for image as arg 2\n",
		progname, file_in, line);
	exit(-1);
     }
   if (img->source_type != EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY) return;
   str = evas_list_nth(params, 2);
   if (str)
     {
	img->source_param = atoi(str);
     }
   else
     {
	fprintf(stderr, "%s: Error. %s:%i: no encoding quality for lossy as arg 3\n",
		progname, file_in, line);
	exit(-1);
     }
}

New_Object_Handler object_handlers[] =
{
     {"images", ob_images},
     {"images.image", ob_images_image}
};

New_Statement_Handler statement_handlers[] =
{
     {"images.image", st_images_image}
};

void
data_setup(void)
{
   edd_edje_image_directory_entry = eet_data_descriptor_new("Edje_Image_Directory_Entry",
						      sizeof(Edje_Image_Directory_Entry),
						      evas_list_next,
						      evas_list_append,
						      evas_list_data,
						      evas_hash_foreach,
						      evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_type", source_type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_param", source_param, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "id", id, EET_T_INT);
   edd_edje_image_directory = eet_data_descriptor_new("Edje_Image_Directory", 
						      sizeof(Edje_Image_Directory),
						      evas_list_next,
						      evas_list_append,
						      evas_list_data,
						      evas_hash_foreach,
						      evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_LIST(edd_edje_image_directory, Edje_Image_Directory, "entries", entries, edd_edje_image_directory_entry);
   edd_edje_file = eet_data_descriptor_new("Edje_File", 
					   sizeof(Edje_File),
					   evas_list_next,
					   evas_list_append,
					   evas_list_data,
					   evas_hash_foreach,
					   evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_SUB(edd_edje_file, Edje_File, "image_dir", image_dir, edd_edje_image_directory);
}

void
data_write(void)
{
   Eet_File *ef;
   Evas_List *l;
   
   ef = eet_open(file_out, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	fprintf(stderr, "%s: Error. unable to open %s for writing output\n",
		progname, file_out);
	exit(-1);
     }
   eet_data_write(ef, edd_edje_file, "data", edje_file, 1);
   for (l = edje_file->image_dir->entries; l; l = l->next)
     {
	Edje_Image_Directory_Entry *img;
	
	img = l->data;	
	if (img->source_type != EDJE_IMAGE_SOURCE_TYPE_EXTERNAL)
	  {
	     if (img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT)
	       printf("FIXME: should inline image %s lossless, compression: %i\n", img->entry, img->source_param);
	     else
	       printf("FIXME: should inline image %s lossy, quality: %i%%\n", img->entry, img->source_param);
	  }
     }
   eet_close(ef);
}

void
new_object(void)
{
   char *id;
   int i;
   
   id = stack_id();
//   printf("+++: %s\n", id);
   for (i = 0; i < (sizeof(object_handlers) / sizeof (New_Object_Handler)); i++)
     {
	if (!strcmp(object_handlers[i].type, id))
	  {
	     if (object_handlers[i].func) object_handlers[i].func();
	  }
     }
   free(id);
}

void
new_statement(void)
{
   char *id;
   int i;
   
   id = stack_id();
// {   
//   Evas_List *l;
//   printf("===: %s", id);
//   for (l = params; l; l = l->next) printf(" [%s]", l->data);
//   printf("\n");
// }
   for (i = 0; i < (sizeof(statement_handlers) / sizeof (New_Object_Handler)); i++)
     {
	if (!strcmp(statement_handlers[i].type, id))
	  {
	     if (statement_handlers[i].func) statement_handlers[i].func();
	  }
     }   
   free(id);
}

int
isdelim(char c)
{
   const char *delims = "{},;";
   char *d;
		  
   d = (char *)delims;
   while (*d)
     {
	if (c == *d) return 1;
	d++;
     }
   return 0;
}

char *
next_token(char *p, char *end, char **new_p, int *delim)
{
   char *tok_start = NULL, *tok_end = NULL, *tok = NULL, *sa_start = NULL;
   int in_tok = 0;
   int in_quote = 0;
   int in_comment_ss = 0;
   int in_comment_sa = 0;
   int had_quote = 0;

   *delim = 0;
   if (p >= end) return NULL;
   while (p < end)
     {
	if (*p == '\n')
	  {
	     in_comment_ss = 0;
	     line++;
	  }
	if ((!in_comment_ss) && (!in_comment_sa))
	  {
	     if ((!in_quote) && (*p == '/') && (p < (end - 1)) && (*(p + 1) == '/'))
	       in_comment_ss = 1;
	     if ((!in_quote) && (*p == '/') && (p < (end - 1)) && (*(p + 1) == '*'))
	       {
		  in_comment_sa = 1;
		  sa_start = p;
	       }
	  }
	if ((!in_comment_ss) && (!in_comment_sa))
	  {
	     if (!in_tok)
	       {
		  if (!in_quote)
		    {
		       if (!isspace(*p))
			 {
			    if (*p == '"')
			 {
			    in_quote = 1;
			    had_quote = 1;
			 }
			    in_tok = 1;
			    tok_start = p;
			    if (isdelim(*p)) *delim = 1;
			 }
		    }
	       }
	     else
	       {
		  if (in_quote)
		    {
		       if (((*p) == '"') && (*(p - 1) != '\\'))
		    {
		       in_quote = 0;
		       had_quote = 1;
		    }
		    }
		  else
		    {
		       if (*p == '"')
			 {
			    in_quote = 1;
			    had_quote = 1;
			 }
		       if (
			   (isspace(*p)) ||
			   ((*delim) && (!isdelim(*p))) ||
			   (isdelim(*p))
			   )
			 {
			    in_tok = 0;
			    tok_end = p - 1;
			    goto done;
			 }
		    }
	       }
	  }
	if (in_comment_sa)
	  {
	     if ((*p == '/') && (*(p - 1) == '*') && ((p - sa_start) > 2))
	       in_comment_sa = 0;
	  }
	p++;
     }
   if (!in_tok) return NULL;
   tok_end = p - 1;
   
   done:
   *new_p = p;
   
   tok = malloc(tok_end - tok_start + 2);
   if (!tok)
     {
	fprintf(stderr, "%s: Error. memory allocation of %i bytes failed. %s\n",
		progname, tok_end - tok_start + 2, strerror(errno));
	exit(-1);
     }
   strncpy(tok, tok_start, tok_end - tok_start + 1);
   tok[tok_end - tok_start + 1] = 0;
   
   if (had_quote)
     {
	p = tok;
	
	while (*p)
	  {
	     if (*p == '"')
	       strcpy(p, p + 1);
	     else if ((*p == '\\') && (*(p + 1) == '"'))
	       strcpy(p, p + 1);
	     else if ((*p == '\\') && (*(p + 1) == '\\'))
	       strcpy(p, p + 1);
	     p++;
	  }
     }
   return tok;
}

char *
stack_id(void)
{
   char *id;
   int len;
   Evas_List *l;

   len = 0;
   for (l = stack; l; l = l->next)
     len += strlen(l->data) + 1;
   id = malloc(len);
   if (!id)
     {
	fprintf(stderr, "%s: Error. memory allocation of %i bytes failed. %s\n",
		progname, len, strerror(errno));
	exit(-1);
     }
   id[0] = 0;
   for (l = stack; l; l = l->next)
     {
	strcat(id, l->data);
	if (l->next) strcat(id, ".");
     }
   return id;
}

void
stack_chop_top(void)
{
   char *top;
   
   /* remove top from stack */
   top = evas_list_data(evas_list_last(stack));
   if (top)
     {
	free(top);
	stack = evas_list_remove(stack, top);
     }
   else
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. } marker without matching { marker\n",
		progname, file_in, line);
	exit(-1);
     }
}

void
parse(char *data, off_t size)
{
   char *p, *end, *token;
   int delim = 0;
   int do_params = 0;
   
   p = data;
   end = data + size;
   line = 1;
   while ((token = next_token(p, end, &p, &delim)) != NULL)
     {
	if (delim)
	  {
	     if (!strcmp(token, ",")) do_params = 1;
	     else if (!strcmp(token, "}"))
	       {
		  if (do_params)
		    {
		       fprintf(stderr, "%s: Error. parse error %s:%i. } marker before ; marker\n",
			       progname, file_in, line);
		       exit(-1);
		    }
		  else
		    stack_chop_top();
	       }
	     else if (!strcmp(token, ";"))
	       {
		  
		  do_params = 0;
		  new_statement();
		  /* clear out params */
		  while (params)
		    {
		       free(params->data);
		       params = evas_list_remove(params, params->data);
		    }
		  /* remove top from stack */
		  stack_chop_top();
	       }
	     free(token);
	  }
	else
	  {
	     if (do_params)
	       params = evas_list_append(params, token);
	     else
	       {
		  stack = evas_list_append(stack, token);
		  new_object();
	       }
	  }
     }
}

void
compile(void)
{
   int fd;
   off_t size;
   char *data;
   
   fd = open(file_in, O_RDONLY);
   if (fd < 0)
     {
	fprintf(stderr, "%s: Error. cannot open file %s for input. %s\n",
		progname, file_in, strerror(errno));
	exit(-1);
     }
	
   size = lseek(fd, 0, SEEK_END);
   lseek(fd, 0, SEEK_SET);
   data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
   if (data)
     {
	parse(data, size);
	munmap(data, size);
     }
   else
     {
	fprintf(stderr, "%s: Error. cannot mmap file %s for input. %s\n",
		progname, file_in, strerror(errno));
	exit(-1);
     }
   close(fd);
}

void
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
