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

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

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
#include <ctype.h>
#include <pwd.h>

#ifdef HAVE_XATTR
# include <sys/xattr.h>
#endif

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif

#ifdef HAVE_LIBEXIF
  #include <libexif/exif-data.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Eina.h>
#include <eina_safety_checks.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Edje.h>

#include "Ethumb.h"
#include "ethumb_private.h"
#include "Ethumb_Plugin.h"
#include "md5.h"

static Ethumb_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Ethumb_Version *ethumb_version = &_version;

static int _log_dom = -1;
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)

static int initcount = 0;
static Eina_Bool _plugins_loaded = EINA_FALSE;
static const char *_home_thumb_dir = NULL;
static const char *_thumb_category_normal = NULL;
static const char *_thumb_category_large = NULL;

static const int THUMB_SIZE_NORMAL = 128;
static const int THUMB_SIZE_LARGE = 256;

static Eina_Hash *_plugins_ext = NULL;
static Eina_Array *_plugins = NULL;
static Eina_Prefix *_pfx = NULL;

EAPI Eina_Bool
ethumb_plugin_register(const Ethumb_Plugin *plugin)
{
   const char * const *ext;

   EINA_SAFETY_ON_NULL_RETURN_VAL(plugin, EINA_FALSE);

   if (plugin->version != ETHUMB_PLUGIN_API_VERSION)
     {
        ERR("Plugin '%p' uses api version=%u while %u was expected",
            plugin, plugin->version, ETHUMB_PLUGIN_API_VERSION);
        return EINA_FALSE;
     }

   EINA_SAFETY_ON_NULL_RETURN_VAL(plugin->name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(plugin->extensions, EINA_FALSE);

   DBG("registered plugin '%s' (%p) with extensions:", plugin->name, plugin);
   for (ext = plugin->extensions; *ext; ext++)
     {
        Eina_Bool r = eina_hash_add(_plugins_ext, *ext, plugin);
        DBG("   extension \"%s\": %hhu", *ext, r);
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
ethumb_plugin_unregister(const Ethumb_Plugin *plugin)
{
   const char * const *ext;

   EINA_SAFETY_ON_NULL_RETURN_VAL(plugin, EINA_FALSE);

   if (plugin->version != ETHUMB_PLUGIN_API_VERSION)
     {
        ERR("Plugin '%p' uses api version=%u while %u was expected",
            plugin, plugin->version, ETHUMB_PLUGIN_API_VERSION);
        return EINA_FALSE;
     }

   EINA_SAFETY_ON_NULL_RETURN_VAL(plugin->name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(plugin->extensions, EINA_FALSE);

   DBG("unregister plugin '%s' (%p) with extensions:", plugin->name, plugin);
   for (ext = plugin->extensions; *ext; ext++)
     {
        Eina_Bool r = eina_hash_del(_plugins_ext, *ext, plugin);
        DBG("   extension \"%s\": %hhu", *ext, r);
     }

   return EINA_TRUE;
}


static void
_ethumb_plugins_load(void)
{
   char buf[PATH_MAX];

   if (_plugins_loaded) return;
   _plugins_loaded = EINA_TRUE;

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        if (getenv("EFL_RUN_IN_TREE"))
          {
             struct stat st;
             snprintf(buf, sizeof(buf), "%s/src/modules/ethumb",
                      PACKAGE_BUILD_DIR);
             if (stat(buf, &st) == 0)
               {
                  const char *built_modules[] = {
                     "emotion",
                     NULL
                  };
                  const char **itr;
                  for (itr = built_modules; *itr != NULL; itr++)
                    {
                       snprintf(buf, sizeof(buf),
                                "%s/src/modules/ethumb/%s/.libs",
                                PACKAGE_BUILD_DIR, *itr);
                       _plugins = eina_module_list_get(_plugins, buf,
                                                       EINA_FALSE, NULL, NULL);
                    }
                  goto load;
               }
          }
     }

   snprintf(buf, sizeof(buf), "%s/ethumb/modules", eina_prefix_lib_get(_pfx));
   _plugins = eina_module_arch_list_get(_plugins, buf, MODULE_ARCH);

 load:
   // XXX: MODFIX: do not list ALL modules and load them ALL! this is
   // wasteful. admittedly this is low priority as we have only 1
   // module - and that is emotion and ethumbd slaves die off quickly
   // but we still pay a module load, init func etc. price even if
   // the code is never needed!
   if (_plugins)
     eina_module_list_load(_plugins);

   if (!eina_hash_population(_plugins_ext))
     ERR("Couldn't find any ethumb plugin.");
}

static void
_ethumb_plugins_unload(void)
{
   if (_plugins)
     {
        eina_module_list_free(_plugins);
        eina_array_free(_plugins);
        _plugins = NULL;
     }

   eina_hash_free(_plugins_ext);
   _plugins_ext = NULL;

   _plugins_loaded = EINA_FALSE;
}

EAPI int
ethumb_init(void)
{
   const char *home;
   char buf[PATH_MAX];

   if (initcount)
     return ++initcount;

   if (!eina_init())
     {
        fprintf(stderr, "ERROR: Could not initialize eina.\n");
        return 0;
     }
   _log_dom = eina_log_domain_register("ethumb", EINA_COLOR_GREEN);
   if (_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: ethumb");
        goto error_log;
     }

   _pfx = eina_prefix_new(NULL, ethumb_init,
                          "ETHUMB", "ethumb", "checkme",
                          PACKAGE_BIN_DIR, PACKAGE_LIB_DIR,
                          PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);
   if (!_pfx)
     {
        ERR("Could not get ethumb installation prefix.");
        goto error_pfx;
     }

   _plugins_ext = eina_hash_string_small_new(NULL);
   EINA_SAFETY_ON_NULL_GOTO(_plugins_ext, error_plugins_ext);

   evas_init();
   ecore_init();
   ecore_evas_init();
   edje_init();

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        home = getenv("HOME");
        snprintf(buf, sizeof(buf), "%s/.thumbnails", home);
     }
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   else
     {
        struct passwd *pw = getpwent();

        if ((!pw) || (!pw->pw_dir)) goto error_plugins_ext;
        snprintf(buf, sizeof(buf), "%s/.thumbnails", pw->pw_dir);
     }
#endif

   _home_thumb_dir = eina_stringshare_add(buf);
   _thumb_category_normal = eina_stringshare_add("normal");
   _thumb_category_large = eina_stringshare_add("large");

   return ++initcount;

 error_plugins_ext:
   eina_prefix_free(_pfx);
   _pfx = NULL;

 error_pfx:
   eina_log_domain_unregister(_log_dom);
   _log_dom = -1;

 error_log:
   eina_shutdown();
   return 0;
}

EAPI int
ethumb_shutdown(void)
{
   if (initcount <= 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   initcount--;
   if (initcount == 0)
     {
        _ethumb_plugins_unload();
        eina_stringshare_del(_home_thumb_dir);
        eina_stringshare_del(_thumb_category_normal);
        eina_stringshare_del(_thumb_category_large);
        evas_shutdown();
        ecore_shutdown();
        ecore_evas_shutdown();
        edje_shutdown();
        eina_prefix_free(_pfx);
        _pfx = NULL;
        eina_log_domain_unregister(_log_dom);
        _log_dom = -1;
        eina_shutdown();
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

   /* IF CHANGED, UPDATE DOCS in (Ethumb.c, Ethumb_Client.c, python...)!!! */
   ethumb->tw = THUMB_SIZE_NORMAL;
   ethumb->th = THUMB_SIZE_NORMAL;
   ethumb->orientation = ETHUMB_THUMB_ORIENT_ORIGINAL;
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
        ERR("could not create ecore evas buffer");
        free(ethumb);
        return NULL;
     }

   evas_image_cache_set(e, 0);
   evas_font_cache_set(e, 0);

   o = ecore_evas_object_image_new(ee);
   if (!o)
     {
        ERR("could not create sub ecore evas buffer");
        ecore_evas_free(ee);
        free(ethumb);
        return NULL;
     }

   sub_ee = ecore_evas_object_ecore_evas_get(o);
   sub_e = ecore_evas_object_evas_get(o);
   ecore_evas_alpha_set(sub_ee, EINA_TRUE);

   evas_image_cache_set(sub_e, 0);
   evas_font_cache_set(sub_e, 0);

   img = evas_object_image_add(sub_e);
   if (!img)
     {
        ERR("could not create source objects.");
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

   DBG("ethumb=%p", ethumb);

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

   DBG("ethumb=%p", ethumb);

   if (ethumb->frame)
     _ethumb_frame_free(ethumb->frame);
   ethumb_file_free(ethumb);
   evas_object_del(ethumb->o);
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
   DBG("ethumb=%p, size=%d", e, s);

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
   e->orientation = ETHUMB_THUMB_ORIENT_ORIGINAL;
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

   DBG("ethumb=%p, w=%d, h=%d", e, tw, th);
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

   DBG("ethumb=%p, format=%d", e, f);
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

   DBG("ethumb=%p, aspect=%d", e, a);
   e->aspect = a;
}

EAPI Ethumb_Thumb_Aspect
ethumb_thumb_aspect_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);
   return e->aspect;
}

EAPI void
ethumb_thumb_orientation_set(Ethumb *e, Ethumb_Thumb_Orientation o)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_SAFETY_ON_FALSE_RETURN(o == ETHUMB_THUMB_ORIENT_NONE ||
                               o == ETHUMB_THUMB_ROTATE_90_CW ||
                               o == ETHUMB_THUMB_ROTATE_180 ||
                               o == ETHUMB_THUMB_ROTATE_90_CCW ||
                               o == ETHUMB_THUMB_FLIP_HORIZONTAL ||
                               o == ETHUMB_THUMB_FLIP_VERTICAL ||
                               o == ETHUMB_THUMB_FLIP_TRANSPOSE ||
                               o == ETHUMB_THUMB_FLIP_TRANSVERSE ||
                               o == ETHUMB_THUMB_ORIENT_ORIGINAL);

   DBG("ethumb=%p, orientation=%d", e, o);
   e->orientation = o;
}

EAPI Ethumb_Thumb_Orientation
ethumb_thumb_orientation_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);
   return e->orientation;
}

EAPI void
ethumb_thumb_crop_align_set(Ethumb *e, float x, float y)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   DBG("ethumb=%p, x=%f, y=%f", e, x, y);
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

   DBG("ethumb=%p, quality=%d", e, quality);
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

   DBG("ethumb=%p, compress=%d", e, compress);
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

   DBG("ethumb=%p, theme_file=%s, group=%s, swallow=%s",
       e, theme_file ? theme_file : "", group ? group : "",
       swallow ? swallow : "");

   if (frame)
     {
        edje_object_part_unswallow(frame->edje, e->img);
        if (!theme_file)
          _ethumb_frame_free(frame);
     }

   if (!theme_file)
     {
        e->frame = NULL;
        return EINA_FALSE;
     }

   if (!frame)
     {
        frame = calloc(1, sizeof(Ethumb_Frame));
        if (!frame)
          {
             ERR("could not allocate Ethumb_Frame structure.");
             return EINA_FALSE;
          }

        frame->edje = edje_object_add(e->sub_e);
        if (!frame->edje)
          {
             ERR("could not create edje frame object.");
             _ethumb_frame_free(frame);
             e->frame = NULL;
             return EINA_FALSE;
          }
     }

   if (!edje_object_file_set(frame->edje, theme_file, group))
     {
        ERR("could not load frame theme.");
        _ethumb_frame_free(frame);
        e->frame = NULL;
        return EINA_FALSE;
     }

   edje_object_part_swallow(frame->edje, swallow, e->img);
   if (!edje_object_part_swallow_get(frame->edje, swallow))
     {
        ERR("could not swallow image to edje frame.");
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
_ethumb_build_absolute_path(const char *path, char buf[PATH_MAX])
{
   char *p;
   int len;

   if (!path)
     return NULL;

   p = buf;

   if (path[0] == '/')
     {
        strncpy(p, path, PATH_MAX - 1);
        p[PATH_MAX - 1] = 0;
     }
   else if (path[0] == '~')
     {
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
        if (getuid() == geteuid())
#endif
          {
             const char *home = getenv("HOME");
             if (!home) return NULL;
             strncpy(p, home, PATH_MAX - 1);
             p[PATH_MAX - 1] = 0;
          }
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
        else
          {
             struct passwd *pw = getpwent();

             if ((!pw) || (!pw->pw_dir)) return NULL;
             strncpy(p, pw->pw_dir, PATH_MAX - 1);
             p[PATH_MAX - 1] = 0;
          }
#endif
        len = strlen(p);
        p += len;
        p[0] = '/';
        p++;
        strcpy(p, path + 2);
     }
   else
     {
        if (!getcwd(p, PATH_MAX))
          return NULL;
        len = strlen(p);
        p += len;
        p[0] = '/';
        p++;
        strncpy(p, path, PATH_MAX - 1 - len - 1);
        p[PATH_MAX - 1 - len - 1] = 0;
     }

   return buf;
}

EAPI void
ethumb_thumb_dir_path_set(Ethumb *e, const char *path)
{
   char buf[PATH_MAX];
   EINA_SAFETY_ON_NULL_RETURN(e);

   DBG("ethumb=%p, path=%s", e, path ? path : "");
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

   DBG("ethumb=%p, category=%s", e, category ? category : "");
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
   EINA_SAFETY_ON_FALSE_RETURN(start >= 0.0);
   EINA_SAFETY_ON_FALSE_RETURN(start <= 1.0);

   DBG("ethumb=%p, video_start=%f", e, start);
   e->video.start = start;
}

EAPI float
ethumb_video_start_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   return e->video.start;
}

EAPI void
ethumb_video_time_set(Ethumb *e, float t)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   DBG("ethumb=%p, video_start=%f", e, t);
   e->video.time = t;
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

   DBG("ethumb=%p, video_interval=%f", e, interval);
   e->video.interval = interval;
}

EAPI float
ethumb_video_interval_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   return e->video.interval;
}

EAPI void
ethumb_video_ntimes_set(Ethumb *e, unsigned int ntimes)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_SAFETY_ON_FALSE_RETURN(ntimes > 0);

   DBG("ethumb=%p, video_ntimes=%d", e, ntimes);
   e->video.ntimes = ntimes;
}

EAPI unsigned int
ethumb_video_ntimes_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   return e->video.ntimes;
}

EAPI void
ethumb_video_fps_set(Ethumb *e, unsigned int fps)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_SAFETY_ON_FALSE_RETURN(fps > 0);

   DBG("ethumb=%p, video_fps=%d", e, fps);
   e->video.fps = fps;
}

EAPI unsigned int
ethumb_video_fps_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   return e->video.fps;
}

EAPI void
ethumb_document_page_set(Ethumb *e, unsigned int page)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   DBG("ethumb=%p, document_page=%d", e, page);
   e->document.page = page;
}

EAPI unsigned int
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

   eina_stringshare_replace(&e->thumb_path, NULL);
   eina_stringshare_replace(&e->thumb_key, NULL);

   DBG("ethumb=%p, path=%s, key=%s", e, path ? path : "", key ? key : "");
   if (path && access(path, R_OK))
     {
        ERR("couldn't access file \"%s\"", path);
        return EINA_FALSE;
     }

   path = _ethumb_build_absolute_path(path, buf);
   eina_stringshare_replace(&e->src_hash, NULL);
   eina_stringshare_replace(&e->src_path, path);
   eina_stringshare_replace(&e->src_key, key);

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
   const unsigned char *c;

#ifdef HAVE_XATTR
   ssize_t length;

   length = getxattr(file, "user.e.md5", NULL, 0);

   if (length > 0)
     {
        char *tmp;

        tmp = alloca(length);
        length = getxattr(file, "user.e.md5", tmp, length);

        /* check if we have at least something that look like a md5 hash */
        if (length > 0 && (length == MD5_HASHBYTES * 2 + 1))
          {
             tmp[length] = '\0';
             return eina_stringshare_add(tmp);
          }
     }
#endif

#define _check_uri_char(c) \
   ((c) >= 32 && (c) < 128 && (ACCEPTABLE_URI_CHARS[(c) - 32] & 0x08))

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

   uri = alloca(3 * strlen(file) + 9);
   memcpy(uri, "file://", sizeof("file://") - 1);
   t = uri + sizeof("file://") - 1;

   for (c = (const unsigned char *)file; *c != '\0'; c++)
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

#ifdef HAVE_XATTR
   setxattr(file, "user.e.md5", md5out, 2 * n + 1, 0);
#endif

   DBG("md5=%s, file=%s", md5out, file);
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
        else
          {
             ERR("fdo_format but size %d is not NORMAL (%d) or LARGE (%d)?",
                 e->tw, THUMB_SIZE_NORMAL, THUMB_SIZE_LARGE);
             category = "unknown";
          }
     }

   if (e->format == ETHUMB_THUMB_FDO)
     ext = "png";
   else if (e->format == ETHUMB_THUMB_JPEG)
     ext = "jpg";
   else
     ext = "eet";

   if (!e->src_hash)
     {
        char *fullname;

        fullname = ecore_file_realpath(e->src_path);
        e->src_hash = _ethumb_generate_hash(fullname);
        free(fullname);
     }
   snprintf(buf, sizeof(buf), "%s/%s/%s.%s", thumb_dir, category, e->src_hash, ext);
   DBG("ethumb=%p, path=%s", e, buf);
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
}

EAPI void
ethumb_file_free(Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   DBG("ethumb=%p", e);

   eina_stringshare_replace(&e->src_hash, NULL);
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
   DBG("ethumb=%p, path=%s, key=%s", e, path ? path : "", key ? key : "");

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

EAPI void
ethumb_thumb_hash(Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   if (!e->src_hash)
     {
        char *fullname;

        fullname = ecore_file_realpath(e->src_path);
        e->src_hash = _ethumb_generate_hash(fullname);
        free(fullname);
     }
}

EAPI void
ethumb_thumb_hash_copy(Ethumb *dst, const Ethumb *src)
{
   EINA_SAFETY_ON_NULL_RETURN(dst);
   EINA_SAFETY_ON_NULL_RETURN(src);

   if (src == dst) return;

   eina_stringshare_del(dst->src_hash);
   dst->src_hash = eina_stringshare_ref(src->src_hash);
}

EAPI void
ethumb_calculate_aspect_from_ratio(Ethumb *e, float ia, int *w, int *h)
{
   float a;

   EINA_SAFETY_ON_NULL_RETURN(e);

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

EAPI void
ethumb_calculate_aspect(Ethumb *e, int iw, int ih, int *w, int *h)
{
   float ia;

   if (ih == 0)
     return;

   ia = iw / (float)ih;

   ethumb_calculate_aspect_from_ratio(e, ia, w, h);
}

EAPI void
ethumb_calculate_fill_from_ratio(Ethumb *e, float ia, int *fx, int *fy, int *fw, int *fh)
{
   float a;

   EINA_SAFETY_ON_NULL_RETURN(e);

   *fw = e->tw;
   *fh = e->th;
   *fx = 0;
   *fy = 0;

   if (ia == 0)
     return;

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

EAPI void
ethumb_calculate_fill(Ethumb *e, int iw, int ih, int *fx, int *fy, int *fw, int *fh)
{
   float ia;

   if (ih == 0)
     return;

   ia = iw / (float)ih;

   ethumb_calculate_fill_from_ratio(e, ia, fx, fy, fw, fh);
}

static Eina_Bool
_ethumb_plugin_generate(Ethumb *e)
{
   const char *extp;
   char ext[PATH_MAX];
   Ethumb_Plugin *plugin;
   int i;

   extp = strrchr(e->src_path, '.');
   if (!extp)
     {
        ERR("could not get extension for file \"%s\"", e->src_path);
        return EINA_FALSE;
     }

   for (i = 0; extp[i] != '\0'; i++)
     ext[i] = tolower(extp[i + 1]);

   _ethumb_plugins_load();

   plugin = eina_hash_find(_plugins_ext, ext);
   if (!plugin)
     {
        DBG("no plugin for extension: \"%s\"", ext);
        return EINA_FALSE;
     }

   if (e->frame)
     evas_object_hide(e->frame->edje);
   else
     evas_object_hide(e->img);

   e->plugin = plugin;
   e->pdata = plugin->thumb_generate(e);

   return EINA_TRUE;
}

EAPI Eina_Bool
ethumb_plugin_image_resize(Ethumb *e, int w, int h)
{
   Evas_Object *img;

   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   img = e->img;

   if (e->frame)
     {
        evas_object_move(e->frame->edje, 0, 0);
        evas_object_resize(e->frame->edje, w, h);
        edje_object_calc_force(e->frame->edje);
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

EAPI Eina_Bool
ethumb_image_save(Ethumb *e)
{
   Eina_Bool r;
   char *dname, *buf, flags[256];
   int len;

   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);

   evas_damage_rectangle_add(e->sub_e, 0, 0, e->rw, e->rh);
   evas_render(e->sub_e);

   if (!e->thumb_path)
     _ethumb_file_generate_path(e);

   if (!e->thumb_path)
     {
        ERR("could not create file path...");
        return EINA_FALSE;
     }

   dname = ecore_file_dir_get(e->thumb_path);
   r = ecore_file_mkpath(dname);
   if (!r)
     {
        ERR("could not create directory '%s'", dname);
        free(dname);
        return EINA_FALSE;
     }
   buf = alloca(strlen(e->thumb_path) + 1 + 5);
   strcpy(buf, dname);
   len = strlen(dname);
   strcpy(&(buf[len]), "/.tmp.");
   strcpy(&(buf[len + 6]), e->thumb_path + len + 1);
   free(dname);

   snprintf(flags, sizeof(flags), "quality=%d compress=%d",
            e->quality, e->compress);
   r = evas_object_image_save(e->o, buf, e->thumb_key, flags);
   if (!r)
     {
        ERR("could not save image: path=%s, key=%s", e->thumb_path,
            e->thumb_key);
        return EINA_FALSE;
     }
   if (rename(buf, e->thumb_path) < 0)
     {
        ERR("could not rename image: path=%s, key=%s to path=%s",
            buf, e->thumb_key, e->thumb_path);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_ethumb_image_orient(Ethumb *e, int orientation)
{
   Evas_Object *img = e->img, *tmp;
   unsigned int *data, *data2, *to, *from, *p1, *p2, pt;
   int x, y, w, hw, iw, ih, tw, th;
   const char *file, *key;

   evas_object_image_size_get(img, &iw, &ih);
   evas_object_image_load_size_get(img, &tw, &th);
   evas_object_image_file_get(img, &file, &key);
   data = evas_object_image_data_get(img, 1);

   switch (orientation)
     {
      case ETHUMB_THUMB_FLIP_HORIZONTAL:
         for (y = 0; y < ih; y++)
           {
              p1 = data + (y * iw);
              p2 = data + ((y + 1) * iw) - 1;
              for (x = 0; x < (iw >> 1); x++)
                {
                   pt = *p1;
                   *p1 = *p2;
                   *p2 = pt;
                   p1++;
                   p2--;
                }
           }
         evas_object_image_data_set(img, data);
         evas_object_image_data_update_add(img, 0, 0, iw, ih);
         return;
      case ETHUMB_THUMB_FLIP_VERTICAL:
         for (y = 0; y < (ih >> 1); y++)
           {
              p1 = data + (y * iw);
              p2 = data + ((ih - 1 - y) * iw);
              for (x = 0; x < iw; x++)
                {
                   pt = *p1;
                   *p1 = *p2;
                   *p2 = pt;
                   p1++;
                   p2++;
                }
           }
         evas_object_image_data_set(img, data);
         evas_object_image_data_update_add(img, 0, 0, iw, ih);
         return;
      case ETHUMB_THUMB_ROTATE_180:
         hw = iw * ih;
         x = (hw / 2);
         p1 = data;
         p2 = data + hw - 1;
         for (; --x > 0;)
           {
              pt = *p1;
              *p1 = *p2;
              *p2 = pt;
              p1++;
              p2--;
           }
         evas_object_image_data_set(img, data);
         evas_object_image_data_update_add(img, 0, 0, iw, ih);
         return;
     }

   tmp = evas_object_image_add(evas_object_evas_get(img));
   evas_object_image_load_size_set(tmp, tw, th);
   evas_object_image_file_set(tmp, file, key);
   data2 = evas_object_image_data_get(tmp, 0);

   w = ih;
   ih = iw;
   iw = w;
   hw = w * ih;

   evas_object_image_size_set(img, iw, ih);
   data = evas_object_image_data_get(img, 1);

   switch (orientation)
     {
      case ETHUMB_THUMB_FLIP_TRANSPOSE:
         to = data;
         hw = -hw + 1;
         break;
      case ETHUMB_THUMB_FLIP_TRANSVERSE:
         to = data + hw - 1;
         w = -w;
         hw = hw - 1;
         break;
      case ETHUMB_THUMB_ROTATE_90_CW:
         to = data + w - 1;
         hw = -hw - 1;
         break;
      case ETHUMB_THUMB_ROTATE_90_CCW:
         to = data + hw - w;
         w = -w;
         hw = hw + 1;
         break;
      default:
         ERR("unknown orient %d", orientation);
         evas_object_del(tmp);
         evas_object_image_data_set(img, data); // give it back
         return;
     }
   from = data2;
   for (x = iw; --x >= 0;)
     {
        for (y = ih; --y >= 0;)
          {
             *to = *from;
             from++;
             to += w;
          }
        to += hw;
     }
   evas_object_del(tmp);
   evas_object_image_data_set(img, data);
   evas_object_image_data_update_add(img, 0, 0, iw, ih);
}

static int
_ethumb_image_load(Ethumb *e)
{
   int error;
   Evas_Coord w, h, ww, hh, fx, fy, fw, fh;
   Evas_Object *img;
   int orientation = ETHUMB_THUMB_ORIENT_NONE;

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
        ERR("could not load image '%s': %d", e->src_path, error);
        return 0;
     }

   if (e->orientation == ETHUMB_THUMB_ORIENT_ORIGINAL)
     {
        /* TODO: rewrite to not need libexif just to get this */
#ifdef HAVE_LIBEXIF
        ExifData  *exif = exif_data_new_from_file(e->src_path);
        ExifEntry *entry = NULL;
        ExifByteOrder bo;
        int o = 0;

        if (exif)
          {
             entry = exif_data_get_entry(exif, EXIF_TAG_ORIENTATION);
             if (entry)
               {
                  bo = exif_data_get_byte_order(exif);
                  o = exif_get_short(entry->data, bo);
               }
             exif_data_free(exif);
             switch (o)
               {
                case 2:
                   orientation = ETHUMB_THUMB_FLIP_HORIZONTAL;
                   break;
                case 3:
                   orientation = ETHUMB_THUMB_ROTATE_180;
                   break;
                case 4:
                   orientation = ETHUMB_THUMB_FLIP_VERTICAL;
                   break;
                case 5:
                   orientation = ETHUMB_THUMB_FLIP_TRANSPOSE;
                   break;
                case 6:
                   orientation = ETHUMB_THUMB_ROTATE_90_CW;
                   break;
                case 7:
                   orientation = ETHUMB_THUMB_FLIP_TRANSVERSE;
                   break;
                case 8:
                   orientation = ETHUMB_THUMB_ROTATE_90_CCW;
                   break;
               }
          }
#endif
     }
   else
     orientation = e->orientation;

   if (orientation != ETHUMB_THUMB_ORIENT_NONE)
     _ethumb_image_orient(e, orientation);

   evas_object_image_size_get(img, &w, &h);
   if ((w <= 0) || (h <= 0))
     return 0;

   ethumb_calculate_aspect(e, w, h, &ww, &hh);

   if (e->frame)
     {
        evas_object_move(e->frame->edje, 0, 0);
        evas_object_resize(e->frame->edje, ww, hh);
        edje_object_calc_force(e->frame->edje);
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

static Eina_Bool
_ethumb_finished_idler_cb(void *data)
{
   Ethumb *e = data;

   e->finished_cb(e->cb_data, e, e->cb_result);
   if (e->cb_data_free)
     e->cb_data_free(e->cb_data);
   e->finished_idler = NULL;
   e->finished_cb = NULL;
   e->cb_data = NULL;
   e->cb_data_free = NULL;

   return EINA_FALSE;
}

EAPI void
ethumb_finished_callback_call(Ethumb *e, int result)
{
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->cb_result = result;
   if (e->finished_idler)
     ecore_idler_del(e->finished_idler);
   e->finished_idler = ecore_idler_add(_ethumb_finished_idler_cb, e);
   e->plugin = NULL;
   e->pdata = NULL;
}

EAPI Eina_Bool
ethumb_generate(Ethumb *e, Ethumb_Generate_Cb finished_cb, const void *data, Eina_Free_Cb free_data)
{
   int r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(e, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(finished_cb, 0);
   DBG("ethumb=%p, finished_cb=%p, data=%p, free_data=%p, path=%s, key=%s",
       e, finished_cb, data, free_data,
       e->src_path ? e->src_path : "", e->src_key ? e->src_key : "");

   if (e->finished_idler)
     {
        ERR("thumbnail generation already in progress.");
        return EINA_FALSE;
     }
   if (e->pdata)
     {
        e->plugin->thumb_cancel(e, e->pdata);
        e->pdata = NULL;
        e->plugin = NULL;
     }

   e->finished_cb = finished_cb;
   e->cb_data = (void *)data;
   e->cb_data_free = free_data;

   if (!e->src_path)
     {
        ERR("no file set.");
        ethumb_finished_callback_call(e, 0);
        return EINA_FALSE;
     }

   r = _ethumb_plugin_generate(e);
   DBG("ethumb plugin generate: %i: %p\n", r, e->pdata);
   if (r)
     {
        return EINA_TRUE;
     }

   if (!_ethumb_image_load(e))
     {
        ERR("could not load input image: file=%s, key=%s",
            e->src_path, e->src_key);
        ethumb_finished_callback_call(e, 0);
        return EINA_FALSE;
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
   DBG("ethumb=%p, path=%s", e, e->src_path ? e->src_path : "");

   if (!e->thumb_path)
     _ethumb_file_generate_path(e);

   EINA_SAFETY_ON_NULL_RETURN_VAL(e->thumb_path, 0);

   r_thumb = stat(e->thumb_path, &thumb);
   r_src = stat(e->src_path, &src);

   EINA_SAFETY_ON_TRUE_RETURN_VAL(r_src, 0);

   if (r_thumb && errno != ENOENT)
     ERR("could not access file \"%s\": %s", e->thumb_path, strerror(errno));
   else if (!r_thumb && thumb.st_mtime > src.st_mtime)
     r = EINA_TRUE;

   return r;
}

EAPI Evas *
ethumb_evas_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);

   return e->sub_e;
}

EAPI Ecore_Evas *
ethumb_ecore_evas_get(const Ethumb *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);

   return e->sub_ee;
}

EAPI Ethumb *
ethumb_dup(const Ethumb *e)
{
   Ecore_Evas *ee;
   Ecore_Evas *sub_ee;
   Evas *ev;
   Evas *sub_ev;
   Evas_Object *o;
   Evas_Object *img;
   Ethumb *r;

   r = malloc(sizeof (Ethumb));
   if (!r) return NULL;

   memcpy(r, e, sizeof (Ethumb));

   r->thumb_dir = eina_stringshare_ref(e->thumb_dir);
   r->category = eina_stringshare_ref(e->category);
   r->src_hash = eina_stringshare_ref(e->src_hash);
   r->src_path = eina_stringshare_ref(e->src_path);
   r->src_key = eina_stringshare_ref(e->src_key);
   r->thumb_path = eina_stringshare_ref(e->thumb_path);
   r->thumb_key = eina_stringshare_ref(e->thumb_key);

   ee = ecore_evas_buffer_new(1, 1);
   ev = ecore_evas_get(ee);
   if (!ev)
     {
        ERR("could not create ecore evas buffer");
        free(r);
        return NULL;
     }

   evas_image_cache_set(ev, 0);
   evas_font_cache_set(ev, 0);

   o = ecore_evas_object_image_new(ee);
   if (!o)
     {
        ERR("could not create sub ecore evas buffer");
        ecore_evas_free(ee);
        free(r);
        return NULL;
     }

   sub_ee = ecore_evas_object_ecore_evas_get(o);
   sub_ev = ecore_evas_object_evas_get(o);
   ecore_evas_alpha_set(sub_ee, EINA_TRUE);

   evas_image_cache_set(sub_ev, 0);
   evas_font_cache_set(sub_ev, 0);

   img = evas_object_image_add(sub_ev);
   if (!img)
     {
        ERR("could not create source objects.");
        ecore_evas_free(ee);
        free(r);
        return NULL;
     }

   r->ee = ee;
   r->sub_ee = sub_ee;
   r->e = ev;
   r->sub_e = sub_ev;
   r->o = o;
   r->img = img;

   r->frame = NULL;
   r->finished_idler = NULL;
   r->finished_cb = NULL;
   r->cb_data = NULL;
   r->cb_data_free = NULL;
   r->cb_result = 0;
   r->plugin = NULL;
   r->pdata = NULL;

   return r;
}

#define CHECK_DELTA(Param)                      \
  if (e1->Param != e2->Param)                   \
    return EINA_TRUE;

EAPI Eina_Bool
ethumb_cmp(const Ethumb *e1, const Ethumb *e2)
{
   CHECK_DELTA(thumb_dir);
   CHECK_DELTA(category);
   CHECK_DELTA(tw);
   CHECK_DELTA(th);
   CHECK_DELTA(format);
   CHECK_DELTA(aspect);
   CHECK_DELTA(orientation);
   CHECK_DELTA(crop_x);
   CHECK_DELTA(crop_y);
   CHECK_DELTA(quality);
   CHECK_DELTA(compress);
   CHECK_DELTA(rw);
   CHECK_DELTA(rh);
   CHECK_DELTA(video.start);
   CHECK_DELTA(video.time);
   CHECK_DELTA(video.interval);
   CHECK_DELTA(video.ntimes);
   CHECK_DELTA(video.fps);
   CHECK_DELTA(document.page);

   return EINA_FALSE;
}

EAPI unsigned int
ethumb_length(EINA_UNUSED const void *key)
{
   return sizeof (Ethumb);
}

#define CMP_PARAM(Param)			\
  if (e1->Param != e2->Param)			\
    return e1->Param - e2->Param;

EAPI int
ethumb_key_cmp(const void *key1, EINA_UNUSED int key1_length,
               const void *key2, EINA_UNUSED int key2_length)
{
   const Ethumb *e1 = key1;
   const Ethumb *e2 = key2;

   CMP_PARAM(thumb_dir);
   CMP_PARAM(category);
   CMP_PARAM(tw);
   CMP_PARAM(th);
   CMP_PARAM(format);
   CMP_PARAM(aspect);
   CMP_PARAM(orientation);
   CMP_PARAM(crop_x);
   CMP_PARAM(crop_y);
   CMP_PARAM(quality);
   CMP_PARAM(compress);
   CMP_PARAM(rw);
   CMP_PARAM(rh);
   CMP_PARAM(video.start);
   CMP_PARAM(video.time);
   CMP_PARAM(video.interval);
   CMP_PARAM(video.ntimes);
   CMP_PARAM(video.fps);
   CMP_PARAM(document.page);
   CMP_PARAM(src_path);
   CMP_PARAM(src_key);

   return 0;
}

#undef CMP_PARAM

#define HASH_PARAM_I(Param) r ^= eina_hash_int32((unsigned int*) &e->Param, 0);
#ifdef EFL64
# define HASH_PARAM_P(Param) r ^= eina_hash_int64((unsigned long long int*) &e->Param, 0);
#else
# define HASH_PARAM_P(Param) r ^= eina_hash_int32((unsigned int*) &e->Param, 0);
#endif
#define HASH_PARAM_D(Param) r ^= eina_hash_int64((unsigned long long int*)&e->Param, 0);
#define HASH_PARAM_F(Param) r ^= eina_hash_int32((unsigned int*) &e->Param, 0);

EAPI int
ethumb_hash(const void *key, int key_length EINA_UNUSED)
{
   const Ethumb *e = key;
   int r = 0;

   HASH_PARAM_P(thumb_dir);
   HASH_PARAM_P(category);
   HASH_PARAM_I(tw);
   HASH_PARAM_I(th);
   HASH_PARAM_I(format);
   HASH_PARAM_I(aspect);
   HASH_PARAM_I(orientation);
   HASH_PARAM_F(crop_x);
   HASH_PARAM_F(crop_y);
   HASH_PARAM_I(quality);
   HASH_PARAM_I(compress);
   HASH_PARAM_P(src_path);
   HASH_PARAM_P(src_key);
   HASH_PARAM_I(rw);
   HASH_PARAM_I(rh);
   HASH_PARAM_D(video.start);
   HASH_PARAM_D(video.time);
   HASH_PARAM_D(video.interval);
   HASH_PARAM_I(video.ntimes);
   HASH_PARAM_I(video.fps);
   HASH_PARAM_I(document.page);

   return r;
}
