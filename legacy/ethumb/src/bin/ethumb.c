/**
 * @file
 *
 * Copyright (C) 2009 by ProFUSION embedded systems
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the  GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 *
 * @author Rafael Antognolli <antognolli@profusion.mobi>
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <Ethumb.h>
#include <Eina.h>

static const char short_options[] = "s:f:a:d:c:t:h";

static const struct option long_options[] = {
       {"size", 1, NULL, 's'},
       {"format", 1, NULL, 'f'},
       {"aspect", 1, NULL, 'a'},
       {"directory", 1, NULL, 'd'},
       {"category", 1, NULL, 'c'},
       {"theme", 1, NULL, 't'},
       {"help", 0, NULL, 'h'},
       {NULL, 0, 0, 0}
};

static const char *help_texts[] = {
     "size expected",
     "file format to save",
     "original image aspect ratio",
     "directory to save thumbnails",
     "thumbnails category",
     "path to theme file, group and swallow part",
     "this message",
     NULL
};

static void
show_help(const char *prg_name)
{
   const struct option *lo;
   const char **help;
   int largest;

   fprintf(stderr,
	   "\nUsage:\n"
	   "\t%s [options] <image> [thumbnail path]\n"
	   "where options are:\n",
	   prg_name);

   lo = long_options;

   largest = 0;
   for (; lo->name != NULL; lo++)
     {
	int len = strlen(lo->name) + 9;

	if (lo->has_arg)
	  len += sizeof("=ARG") - 1;

	if (largest < len)
	  largest = len;
     }

   lo = long_options;
   help = help_texts;
   for (; lo->name != NULL; lo++, help++)
     {
	int len, i;

	fprintf(stderr, "\t-%c, --%s", lo->val, lo->name);
	len = strlen(lo->name) + 7;
	if (lo->has_arg)
	  {
	     fputs("=ARG", stderr);
	     len += sizeof("=ARG") - 1;
	  }

	for (i = len; i < largest; i++)
	  fputc(' ', stderr);

	fputs("   ", stderr);
	fputs(*help, stderr);
	fputc('\n', stderr);
     }
   fputc('\n', stderr);
}

static int
parse_size(const char *text, int *w, int *h)
{
   const char *sep;
   char *p;

   sep = strchr(text, 'x');
   if (!sep)
     {
	fprintf(stderr,
		"ERROR: invalid size format, must be WIDTHxHEIGHT, got '%s'\n",
	      text);
	return 0;
     }
   sep++;

   *w = strtol(text, &p, 10);
   if (text == p)
     {
	fprintf(stderr, "ERROR: could not parse size width '%s'\n", text);
	return 0;
     }

   *h = strtol(sep, &p, 10);
   if (sep == p)
     {
	fprintf(stderr, "ERROR: could not parse size height '%s'\n", text);
	return 0;
     }

   return 1;
}

static int
parse_format(const char *text, int *f)
{
   if (!strncmp(text, "png", 3))
     *f = ETHUMB_THUMB_FDO;
   else if (!strncmp(text, "jpg", 3))
     *f = ETHUMB_THUMB_JPEG;
   else
     {
	fprintf(stderr, "ERROR: invalid format specified: %s\n", text);
	fprintf(stderr, "valid options: \"png\" and \"jpg\"\n");
	return 0;
     }

   return 1;
}

static int
parse_aspect(const char *text, int *a)
{
   if (!strncmp(text, "keep", sizeof("keep")))
     *a = ETHUMB_THUMB_KEEP_ASPECT;
   else if (!strncmp(text, "ignore", sizeof("ignore")))
     *a = ETHUMB_THUMB_IGNORE_ASPECT;
   else if (!strncmp(text, "crop", sizeof("crop")))
     *a = ETHUMB_THUMB_CROP;
   else
     {
	fprintf(stderr, "ERROR: invalid aspect option: %s\n", text);
	fprintf(stderr, "valid options: \"keep\", \"ignore\" and \"crop\"\n");
	return 0;
     }

   return 1;
}

static int
parse_theme(const char *text, const char **file, const char **group, const char **swallow)
{
   char *sep;
   const char *tfile, *tgroup, *tswallow;

   tfile = NULL;
   tgroup = NULL;
   tswallow = NULL;

   sep = strchr(text, ':');
   if (!sep)
     {
	fprintf(stderr, "ERROR: invalid theme: %s\n"
			"format: '<filename>:<group>:<swallow part>'\n",
			text);
	goto error;
     }
   *sep = '\0';
   tfile = eina_stringshare_add(text);

   sep++;
   text = sep;
   sep = strchr(text, ':');
   if (!sep)
     {
	fprintf(stderr, "ERROR: invalid theme: %s\n"
			"format: '<filename>:<group>:<swallow part>'\n",
			text);
	goto error;
     }
   *sep = '\0';
   tgroup = eina_stringshare_add(text);

   sep++;
   tswallow = eina_stringshare_add(sep);

   *file = tfile;
   *group = tgroup;
   *swallow = tswallow;

   return 1;

 error:
   eina_stringshare_del(tfile);

   return 0;
}

static int
parse_options(Ethumb *e, int argc, char *argv[])
{
   int opt_index;
   const char *size, *format, *aspect, *directory;
   const char *category, *theme;
   int r;

   size = NULL;
   format = NULL;
   aspect = NULL;
   directory = NULL;
   category = NULL;
   theme = NULL;

   optind = 0;
   opterr = 0;
   opt_index = 0;
   while (1)
     {
	int c;

	c = getopt_long(argc, argv, short_options, long_options, &opt_index);
	if (c == -1)
	  break;

	switch (c)
	  {
	   case 's':
	      size = optarg;
	      break;
	   case 'f':
	      format = optarg;
	      break;
	   case 'a':
	      aspect = optarg;
	      break;
	   case 'd':
	      directory = optarg;
	      break;
	   case 'c':
	      category = optarg;
	      break;
	   case 't':
	      theme = optarg;
	      break;
	   default:
	      break;
	  }
     }

   if (size)
     {
	int w, h;
	r = parse_size(size, &w, &h);
	if (r)
	  ethumb_thumb_size_set(e, w, h);
	else
	  return 0;
     }

   if (format)
     {
	int f;
	r = parse_format(format, &f);
	if (r)
	  ethumb_thumb_format_set(e, f);
	else
	  return 0;
     }

   if (aspect)
     {
	int a;
	r = parse_aspect(aspect, &a);
	if (r)
	  ethumb_thumb_aspect_set(e, a);
	else
	  return 0;
     }

   if (directory)
     ethumb_thumb_dir_path_set(e, directory);

   if (category)
     ethumb_thumb_category_set(e, category);

   if (theme)
     {
	const char *file, *group, *swallow;
	r = parse_theme(theme, &file, &group, &swallow);
	if (r)
	  {
	     ethumb_frame_set(e, file, group, swallow);
	     eina_stringshare_del(file);
	     eina_stringshare_del(group);
	     eina_stringshare_del(swallow);
	  }
	else
	  return 0;
     }

   return 1;
}

int
main(int argc, char *argv[])
{
   Ethumb *e;
   Ethumb_File *ef;
   int r;

   ef = NULL;

   ethumb_init();

   e = ethumb_new();

   r = parse_options(e, argc, argv);

   if (r && optind < argc)
     ef = ethumb_file_new(e, argv[optind++]);
   if (ef && optind < argc)
     ethumb_file_thumb_path_set(ef, argv[optind++]);

   if (ef)
     ethumb_file_generate(ef);
   else
     show_help(argv[0]);

   ethumb_file_free(ef);
   ethumb_free(e);

   ethumb_shutdown();

   return 0;
}
