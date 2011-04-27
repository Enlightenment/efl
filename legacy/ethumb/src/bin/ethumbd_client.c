/**
 * @file
 *
 * Copyright (C) 2009 by ProFUSION embedded systems
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 *
 * @author Rafael Antognolli <antognolli@profusion.mobi>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <Ethumb_Client.h>
#include <Eina.h>
#include <Ecore_Getopt.h>
#include <Ecore.h>

const char *aspect_opt[] = { "keep", "ignore", "crop", NULL };
const char *format_opt[] = { "png", "jpg", "eet", NULL };
struct frame
{
   const char *file;
   const char *group;
   const char *swallow;
};

struct options
{
   Eina_Rectangle geometry;
   unsigned int format, aspect;
   char *format_str;
   char *aspect_str;
   char *directory;
   char *category;
   struct frame frame;
   char *src_path;
   char *src_key;
   const char *thumb_path;
   const char *thumb_key;
   double video_time;
   int page;
};

static unsigned char
_ethumb_getopt_callback_frame_parse(const Ecore_Getopt *parser __UNUSED__, const Ecore_Getopt_Desc *desc __UNUSED__, const char *str, void *data __UNUSED__, Ecore_Getopt_Value *storage)
{
   struct frame *f = (struct frame *)storage->ptrp;
   const char *tfile, *tgroup, *tswallow, *base, *sep;

   base = str;
   sep = strchr(base, ':');
   if (!sep)
	goto error;
   tfile = eina_stringshare_add_length(base, sep - base);
   base = sep + 1;

   sep = strchr(base, ':');
   if (!sep)
     {
	eina_stringshare_del(tfile);
	goto error;
     }
   tgroup = eina_stringshare_add_length(base, sep - base);
   base = sep + 1;
   if (base[0] == '\0')
     {
	eina_stringshare_del(tfile);
	eina_stringshare_del(tgroup);
	goto error;
     }
   tswallow = eina_stringshare_add(base);

   f->file = tfile;
   f->group = tgroup;
   f->swallow = tswallow;
   return 1;

 error:
   fprintf(stderr,
	   "ERROR: invalid theme, not in format "
	   "'file:group:swallow_part': '%s'\n",
	   str);
   return 0;
}

const Ecore_Getopt optdesc = {
  "ethumbd_client",
  NULL,
  PACKAGE_VERSION,
  "(C) 2009 - ProFUSION embedded systems",
  "LGPL v3 - GNU Lesser General Public License",
  "Thumbnails generator client using DBus and ethumbd.\n"
  "\n"
  "This program uses ethumbd server to create thumbnails from pictures. "
  "It's an example of use and a test for ethumbd.\n",
  0,
  {
     ECORE_GETOPT_CALLBACK_ARGS
     ('s', "size", "thumbnail size expected.",
      "WxH", ecore_getopt_callback_size_parse, NULL),
     ECORE_GETOPT_CHOICE
     ('f', "format", "file format to save.", format_opt),
     ECORE_GETOPT_CHOICE
     ('a', "aspect", "original image aspect ratio.", aspect_opt),
     ECORE_GETOPT_STORE_STR
     ('d', "directory", "directory to save thumbnails."),
     ECORE_GETOPT_STORE_STR
     ('c', "category", "thumbnails category."),
     ECORE_GETOPT_CALLBACK_ARGS
     ('t', "theme", "path to theme file, group and swallow part.",
      "file:group:swallow_part", _ethumb_getopt_callback_frame_parse, NULL),
     ECORE_GETOPT_STORE_STR
     ('k', "key", "key inside eet file to read image from."),
     ECORE_GETOPT_STORE_DOUBLE
     ('v', "video_time", "time of video frame to use as thumbnail."),
     ECORE_GETOPT_STORE_INT
     ('p', "document_page", "document page to use as thumbnail."),
     ECORE_GETOPT_LICENSE('L', "license"),
     ECORE_GETOPT_COPYRIGHT('C', "copyright"),
     ECORE_GETOPT_VERSION('V', "version"),
     ECORE_GETOPT_HELP('h', "help"),
     ECORE_GETOPT_SENTINEL
  }
};

static void
_thumb_report(const char *mode, const char *src_path, const char *src_key, const char *thumb_path, const char *thumb_key)
{
   printf("%s '%s' '%s' => '%s' '%s'\n",
	  mode,
	  src_path, src_key ? src_key : "",
	  thumb_path, thumb_key ? thumb_key : "");
}

static void
_finished_thumb(void *data __UNUSED__, Ethumb_Client *client __UNUSED__, int id __UNUSED__, const char *src_path, const char *src_key, const char *thumb_path, const char *thumb_key, Eina_Bool success)
{
   const char *mode = success ? "GENERATED" : "FAILED";
   _thumb_report(mode, src_path, src_key, thumb_path, thumb_key);
   ecore_main_loop_quit();
}

static void
_exists(Ethumb_Client *c, __UNUSED__ Ethumb_Exists *thread, Eina_Bool exists, void *data)
{
   struct options *opts = data;
   const char *thumb_path, *thumb_key;
   long id;

   if (exists)
     {
        ethumb_client_thumb_path_get(c, &thumb_path, &thumb_key);
        _thumb_report
          ("EXISTS", opts->src_path, opts->src_key, thumb_path, thumb_key);
        ecore_main_loop_quit();
        return;
     }

   id = ethumb_client_generate(c, _finished_thumb, NULL, NULL);
   if (id < 0)
     {
	fputs("ERROR: could not request thumbnail to be generated.\n", stderr);
	ecore_main_loop_quit();
	return;
     }
   printf("request id=%ld, file='%s', key='%s'\n",
	  id, opts->src_path, opts->src_key ? opts->src_key : "");

}

static void
_connected(void *data, Ethumb_Client *c, Eina_Bool success)
{
   struct options *opts = data;

   if (!success)
     {
	fputs("ERROR: could not connect to DBus server.\n", stderr);
	ecore_main_loop_quit();
	return;
     }

   fputs("connected to DBus server, setup parameters...\n", stdout);

   ethumb_client_format_set(c, opts->format);
   ethumb_client_aspect_set(c, opts->aspect);

   if (opts->directory) ethumb_client_dir_path_set(c, opts->directory);
   if (opts->category) ethumb_client_category_set(c, opts->category);
   if (opts->geometry.w > 0 && opts->geometry.h > 0)
     ethumb_client_size_set(c, opts->geometry.w, opts->geometry.h);
   if (opts->frame.file)
     ethumb_client_frame_set
       (c, opts->frame.file, opts->frame.group, opts->frame.swallow);
   if (opts->video_time > 0)
     ethumb_client_video_time_set(c, opts->video_time);
   if (opts->page > 0)
     ethumb_client_document_page_set(c, opts->page);

   if (!ethumb_client_file_set(c, opts->src_path, opts->src_key))
     {
	fprintf(stderr, "ERROR: could not set file '%s', key '%s'\n",
		opts->src_path, opts->src_key ? opts->src_key : "");
	ecore_main_loop_quit();
	return;
     }

   ethumb_client_thumb_path_set(c, opts->thumb_path, opts->thumb_key);
   ethumb_client_thumb_exists(c, _exists, opts);
}

int
main(int argc, char *argv[])
{
   Ethumb_Client *c;
   Eina_Bool quit_option = 0;
   const char *format_str = NULL, *aspect_str;
   struct options opts = {
     {-1, -1, -1, -1},
     0, 0,
     NULL, NULL, NULL, NULL,
     {NULL, NULL, NULL},
     NULL, NULL, NULL, NULL,
     0.0,
     0
   };
   int arg_index;
   int i, ret = 0;

   ethumb_client_init();
   ecore_init();

   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_PTR_CAST(opts.geometry),
     ECORE_GETOPT_VALUE_PTR_CAST(format_str),
     ECORE_GETOPT_VALUE_PTR_CAST(aspect_str),
     ECORE_GETOPT_VALUE_STR(opts.directory),
     ECORE_GETOPT_VALUE_STR(opts.category),
     ECORE_GETOPT_VALUE_PTR_CAST(opts.frame),
     ECORE_GETOPT_VALUE_STR(opts.src_key),
     ECORE_GETOPT_VALUE_DOUBLE(opts.video_time),
     ECORE_GETOPT_VALUE_INT(opts.page),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_NONE
   };

   arg_index = ecore_getopt_parse(&optdesc, values, argc, argv);
   if ((arg_index < 0) || (arg_index == argc))
     {
	if (arg_index < 0)
	  fprintf(stderr, "Could not parse arguments.\n");
	else
	  fprintf(stderr, "Missing source file to thumbnail.\n");

	ret = 1;
	goto end;
     }

   if (quit_option)
     {
	ret = 0;
	goto end;
     }

   for (i = 0; i < 3; i++)
     if (format_opt[i] == format_str)
       {
	  opts.format = i;
	  break;
       }

   for (i = 0; i < 3; i++)
     if (aspect_opt[i] == aspect_str)
       {
	  opts.aspect = i;
	  break;
       }

   opts.src_path = argv[arg_index++];
   if (arg_index < argc)
     {
	opts.thumb_path = argv[arg_index++];
	if (arg_index < argc)
	  opts.thumb_key = argv[arg_index];
     }

   c = ethumb_client_connect(_connected, &opts, NULL);
   if (!c)
     {
	fputs("ERROR: could not connect to server.\n", stderr);
	ret = 2;
	goto end;
     }

   ecore_main_loop_begin();
   ethumb_client_disconnect(c);

 end:
   if (opts.frame.file)
     {
	eina_stringshare_del(opts.frame.file);
	eina_stringshare_del(opts.frame.group);
	eina_stringshare_del(opts.frame.swallow);
     }
   ecore_shutdown();
   ethumb_client_shutdown();

   return ret;
}
