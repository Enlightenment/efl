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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <Eina.h>
#include <eina_safety_checks.h>
#include "Ethumb.h"
#include "ethumb_private.h"
#include "Ethumb_Plugin.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <dlfcn.h>
#include "md5.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Edje.h>

#define DBG(...) EINA_ERROR_PDBG(__VA_ARGS__)
#define INF(...) EINA_ERROR_PINFO(__VA_ARGS__)
#define WRN(...) EINA_ERROR_PWARN(__VA_ARGS__)
#define ERR(...) EINA_ERROR_PERR(__VA_ARGS__)

static int initcount = 0;
static const char *_home_thumb_dir = NULL;
static const char *_thumb_category_normal = NULL;
static const char *_thumb_category_large = NULL;

static const int THUMB_SIZE_NORMAL = 128;
static const int THUMB_SIZE_LARGE = 256;

static Eina_Hash *_plugins_ext = NULL;
static Eina_Array *_plugins = NULL;

static Eina_Bool
_ethumb_plugin_list_cb(Eina_Module *m, void *data)
{
   const char *file;
   const char **ext;
   Ethumb_Plugin *plugin;
   Ethumb_Plugin *(*plugin_get)(void);

   if (!eina_module_load(m))
     return EINA_FALSE;

   plugin_get = eina_module_symbol_get(m, "ethumb_plugin_get");
   if (!plugin_get)
     {
	eina_module_unload(m);
	return EINA_FALSE;
     }

   file = eina_module_file_get(m);

   plugin = plugin_get();
   if (!plugin)
     {
	ERR("plugin \"%s\" failed to init.\n", file);
	eina_module_unload(m);
	return EINA_FALSE;
     }

   for (ext = plugin->extensions; *ext; ext++)
     eina_hash_add(_plugins_ext, *ext, plugin);

   return EINA_TRUE;
}

static void
_ethumb_plugins_load(void)
{
   _plugins_ext = eina_hash_string_small_new(NULL);
   EINA_SAFETY_ON_NULL_RETURN(_plugins_ext);

   _plugins = eina_module_list_get(_plugins, PLUGINSDIR, 1,
				   &_ethumb_plugin_list_cb, NULL);
}

static void
_ethumb_plugins_unload(void)
{
   eina_hash_free(_plugins_ext);
   _plugins_ext = NULL;
   eina_module_list_unload(_plugins);
   eina_module_list_delete(_plugins);
   eina_array_free(_plugins);
   _plugins = NULL;
}

EAPI int
ethumb_init(void)
{
   const char *home;
   char buf[PATH_MAX];

   if (initcount)
     return ++initcount;

   eina_stringshare_init();
   eina_list_init();
   eina_hash_init();
   eina_module_init();
   evas_init();
   ecore_init();
   ecore_evas_init();
   edje_init();

   home = getenv("HOME");
   snprintf(buf, sizeof(buf), "%s/.thumbnails", home);

   _home_thumb_dir = eina_stringshare_add(buf);
   _thumb_category_normal = eina_stringshare_add("normal");
   _thumb_category_large = eina_stringshare_add("large");

   _ethumb_plugins_load();
   return ++initcount;
}

EAPI int
ethumb_shutdown(void)
{
   initcount--;
   if (initcount == 0)
     {
	_ethumb_plugins_unload();
	eina_stringshare_del(_home_thumb_dir);
	eina_stringshare_del(_thumb_category_normal);
	eina_stringshare_del(_thumb_category_large);
	eina_stringshare_shutdown();
	eina_list_shutdown();
	eina_hash_shutdown();
	eina_module_shutdown();
	evas_shutdown();
	ecore_shutdown();
	ecore_evas_shutdown();
	edje_shutdown();
     }

   return initcount;
}

EAPI Ethumb *
ethumb_new(void)
{
   Ethumb *ethumb;
   Ecore_Evas *ee, *sub_ee;
   Evas *e, *sub_e;
   Evas_Object *o, *img;

   ethumb = calloc(1, sizeof(Ethumb));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ethumb, NULL);

   ethumb->tw = THUMB_SIZE_NORMAL;
   ethumb->th = THUMB_SIZE_NORMAL;
   ethumb->crop_x = 0.5;
   ethumb->crop_y = 0.5;
   ethumb->quality = 80;
   ethumb->compress = 9;
   ethumb->video.start = 0.1;
   ethumb->video.time = 3;
   ethumb->video.interval = 0.05;
   ethumb->video.ntimes = 3;
   ethumb->video.fps = 10;

   ee = ecore_evas_buffer_new(1, 1);
   e = ecore_evas_get(ee);
   if (!e)
     {
	ERR("could not create ecore evas buffer\n");
	return NULL;
     }

   evas_image_cache_set(e, 0);
   evas_font_cache_set(e, 0);

   o = ecore_evas_object_image_new(ee);
   if (!o)
     {
	ERR("could not create sub ecore evas buffer\n");
	ecore_evas_free(ee);
	free(ethumb);
	return NULL;
     }

   sub_ee = evas_object_data_get(o, "Ecore_Evas");
   sub_e = ecore_evas_get(sub_ee);

   evas_image_cache_set(sub_e, 0);
   evas_font_cache_set(sub_e, 0);

   img = evas_object_image_add(sub_e);
   if (!img)
     {
	ERR("could not create source objects.\n");
	ecore_evas_free(ee);
	free(ethumb);
	return NULL;
     }

   ethumb->ee = ee;
   ethumb->e = e;
   ethumb->sub_ee = sub_ee;
   ethumb->sub_e = sub_e;
   ethumb->o = o;
   ethumb->img = img;

   return ethumb;
}

static void
_ethumb_frame_free(Ethumb_Frame *frame)
{
   Evas_Object *o;

   if (!frame)
     return;

   if (frame->swallow && frame->edje)
     {
     o = edje_object_part_swallow_get(frame->edje, frame->swallow);
     if (o)
       edje_object_part_unswallow(frame->edje, o);
     }
   eina_stringshare_del(frame->file);
   eina_stringshare_del(frame->group);
   eina_stringshare_del(frame->swallow);

   if (frame->edje)
     evas_object_del(frame->edje);

   free(frame);
}

EAPI void
ethumb_free(Ethumb *ethumb)
{
   EINA_SAFETY_ON_NULL_RETURN(ethumb);

   if (ethumb->frame)
     _ethumb_frame_free(ethumb->frame);
   ethumb_file_free(ethumb);
   ecore_evas_free(ethumb->ee);
   eina_stringshare_del(ethumb->thumb_dir);
   eina_stringshare_del(ethumb->category);
   if (ethumb->finished_idler)
     ecore_idler_del(ethumb->finished_idler);
   free(ethumb);
}

EAPI void
ethumb_thumb_fdo_set(Ethumb *e, Ethumb_Thumb_FDO_Size s)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_SAFETY_ON_FALSE_RETURN(s == ETHUMB_THUMB_NORMAL ||
			       s == ETHUMB_THUMB_LARGE);

   if (s == ETHUMB_THUMB_NORMAL)
     {
	e->tw = THUMB_SIZE_NORMAL;
	e->th = THUMB_SIZE_NORMAL;
     }
   else
     {
	e->tw = THUMB_SIZE_LARGE;
	e->th = THUMB_SIZE_LARGE;
     }

   e->format = ETHUMB_THUMB_FDO;
   e->aspect = ETHUMB_THUMB_KEEP_ASPECT;
   _ethumb_frame_free(e->frame);
   e->frame = NULL;
   eina_stringshare_del(e->thumb_dir);
   eina_stringshare_del(e->category);
   e->thumb_dir = NULL;
   e->category = NULL;
}

EAPI void
ethumb_thumb_size_set(Ethumb *e, int tw, int th)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_SAFETY_ON_FALSE_RETURN(tw > 0);
   EINA_SAFETY_ON_FALSE_RETURN(th > 0);

   e->tw = tw;
   e->th = th;
}

EAPI void
ethumb_thumb_size_get(const Ethumb *e, int *tw, int *th)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   if (tw) *tw = e->tw;
   if (th) *th = e->th;
}

EAPI void
ethumb_thumb_format_set(Ethumb *e, Ethumb_Thumb_Format f)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_SAFETY_ON_FALSE_RETURN(f == ETHUMB_THUMB_FDO ||
			       f == ETHUMB_THUMB_JPEG ||
			       f == ETHUMB_THUMB_EET);

   e->format = f;
}

EAPI Ethumb_Thumb_Format
ethumb_thumb_format_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);
   return e->format;
}

EAPI void
ethumb_thumb_aspect_set(Ethumb *e, Ethumb_Thumb_Aspect a)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_SAFETY_ON_FALSE_RETURN(a == ETHUMB_THUMB_KEEP_ASPECT ||
			       a == ETHUMB_THUMB_IGNORE_ASPECT ||
			       a == ETHUMB_THUMB_CROP);

   e->aspect = a;
}

EAPI Ethumb_Thumb_Aspect
ethumb_thumb_aspect_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);
   return e->aspect;
}

EAPI void
ethumb_thumb_crop_align_set(Ethumb *e, float x, float y)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->crop_x = x;
   e->crop_y = y;
}

EAPI void
ethumb_thumb_crop_align_get(const Ethumb *e, float *x, float *y)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   if (x) *x = e->crop_x;
   if (y) *y = e->crop_y;
}

EAPI void
ethumb_thumb_quality_set(Ethumb *e, int quality)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->quality = quality;
}

EAPI int
ethumb_thumb_quality_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);
   return e->quality;
}

EAPI void
ethumb_thumb_compress_set(Ethumb *e, int compress)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->compress = compress;
}

EAPI int
ethumb_thumb_compress_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);
   return e->compress;
}

EAPI Eina_Bool
ethumb_frame_set(Ethumb *e, const char *theme_file, const char *group, const char *swallow)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   Ethumb_Frame *frame;
   frame = e->frame;

   if (frame)
     {
	edje_object_part_unswallow(frame->edje, e->img);
	if (!theme_file)
	  _ethumb_frame_free(frame);
     }

   if (!theme_file)
     {
	e->frame = NULL;
	return EINA_TRUE;
     }

   if (!frame)
     {
	frame = calloc(1, sizeof(Ethumb_Frame));
	if (!frame)
	  {
	     ERR("could not allocate Ethumb_Frame structure.\n");
	     return EINA_FALSE;
	  }

	frame->edje = edje_object_add(e->sub_e);
	if (!frame->edje)
	  {
	     ERR("could not create edje frame object.\n");
	     _ethumb_frame_free(frame);
	     e->frame = NULL;
	     return EINA_FALSE;
	  }
     }

   if (!edje_object_file_set(frame->edje, theme_file, group))
     {
	ERR("could not load frame theme.\n");
	_ethumb_frame_free(frame);
	e->frame = NULL;
	return EINA_FALSE;
     }

   edje_object_part_swallow(frame->edje, swallow, e->img);
   if (!edje_object_part_swallow_get(frame->edje, swallow))
     {
	ERR("could not swallow image to edje frame.\n");
	_ethumb_frame_free(frame);
	e->frame = NULL;
	return EINA_FALSE;
     }

   eina_stringshare_replace(&frame->file, theme_file);
   eina_stringshare_replace(&frame->group, group);
   eina_stringshare_replace(&frame->swallow, swallow);

   e->frame = frame;

   return EINA_TRUE;
}

EAPI void
ethumb_frame_get(const Ethumb *e, const char **theme_file, const char **group, const char **swallow)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   if (e->frame)
     {
	if (theme_file) *theme_file = e->frame->file;
	if (group) *group = e->frame->group;
	if (swallow) *swallow = e->frame->swallow;
     }
   else
     {
	if (theme_file) *theme_file = NULL;
	if (group) *group = NULL;
	if (swallow) *swallow = NULL;
     }
}

static const char *
_ethumb_build_absolute_path(const char *path, char buf[])
{
   char *p;
   int len;

   if (!path)
     return NULL;

   p = buf;

   if (path[0] == '/')
     strcpy(p, path);
   else if (path[0] == '~')
     {
	strcpy(p, getenv("HOME"));
	len = strlen(p);
	p += len;
	p[0] = '/';
	p++;
	strcpy(p, path + 2);
     }
   else
     {
	getcwd(p, PATH_MAX);
	len = strlen(p);
	p += len;
	p[0] = '/';
	p++;
	strcpy(p, path);
     }

   return buf;
}

EAPI void
ethumb_thumb_dir_path_set(Ethumb *e, const char *path)
{
   char buf[PATH_MAX];
   EINA_SAFETY_ON_NULL_RETURN(e);

   path = _ethumb_build_absolute_path(path, buf);
   eina_stringshare_replace(&e->thumb_dir, path);
}

EAPI const char *
ethumb_thumb_dir_path_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);

   return e->thumb_dir;
}

EAPI void
ethumb_thumb_category_set(Ethumb *e, const char *category)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   eina_stringshare_replace(&e->category, category);
}

EAPI const char *
ethumb_thumb_category_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);

   return e->category;
}

EAPI void
ethumb_video_start_set(Ethumb *e, float start)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->video.start = start;
}

EAPI float
ethumb_video_start_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   return e->video.start;
}

EAPI void
ethumb_video_time_set(Ethumb *e, float time)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->video.time = time;
}

EAPI float
ethumb_video_time_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   return e->video.time;
}

EAPI void
ethumb_video_interval_set(Ethumb *e, float interval)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->video.interval = interval;
}

EAPI float
ethumb_video_interval_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   return e->video.interval;
}

EAPI void
ethumb_video_ntimes_set(Ethumb *e, int ntimes)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->video.ntimes = ntimes;
}

EAPI int
ethumb_video_ntimes_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   return e->video.ntimes;
}

EAPI void
ethumb_video_fps_set(Ethumb *e, int fps)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->video.fps = fps;
}

EAPI int
ethumb_video_fps_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   return e->video.fps;
}

EAPI void
ethumb_document_page_set(Ethumb *e, int page)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->document.page = page;
}

EAPI int
ethumb_document_page_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   return e->document.page;
}

EAPI Eina_Bool
ethumb_file_set(Ethumb *e, const char *path, const char *key)
{
   char buf[PATH_MAX];
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   if (path && access(path, R_OK))
     {
	ERR("couldn't access file \"%s\"\n", path);
	return EINA_FALSE;
     }

   path = _ethumb_build_absolute_path(path, buf);
   eina_stringshare_replace(&e->src_path, path);
   eina_stringshare_replace(&e->src_key, key);
   eina_stringshare_replace(&e->thumb_path, NULL);
   eina_stringshare_replace(&e->thumb_key, NULL);

   return EINA_TRUE;
}

EAPI void
ethumb_file_get(const Ethumb *e, const char **path, const char **key)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   if (path) *path = e->src_path;
   if (key) *key = e->src_key;
}

static const char ACCEPTABLE_URI_CHARS[96] = {
     /*      !    "    #    $    %    &    '    (    )    *    +    ,    -    .    / */ 
     0x00,0x3F,0x20,0x20,0x28,0x00,0x2C,0x3F,0x3F,0x3F,0x3F,0x2A,0x28,0x3F,0x3F,0x1C,
     /* 0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ? */
     0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x38,0x20,0x20,0x2C,0x20,0x20,
     /* @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O */
     0x38,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
     /* P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^    _ */
     0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x20,0x3F,
     /* `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o */
     0x20,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
     /* p    q    r    s    t    u    v    w    x    y    z    {    |    }    ~  DEL */
     0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x3F,0x20
};

static const char *
_ethumb_generate_hash(const char *file)
{
  int n;
  MD5_CTX ctx;
  char md5out[(2 * MD5_HASHBYTES) + 1];
  unsigned char hash[MD5_HASHBYTES];
  static const char hex[] = "0123456789abcdef";

  char *uri;
  char *t;
  const char *c;

#define _check_uri_char(c) \
  ((c) >= 32 && (c) < 128 && (ACCEPTABLE_URI_CHARS[(c) - 32] & 0x08))

  EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

  uri = alloca(3 * strlen(file) + 9);
  strncpy(uri, "file://", sizeof(uri));
  t = uri + 7;

  for (c = file; *c != '\0'; c++)
    {
       if (!_check_uri_char(*c))
	 {
	    *t++ = '%';
	    *t++ = hex[*c >> 4];
	    *t++ = hex[*c & 15];
	 }
       else
	 *t++ = *c;
    }
  *t = '\0';

#undef _check_uri_char

  MD5Init (&ctx);
  MD5Update (&ctx, (unsigned char const*)uri, (unsigned)strlen (uri));
  MD5Final (hash, &ctx);

  for (n = 0; n < MD5_HASHBYTES; n++)
    {
      md5out[2 * n] = hex[hash[n] >> 4];
      md5out[2 * n + 1] = hex[hash[n] & 0x0f];
    }
  md5out[2 * n] = '\0';
  return eina_stringshare_add(md5out);
}

static int
_ethumb_file_check_fdo(Ethumb *e)
{
   if (!((e->tw == THUMB_SIZE_NORMAL && e->th == THUMB_SIZE_NORMAL) ||
       (e->tw == THUMB_SIZE_LARGE && e->th == THUMB_SIZE_LARGE)))
     return 0;

   if (e->format != ETHUMB_THUMB_FDO)
     return 0;

   if (e->aspect != ETHUMB_THUMB_KEEP_ASPECT)
     return 0;

   if (e->frame)
     return 0;

   return 1;
}

static const char *
_ethumb_file_generate_custom_category(Ethumb *e)
{
   char buf[PATH_MAX];
   const char *aspect, *format;
   const char *frame;

   if (e->aspect == ETHUMB_THUMB_KEEP_ASPECT)
     aspect = "keep_aspect";
   else if (e->aspect == ETHUMB_THUMB_IGNORE_ASPECT)
     aspect = "ignore_aspect";
   else
     aspect = "crop";

   if (e->format == ETHUMB_THUMB_FDO)
     format = "png";
   else if (e->format == ETHUMB_THUMB_JPEG)
     format = "jpg";
   else
     format = "eet";

   if (e->frame)
     frame = "-framed";
   else
     frame = "";

   snprintf(buf, sizeof(buf), "%dx%d-%s%s-%s",
	    e->tw, e->th, aspect, frame, format);

   return eina_stringshare_add(buf);
}

static void
_ethumb_file_generate_path(Ethumb *e)
{
   char buf[PATH_MAX];
   char *fullname;
   const char *hash;
   const char *thumb_dir, *category;
   const char *ext;
   int fdo_format;


   fdo_format = _ethumb_file_check_fdo(e);

   if (e->thumb_dir)
     thumb_dir = eina_stringshare_ref(e->thumb_dir);
   else
     thumb_dir = eina_stringshare_ref(_home_thumb_dir);

   if (e->category)
     category = eina_stringshare_ref(e->category);
   else if (!fdo_format)
     category = _ethumb_file_generate_custom_category(e);
   else
     {
	if (e->tw == THUMB_SIZE_NORMAL)
	  category = eina_stringshare_ref(_thumb_category_normal);
	else if (e->tw == THUMB_SIZE_LARGE)
	  category = eina_stringshare_ref(_thumb_category_large);
     }

   if (e->format == ETHUMB_THUMB_FDO)
     ext = "png";
   else if (e->format == ETHUMB_THUMB_JPEG)
     ext = "jpg";
   else
     ext = "eet";


   fullname = ecore_file_realpath(e->src_path);
   hash = _ethumb_generate_hash(fullname);
   snprintf(buf, sizeof(buf), "%s/%s/%s.%s", thumb_dir, category, hash, ext);
   free(fullname);
   eina_stringshare_replace(&e->thumb_path, buf);
   if (e->format == ETHUMB_THUMB_EET)
     eina_stringshare_replace(&e->thumb_key, "thumbnail");
   else
     {
	eina_stringshare_del(e->thumb_key);
	e->thumb_key = NULL;
     }

   eina_stringshare_del(thumb_dir);
   eina_stringshare_del(category);
   eina_stringshare_del(hash);
}

EAPI void
ethumb_file_free(Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   eina_stringshare_replace(&e->src_path, NULL);
   eina_stringshare_replace(&e->src_key, NULL);
   eina_stringshare_replace(&e->thumb_path, NULL);
   eina_stringshare_replace(&e->thumb_key, NULL);
}

EAPI void
ethumb_thumb_path_set(Ethumb *e, const char *path, const char *key)
{
   char buf[PATH_MAX];

   EINA_SAFETY_ON_NULL_RETURN(e);

   if (!path)
     {
	eina_stringshare_replace(&e->thumb_path, NULL);
	eina_stringshare_replace(&e->thumb_key, NULL);
     }
   else
     {
	path = _ethumb_build_absolute_path(path, buf);
	eina_stringshare_replace(&e->thumb_path, path);
	eina_stringshare_replace(&e->thumb_key, key);
     }
}

EAPI void
ethumb_thumb_path_get(Ethumb *e, const char **path, const char **key)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   if (!e->thumb_path)
     _ethumb_file_generate_path(e);

   if (path) *path = e->thumb_path;
   if (key) *key = e->thumb_key;
}

void
ethumb_calculate_aspect_from_ratio(Ethumb *e, float ia, int *w, int *h)
{
   float a;

   *w = e->tw;
   *h = e->th;

   if (ia == 0)
     return;

   a = e->tw / (float)e->th;

   if (e->aspect == ETHUMB_THUMB_KEEP_ASPECT)
     {
	if ((ia > a && e->tw > 0) || e->th <= 0)
	  *h = e->tw / ia;
	else
	  *w = e->th * ia;
     }
}

void
ethumb_calculate_aspect(Ethumb *e, int iw, int ih, int *w, int *h)
{
   float ia;

   ia = iw / (float)ih;

   ethumb_calculate_aspect_from_ratio(e, ia, w, h);
}

void
ethumb_calculate_fill_from_ratio(Ethumb *e, float ia, int *fx, int *fy, int *fw, int *fh)
{
   float a;

   if (ia == 0)
     return;

   *fw = e->tw;
   *fh = e->th;
   *fx = 0;
   *fy = 0;
   a = e->tw / (float)e->th;

   if (e->aspect == ETHUMB_THUMB_CROP)
     {
	if ((ia > a && e->tw > 0) || e->th <= 0)
	  *fw = e->th * ia;
	else
	  *fh = e->tw / ia;

	*fx = - e->crop_x * (*fw - e->tw);
	*fy = - e->crop_y * (*fh - e->th);
     }
   else if (e->aspect == ETHUMB_THUMB_KEEP_ASPECT)
     {
	if ((ia > a && e->tw > 0) || e->th <= 0)
	  *fh = e->tw / ia;
	else
	  *fw = e->th * ia;
     }
}

void
ethumb_calculate_fill(Ethumb *e, int iw, int ih, int *fx, int *fy, int *fw, int *fh)
{
   float ia;
   ia = iw / (float)ih;

   ethumb_calculate_fill_from_ratio(e, ia, fx, fy, fw, fh);
}

static Eina_Bool
_ethumb_plugin_generate(Ethumb *e)
{
   const char *ext;
   Ethumb_Plugin *plugin;

   ext = strrchr(e->src_path, '.');
   if (!ext)
     {
	ERR("could not get extension for file \"%s\"\n", e->src_path);
	return EINA_FALSE;
     }

   plugin = eina_hash_find(_plugins_ext, ext + 1);
   if (!plugin)
     {
	DBG("no plugin for extension: \"%s\"\n", ext + 1);
	return EINA_FALSE;
     }

   if (e->frame)
     evas_object_hide(e->frame->edje);
   else
     evas_object_hide(e->img);

   plugin->generate_thumb(e);

   return EINA_TRUE;
}

Eina_Bool
ethumb_plugin_image_resize(Ethumb *e, int w, int h)
{
   Evas_Object *img;

   img = e->img;

   if (e->frame)
     {
	edje_extern_object_min_size_set(img, w, h);
	edje_extern_object_max_size_set(img, w, h);
	edje_object_calc_force(e->frame->edje);
	evas_object_move(e->frame->edje, 0, 0);
	evas_object_resize(e->frame->edje, w, h);
     }
   else
     {
	evas_object_move(img, 0, 0);
	evas_object_resize(img, w, h);
     }

   evas_object_image_size_set(e->o, w, h);
   ecore_evas_resize(e->sub_ee, w, h);

   e->rw = w;
   e->rh = h;

   return EINA_TRUE;
}

Eina_Bool
ethumb_image_save(Ethumb *e)
{
   Eina_Bool r;
   char *dname;
   char flags[256];

   evas_damage_rectangle_add(e->sub_e, 0, 0, e->rw, e->rh);
   evas_render(e->sub_e);

   if (!e->thumb_path)
     _ethumb_file_generate_path(e);

   if (!e->thumb_path)
     {
	ERR("could not create file path...\n");
	return EINA_FALSE;
     }

   dname = ecore_file_dir_get(e->thumb_path);
   r = ecore_file_mkpath(dname);
   free(dname);
   if (!r)
     {
	ERR("could not create directory '%s'\n", dname);
	return EINA_FALSE;
     }

   snprintf(flags, sizeof(flags), "quality=%d compress=%d",
	    e->quality, e->compress);
   r = evas_object_image_save(e->o, e->thumb_path, e->thumb_key, flags);

   if (!r)
     {
	ERR("could not save image.\n");
	return EINA_FALSE;
     }

   return EINA_TRUE;
}

static int
_ethumb_image_load(Ethumb *e)
{
   int error;
   Evas_Coord w, h, ww, hh, fx, fy, fw, fh;
   Evas_Object *img;

   img = e->img;

   if (e->frame)
     evas_object_hide(e->frame->edje);
   else
     evas_object_hide(img);
   evas_object_image_file_set(img, NULL, NULL);
   evas_object_image_load_size_set(img, e->tw, e->th);
   evas_object_image_file_set(img, e->src_path, e->src_key);

   if (e->frame)
     evas_object_show(e->frame->edje);
   else
     evas_object_show(img);

   error = evas_object_image_load_error_get(img);
   if (error != EVAS_LOAD_ERROR_NONE)
     {
	ERR("could not load image '%s': %d\n", e->src_path, error);
	return 0;
     }

   evas_object_image_size_get(img, &w, &h);
   if ((w <= 0) || (h <= 0))
     return 0;

   ethumb_calculate_aspect(e, w, h, &ww, &hh);

   if (e->frame)
     {
	edje_extern_object_min_size_set(img, ww, hh);
	edje_extern_object_max_size_set(img, ww, hh);
	edje_object_calc_force(e->frame->edje);
	evas_object_move(e->frame->edje, 0, 0);
	evas_object_resize(e->frame->edje, ww, hh);
     }
   else
     {
	evas_object_move(img, 0, 0);
	evas_object_resize(img, ww, hh);
     }

   ethumb_calculate_fill(e, w, h, &fx, &fy, &fw, &fh);
   evas_object_image_fill_set(img, fx, fy, fw, fh);

   evas_object_image_size_set(e->o, ww, hh);
   ecore_evas_resize(e->sub_ee, ww, hh);

   e->rw = ww;
   e->rh = hh;

   return 1;
}

static int
_ethumb_finished_idler_cb(void *data)
{
   Ethumb *e = data;

   e->finished_cb(e, e->cb_result, e->cb_data);
   if (e->cb_data_free)
     e->cb_data_free(e->cb_data);
   e->finished_idler = NULL;
   e->finished_cb = NULL;
   e->cb_data = NULL;
   e->cb_data_free = NULL;

   return 0;
}

void
ethumb_finished_callback_call(Ethumb *e, int result)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->cb_result = result;
   if (e->finished_idler)
     ecore_idler_del(e->finished_idler);
   e->finished_idler = ecore_idler_add(_ethumb_finished_idler_cb, e);
}

EAPI Eina_Bool
ethumb_generate(Ethumb *e, ethumb_generate_callback_t finished_cb, void *data, void (*free_data)(void *))
{
   int r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(finished_cb, 0);


   if (e->finished_idler)
     {
	ERR("thumbnail generation already in progress.\n");
	return EINA_FALSE;
     }
   e->finished_cb = finished_cb;
   e->cb_data = data;
   e->cb_data_free = free_data;

   if (!e->src_path)
     {
	ERR("no file set.\n");
	ethumb_finished_callback_call(e, 0);
	return EINA_TRUE;
     }

   r = _ethumb_plugin_generate(e);
   if (r)
     return EINA_TRUE;

   if (!_ethumb_image_load(e))
     {
	ERR("could not load input image.\n");
	ethumb_finished_callback_call(e, 0);
	return EINA_TRUE;
     }

   r = ethumb_image_save(e);

   ethumb_finished_callback_call(e, r);

   return EINA_TRUE;
}

EAPI Eina_Bool
ethumb_exists(Ethumb *e)
{
   struct stat thumb, src;
   int r_thumb, r_src;
   Eina_Bool r = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e->src_path, 0);

   if (!e->thumb_path)
     _ethumb_file_generate_path(e);

   EINA_SAFETY_ON_NULL_RETURN_VAL(e->thumb_path, 0);

   r_thumb = stat(e->thumb_path, &thumb);
   r_src = stat(e->src_path, &src);

   EINA_SAFETY_ON_TRUE_RETURN_VAL(r_src, 0);

   if (r_thumb && errno != ENOENT)
     ERR("could not access file \"%s\": %s\n", e->thumb_path, strerror(errno));
   else if (!r_thumb && thumb.st_mtime > src.st_mtime)
     r = EINA_TRUE;

   return r;
}

Evas *
ethumb_evas_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);

   return e->sub_e;
}

Ecore_Evas *
ethumb_ecore_evas_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);

   return e->sub_ee;
}
