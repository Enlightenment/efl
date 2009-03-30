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
#include "Ethumb.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "md5.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Edje.h>

static int initcount = 0;

static const int THUMB_SIZE_NORMAL = 128;
static const int THUMB_SIZE_LARGE = 256;

EAPI int
ethumb_init(void)
{
   if (initcount)
     return ++initcount;

   eina_stringshare_init();
   evas_init();
   ecore_init();
   ecore_evas_init();
   edje_init();

   return ++initcount;
}

EAPI int
ethumb_shutdown(void)
{
   initcount--;
   if (initcount == 0)
     {
	eina_stringshare_shutdown();
	eina_stringshare_init();
	evas_init();
	ecore_init();
	ecore_evas_init();
	edje_init();
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
   if (!ethumb)
     return NULL;

   ethumb->tw = THUMB_SIZE_NORMAL;
   ethumb->th = THUMB_SIZE_NORMAL;
   ethumb->crop_x = 0.5;
   ethumb->crop_y = 0.5;

   ee = ecore_evas_buffer_new(1, 1);
   e = ecore_evas_get(ee);
   if (!e)
     {
	fputs("ERROR: could not create ecore evas buffer\n", stderr);
	return NULL;
     }

   evas_image_cache_set(e, 0);
   evas_font_cache_set(e, 0);

   o = ecore_evas_object_image_new(ee);
   if (!o)
     {
	fputs("ERROR: could not create sub ecore evas buffer\n", stderr);
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
	fputs("ERROR: could not create source objects.\n", stderr);
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
     o = edje_object_part_swallow_get(frame->edje, frame->swallow);
   if (o)
     edje_object_part_unswallow(frame->edje, o);
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
   if (!ethumb)
     return;
   if (ethumb->frame)
     _ethumb_frame_free(ethumb->frame);
   ecore_evas_free(ethumb->ee);
   eina_stringshare_del(ethumb->thumb_dir);
   eina_stringshare_del(ethumb->category);
   free(ethumb);
}

EAPI void
ethumb_thumb_fdo_set(Ethumb *e, Ethumb_Thumb_Size s)
{
   if (!e || (s != ETHUMB_THUMB_NORMAL && s != ETHUMB_THUMB_LARGE))
     return;

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
   if (e && tw > 0 && th > 0)
     {
	e->tw = tw;
	e->th = th;
     }
}

EAPI void
ethumb_thumb_size_get(const Ethumb *e, int *tw, int *th)
{
   if (e)
     {
	*tw = e->tw;
	*th = e->th;
     }
}

EAPI void
ethumb_thumb_format_set(Ethumb *e, Ethumb_Thumb_Format f)
{
   if (e && (f == ETHUMB_THUMB_FDO || f == ETHUMB_THUMB_JPEG))
     e->format = f;
}

EAPI Ethumb_Thumb_Format
ethumb_thumb_format_get(const Ethumb *e)
{
   if (e)
     return e->format;
   else
     return 0;
}

EAPI void
ethumb_thumb_aspect_set(Ethumb *e, Ethumb_Thumb_Aspect a)
{
   if (e && (a == ETHUMB_THUMB_KEEP_ASPECT || a == ETHUMB_THUMB_IGNORE_ASPECT
	     || a == ETHUMB_THUMB_CROP))
     e->aspect = a;
}

EAPI Ethumb_Thumb_Aspect
ethumb_thumb_aspect_get(const Ethumb *e)
{
   if (e)
     return e->aspect;
   else
     return 0;
}

EAPI void
ethumb_thumb_crop_align_set(Ethumb *e, float x, float y)
{
   if (e)
     {
	e->crop_x = x;
	e->crop_y = y;
     }
}

EAPI void
ethumb_thumb_crop_align_get(Ethumb *e, float *x, float *y)
{
   if (e)
     {
	*x = e->crop_x;
	*y = e->crop_y;
     }
}

EAPI int
ethumb_frame_set(Ethumb *e, const char *theme_file, const char *group, const char *swallow)
{
   if (!e)
     return 0;

   Ethumb_Frame *frame;
   frame = e->frame;

   if (frame)
     edje_object_part_unswallow(frame->edje, e->img);
   else
     {
	frame = calloc(1, sizeof(Ethumb_Frame));
	if (!frame)
	  {
	     fputs("ERROR: could not allocate Ethumb_Frame structure.\n",
		   stderr);
	     return 0;
	  }

	frame->edje = edje_object_add(e->sub_e);
	if (!frame->edje)
	  {
	     fputs("ERROR: could not create edje frame object.\n", stderr);
	     _ethumb_frame_free(frame);
	     return 0;
	  }
     }

   if (!edje_object_file_set(frame->edje, theme_file, group))
     {
	fputs("ERROR: could not load frame theme.\n", stderr);
	_ethumb_frame_free(frame);
	return 0;
     }

   edje_object_part_swallow(frame->edje, swallow, e->img);
   if (!edje_object_part_swallow_get(frame->edje, swallow))
     {
	fputs("ERROR: could not swallow image to edje frame.\n", stderr);
	_ethumb_frame_free(frame);
	return 0;
     }

   theme_file = eina_stringshare_add(theme_file);
   group = eina_stringshare_add(group);
   swallow = eina_stringshare_add(swallow);
   eina_stringshare_del(frame->file);
   eina_stringshare_del(frame->group);
   eina_stringshare_del(frame->swallow);
   frame->file = theme_file;
   frame->group = group;
   frame->swallow = swallow;

   e->frame = frame;

   return 1;
}

EAPI void
ethumb_thumb_dir_path_set(Ethumb *e, const char *path)
{
   if (!e)
     return;

   path = eina_stringshare_add(path);
   eina_stringshare_del(e->thumb_dir);
   e->thumb_dir = path;
}

EAPI const char *
ethumb_thumb_dir_path_get(Ethumb *e)
{
   if (!e)
     return NULL;

   return e->thumb_dir;
}

EAPI void
ethumb_thumb_category_set(Ethumb *e, const char *category)
{
   if (!e)
     return;

   category = eina_stringshare_add(category);
   eina_stringshare_del(e->category);
   e->category = category;
}

EAPI const char *
ethumb_thumb_category_get(Ethumb *e)
{
   if (!e)
     return NULL;

   return e->category;
}

EAPI Ethumb_File *
ethumb_file_new(Ethumb *e, const char *path)
{
   Ethumb_File *ef;

   if (!e)
     return NULL;

   if (access(path, R_OK))
     {
	fprintf(stderr, "ERROR: couldn't access file \"%s\"\n", path);
	return NULL;
     }

   ef = calloc(1, sizeof(Ethumb_File));
   ef->ethumb = e;
   ef->src_path = eina_stringshare_add(path);

   return ef;
}

static const char *
_ethumb_generate_hash(const char *file)
{
  int n;
  MD5_CTX ctx;
  char md5out[(2 * MD5_HASHBYTES) + 1];
  unsigned char hash[MD5_HASHBYTES];
  static const char hex[] = "0123456789abcdef";

  char uri[PATH_MAX];

  if (!file)
    return NULL;
  snprintf (uri, sizeof(uri), "file://%s", file);

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
_ethumb_file_generate_custom_category(Ethumb_File *ef)
{
   char buf[PATH_MAX];
   const char *aspect, *format;
   const char *frame;
   Ethumb *e = ef->ethumb;

   if (e->aspect == ETHUMB_THUMB_KEEP_ASPECT)
     aspect = eina_stringshare_add("keep_aspect");
   else if (e->aspect == ETHUMB_THUMB_IGNORE_ASPECT)
     aspect = eina_stringshare_add("ignore_aspect");
   else
     aspect = eina_stringshare_add("crop");

   if (e->format == ETHUMB_THUMB_FDO)
     format = eina_stringshare_add("png");
   else
     format = eina_stringshare_add("jpg");

   if (e->frame)
     frame = eina_stringshare_add("-framed");
   else
     frame = eina_stringshare_add("");

   snprintf(buf, sizeof(buf), "%dx%d-%s%s-%s",
	    e->tw, e->th, aspect, frame, format);

   eina_stringshare_del(aspect);
   eina_stringshare_del(format);
   eina_stringshare_del(frame);

   return eina_stringshare_add(buf);
}

static void
_ethumb_file_generate_path(Ethumb_File *ef)
{
   char buf[PATH_MAX];
   char *fullname;
   const char *hash;
   const char *thumb_dir, *category;
   const char *ext;
   Ethumb *e;
   int fdo_format;


   e = ef->ethumb;
   fdo_format = _ethumb_file_check_fdo(e);

   if (e->thumb_dir)
     thumb_dir = eina_stringshare_add(e->thumb_dir);
   else
     {
	const char *home;

	home = getenv("HOME");
	snprintf(buf, sizeof(buf), "%s/.thumbnails", home);
	thumb_dir = eina_stringshare_add(buf);
     }

   if (e->category)
     category = eina_stringshare_add(e->category);
   else if (!fdo_format)
     category = _ethumb_file_generate_custom_category(ef);
   else
     {
	if (e->tw == THUMB_SIZE_NORMAL)
	  category = eina_stringshare_add("normal");
	else if (e->tw == THUMB_SIZE_LARGE)
	  category = eina_stringshare_add("large");
     }

   if (e->format == ETHUMB_THUMB_FDO)
     ext = eina_stringshare_add("png");
   else
     ext = eina_stringshare_add("jpg");

   fullname = ecore_file_realpath(ef->src_path);
   hash = _ethumb_generate_hash(fullname);
   snprintf(buf, sizeof(buf), "%s/%s/%s.%s", thumb_dir, category, hash, ext);
   free(fullname);
   if (ef->thumb_path)
     eina_stringshare_del(ef->thumb_path);
   ef->thumb_path = eina_stringshare_add(buf);

   eina_stringshare_del(thumb_dir);
   eina_stringshare_del(category);
   eina_stringshare_del(ext);
   eina_stringshare_del(hash);
}

EAPI void
ethumb_file_free(Ethumb_File *ef)
{
   if (!ef)
     return;

   eina_stringshare_del(ef->src_path);
   eina_stringshare_del(ef->thumb_path);
   free(ef);
}

EAPI void
ethumb_file_thumb_path_set(Ethumb_File *ef, const char *path)
{
   char *real_path;
   char buf[PATH_MAX];

   if (ef->thumb_path)
     eina_stringshare_del(ef->thumb_path);

   real_path = realpath(path, buf);
   if (errno == ENOENT || errno == ENOTDIR || real_path)
     ef->thumb_path = eina_stringshare_add(buf);
   else
     fprintf(stderr, "could not set thumbnail path: %s\n", strerror(errno));
}

EAPI const char *
ethumb_file_thumb_path_get(Ethumb_File *ef)
{
   if (!ef->thumb_path)
     _ethumb_file_generate_path(ef);

   return ef->thumb_path;
}

static void
_ethumb_calculate_aspect(Ethumb *e, int iw, int ih, int *w, int *h)
{
   *w = e->tw;
   *h = e->th;

   if (e->aspect == ETHUMB_THUMB_KEEP_ASPECT)
     {
	if ((iw > ih && e->tw > 0) || e->th <= 0)
	  *h = (e->tw * ih) / iw;
	else
	  *w = (e->th * iw) / ih;
     }
}

static void
_ethumb_calculate_fill(Ethumb *e, int iw, int ih, int *fx, int *fy, int *fw, int *fh)
{
   *fw = e->tw;
   *fh = e->th;
   *fx = 0;
   *fy = 0;

   if (e->aspect == ETHUMB_THUMB_CROP)
     {
	if ((iw > ih && e->tw > 0) || e->th <= 0)
	  *fw = (e->th * iw) / ih;
	else
	  *fh = (e->tw * ih) / iw;

	*fx = - e->crop_x * (*fw - iw);
	*fy = - e->crop_y * (*fh - ih);
     }
   else if (e->aspect == ETHUMB_THUMB_KEEP_ASPECT)
     {
	if ((iw > ih && e->tw > 0) || e->th <= 0)
	  *fh = (e->tw * ih) / iw;
	else
	  *fw = (e->th * iw) / ih;
     }
}

static int
_ethumb_image_load(Ethumb_File *ef)
{
   Ethumb *eth;
   int error;
   Evas_Coord w, h, ww, hh, fx, fy, fw, fh;
   Evas_Object *img;

   eth = ef->ethumb;
   img = eth->img;

   if (eth->frame)
     evas_object_hide(eth->frame->edje);
   else
     evas_object_hide(img);
   evas_object_image_file_set(img, NULL, NULL);
   evas_object_image_load_size_set(img, eth->tw, eth->th);
   evas_object_image_file_set(img, ef->src_path, NULL);

   if (eth->frame)
     evas_object_show(eth->frame->edje);
   else
     evas_object_show(img);

   error = evas_object_image_load_error_get(img);
   if (error != EVAS_LOAD_ERROR_NONE)
     {
	fprintf(stderr, "ERROR: could not load image '%s': %d\n",
		ef->src_path, error);
	return 0;
     }

   evas_object_image_size_get(img, &w, &h);
   if ((w <= 0) || (h <= 0))
     return 0;

   _ethumb_calculate_aspect(eth, w, h, &ww, &hh);

   if (eth->frame)
     {
	edje_extern_object_min_size_set(img, ww, hh);
	edje_extern_object_max_size_set(img, ww, hh);
	edje_object_calc_force(eth->frame->edje);
	evas_object_move(eth->frame->edje, 0, 0);
	evas_object_resize(eth->frame->edje, ww, hh);
     }
   else
     {
	evas_object_move(img, 0, 0);
	evas_object_resize(img, ww, hh);
     }

   _ethumb_calculate_fill(eth, w, h, &fx, &fy, &fw, &fh);
   evas_object_image_fill_set(img, fx, fy, fw, fh);

   evas_object_image_size_set(eth->o, ww, hh);
   ecore_evas_resize(eth->sub_ee, ww, hh);

   evas_damage_rectangle_add(eth->sub_e, 0, 0, ww, hh);

   ef->w = ww;
   ef->h = hh;

   return 1;
}

EAPI int
ethumb_file_generate(Ethumb_File *ef)
{
   Ethumb *eth;
   int r;
   char *dname;

   if (!ef)
     return 0;

   eth = ef->ethumb;


   if (!_ethumb_image_load(ef))
     {
	fputs("ERROR: could not load input image.\n", stderr);
	return 0;
     }

   evas_render(eth->sub_e);

   if (!ef->thumb_path)
     _ethumb_file_generate_path(ef);

   if (!ef->thumb_path)
     {
	fputs("ERROR: could not create file path...\n", stderr);
	return 0;
     }

   dname = ecore_file_dir_get(ef->thumb_path);
   r = ecore_file_mkpath(dname);
   free(dname);
   if (!r)
     {
	fprintf(stderr, "ERROR: could not create directory '%s'\n", dname);
	return 0;
     }

   r = evas_object_image_save(eth->o, ef->thumb_path, NULL, "quality=85");

   if (!r)
     {
	fputs("ERROR: could not save image.\n", stderr);
	return 0;
     }

   return 1;
}
