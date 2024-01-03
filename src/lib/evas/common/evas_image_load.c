#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "evas_common_private.h"
#include "evas_private.h"
//#include "evas_cs.h"

struct ext_loader_s
{
   unsigned int length;
   const char *extension;
   const char *loader;
};

#define MATCHING(Ext, Module)                   \
{ sizeof (Ext), Ext, Module }

static const struct ext_loader_s loaders[] =
{ /* map extensions to loaders to use for good first-guess tries */
   MATCHING(".png", "png"),

   MATCHING(".jpg", "jpeg"),
   MATCHING(".jpeg", "jpeg"),
   MATCHING(".jfif", "jpeg"),

   MATCHING(".j2k", "jp2k"),
   MATCHING(".jp2", "jp2k"),
   MATCHING(".jpx", "jp2k"),
   MATCHING(".jpf", "jp2k"),

   MATCHING(".eet", "eet"),
   MATCHING(".edj", "eet"),
   MATCHING(".eap", "eet"),

   MATCHING(".xpm", "xpm"),

   MATCHING(".tiff", "tiff"),
   MATCHING(".tif", "tiff"),

   MATCHING(".gif", "gif"),

   MATCHING(".pbm", "pmaps"),
   MATCHING(".pgm", "pmaps"),
   MATCHING(".ppm", "pmaps"),
   MATCHING(".pnm", "pmaps"),

   MATCHING(".bmp", "bmp"),

   MATCHING(".tga", "tga"),

   MATCHING(".wbmp", "wbmp"),

   MATCHING(".webp", "webp"),

   MATCHING(".ico", "ico"),
   MATCHING(".cur", "ico"),

   MATCHING(".psd", "psd"),

   MATCHING(".tgv", "tgv"),

   MATCHING(".dds", "dds"),

   MATCHING(".jxl", "jxl"),

   MATCHING(".qoi", "qoi"),

   MATCHING(".avif", "avif"),
   MATCHING(".avifs", "avif"),

   MATCHING(".heif", "heif"),
   MATCHING(".heic", "heif"),
   // MATCHING(".avif", "heif"),

   /* xcf - gefenric */
   MATCHING(".xcf", "generic"),
   MATCHING(".xcf.gz", "generic"),
   /* docs */
   MATCHING(".doc", "generic"),
   MATCHING(".docx", "generic"),
   MATCHING(".odp", "generic"),
   MATCHING(".ods", "generic"),
   MATCHING(".odt", "generic"),
   MATCHING(".pdf", "generic"),
   MATCHING(".ps", "generic"),
   MATCHING(".ppt", "generic"),
   MATCHING(".pptx", "generic"),
   MATCHING(".rtf", "generic"),
   MATCHING(".xls", "generic"),
   MATCHING(".xlsx", "generic"),
   /* svg - generic */
   MATCHING(".svg", "generic"),
   MATCHING(".svgz", "generic"),
   MATCHING(".svg.gz", "generic"),
   /* RAW */
   MATCHING(".3fr", "generic"),
   MATCHING(".ari", "generic"),
   MATCHING(".arw", "generic"),
   MATCHING(".bay", "generic"),
   MATCHING(".braw", "generic"),
   MATCHING(".crw", "generic"),
   MATCHING(".cr2", "generic"),
   MATCHING(".cr3", "generic"),
   MATCHING(".cap", "generic"),
   MATCHING(".data", "generic"),
   MATCHING(".dcr", "generic"),
   MATCHING(".dcs", "generic"),
   MATCHING(".dng", "generic"),
   MATCHING(".drf", "generic"),
   MATCHING(".eip", "generic"),
   MATCHING(".erf", "generic"),
   MATCHING(".fff", "generic"),
   MATCHING(".gpr", "generic"),
   MATCHING(".iiq", "generic"),
   MATCHING(".k25", "generic"),
   MATCHING(".kdc", "generic"),
   MATCHING(".mdc", "generic"),
   MATCHING(".mef", "generic"),
   MATCHING(".mos", "generic"),
   MATCHING(".mrw", "generic"),
   MATCHING(".nef", "generic"),
   MATCHING(".nrw", "generic"),
   MATCHING(".obm", "generic"),
   MATCHING(".orf", "generic"),
   MATCHING(".pef", "generic"),
   MATCHING(".ptx", "generic"),
   MATCHING(".pxn", "generic"),
   MATCHING(".r3d", "generic"),
   MATCHING(".raf", "generic"),
   MATCHING(".raw", "generic"),
   MATCHING(".rw1", "generic"),
   MATCHING(".rw2", "generic"),
   MATCHING(".rwz", "generic"),
   MATCHING(".sr2", "generic"),
   MATCHING(".srf", "generic"),
   MATCHING(".srw", "generic"),
   MATCHING(".tif", "generic"),
   MATCHING(".x3f", "generic"),
   /* video */
   MATCHING(".264", "generic"),
   MATCHING(".3g2", "generic"),
   MATCHING(".3gp", "generic"),
   MATCHING(".3gp2", "generic"),
   MATCHING(".3gpp", "generic"),
   MATCHING(".3gpp2", "generic"),
   MATCHING(".3p2", "generic"),
   MATCHING(".asf", "generic"),
   MATCHING(".avi", "generic"),
   MATCHING(".bdm", "generic"),
   MATCHING(".bdmv", "generic"),
   MATCHING(".clpi", "generic"),
   MATCHING(".cpi", "generic"),
   MATCHING(".dv", "generic"),
   MATCHING(".fla", "generic"),
   MATCHING(".flv", "generic"),
   MATCHING(".m1v", "generic"),
   MATCHING(".m2t", "generic"),
   MATCHING(".m2v", "generic"),
   MATCHING(".m4v", "generic"),
   MATCHING(".mkv", "generic"),
   MATCHING(".mov", "generic"),
   MATCHING(".mp2", "generic"),
   MATCHING(".mp2ts", "generic"),
   MATCHING(".mp4", "generic"),
   MATCHING(".mpe", "generic"),
   MATCHING(".mpeg", "generic"),
   MATCHING(".mpg", "generic"),
   MATCHING(".mpl", "generic"),
   MATCHING(".mpls", "generic"),
   MATCHING(".mts", "generic"),
   MATCHING(".mxf", "generic"),
   MATCHING(".nut", "generic"),
   MATCHING(".nuv", "generic"),
   MATCHING(".ogg", "generic"),
   MATCHING(".ogm", "generic"),
   MATCHING(".ogv", "generic"),
   MATCHING(".qt", "generic"),
   MATCHING(".rm", "generic"),
   MATCHING(".rmj", "generic"),
   MATCHING(".rmm", "generic"),
   MATCHING(".rms", "generic"),
   MATCHING(".rmvb", "generic"),
   MATCHING(".rmx", "generic"),
   MATCHING(".rv", "generic"),
   MATCHING(".swf", "generic"),
   MATCHING(".ts", "generic"),
   MATCHING(".weba", "generic"),
   MATCHING(".webm", "generic"),
   MATCHING(".wmv", "generic")
};

static const char *loaders_name[] =
{ /* in order of most likely needed */
  "png", "jpeg", "eet", "xpm", "tiff", "gif", "svg", "webp", "pmaps",
  "bmp", "tga", "wbmp", "ico", "psd", "jp2k", "dds", "jxl", "qoi", "avif",
  "heif", "generic"
};

struct evas_image_foreach_loader_data
{
   Image_Entry *ie;
   int *error;
   Evas_Module *em;
};

static Eina_Bool
_evas_image_file_header(Evas_Module *em, Image_Entry *ie, int *error)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;
   Eina_Bool r = EINA_TRUE;

   if (!evas_module_load(em)) goto load_error;
   evas_image_load_func = em->functions;
   evas_module_use(em);
   *error = EVAS_LOAD_ERROR_NONE;
   if (evas_image_load_func && evas_image_load_func->version == EVAS_IMAGE_LOAD_VERSION)
     {
        Evas_Image_Property property;
        const char *file;

        if (!ie->f)
          {
             ie->f = eina_file_open(ie->file, EINA_FALSE);
             ie->flags.given_mmap = EINA_FALSE;
             file = ie->file;
          }
        else file = eina_file_filename_get(ie->f);

        if (!ie->f)
          {
             *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
             goto load_error;
          }

        ie->loader_data = evas_image_load_func->file_open(ie->f, ie->key,
                                                          &ie->load_opts,
                                                          &ie->animated,
                                                          error);
        if (!ie->loader_data) goto load_error;

        memset(&property, 0, sizeof (property));
        if (evas_image_load_func->file_head(ie->loader_data, &property,
                                            error) &&
            (*error == EVAS_LOAD_ERROR_NONE))
          {
             DBG("loaded file head using module '%s' (%p): %s",
                 em->definition->name, em, file);

             ie->w = property.info.w;
             ie->h = property.info.h;
             ie->borders.l = property.info.borders.l;
             ie->borders.r = property.info.borders.r;
             ie->borders.t = property.info.borders.t;
             ie->borders.b = property.info.borders.b;
             ie->scale = property.info.scale;
             ie->flags.alpha = property.info.alpha;
             ie->need_data = property.need_data;
             if (property.info.cspaces) ie->cspaces = property.info.cspaces;
             ie->flags.rotated = property.info.rotated;
             ie->flags.flipped = property.info.flipped;
             r = EINA_FALSE;
          }
        else
          {
             evas_image_load_func->file_close(ie->loader_data);
             ie->loader_data = NULL;
             evas_module_unload(em);
             INF("failed to load file head using module '%s' (%p): "
                 "%s (%s)",
                 em->definition->name, em, file, evas_load_error_str(*error));
          }
     }
   else
     {
load_error:
        evas_module_unload(em);
        WRN("failed to load module '%s'.", em->definition->name);
     }
   return r;
}

static Eina_Bool
_evas_image_foreach_loader(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   struct evas_image_foreach_loader_data *d = fdata;
   Evas_Module *em = data;
   Image_Entry *ie = d->ie;
   Eina_Bool r;

   r = _evas_image_file_header(em, ie, d->error);
   if (!r) d->em = em;
   return r;
}

EVAS_API int
evas_common_load_rgba_image_module_from_file(Image_Entry *ie)
{
   const char           *loader = NULL, *end;
   const char           *file;
   Evas_Module          *em;
   unsigned int          i;
   int                   len, ret = EVAS_LOAD_ERROR_NONE;
   struct evas_image_foreach_loader_data fdata;
   Eina_Bool             skip;

   skip = ie->load_opts.skip_head;
   if (ie->f)
     {
        len = strlen(eina_file_filename_get(ie->f));
        end = eina_file_filename_get(ie->f) + len;
        file = eina_file_filename_get(ie->f);
     }
   else
     {
        if (!skip)
          {
             struct stat st;

             if (stat(ie->file, &st) != 0 || S_ISDIR(st.st_mode))
               {
                  DBG("trying to open directory '%s' !", ie->file);
                  return EVAS_LOAD_ERROR_DOES_NOT_EXIST;
               }
          }
        len = strlen(ie->file);
        end = ie->file + len;
        file = ie->file;
     }

   for (i = 0; i < (sizeof (loaders) / sizeof(struct ext_loader_s)); i++)
     {
        int len2 = strlen(loaders[i].extension);
        if (len2 > len) continue;
        if (!strcasecmp(end - len2, loaders[i].extension))
          {
             loader = loaders[i].loader;
             DBG("known loader '%s' handles extension '%s' of file '%s'",
                 loader, end - len2, file);
             break;
          }
     }

   if (loader)
     {
        em = evas_module_find_type(EVAS_MODULE_TYPE_IMAGE_LOADER, loader);
        if (em)
          {
             if (!((Evas_Image_Load_Func *)em->functions)->threadable)
               skip = EINA_FALSE;
             DBG("found image loader '%s' (%p)", loader, em);
             if (!skip)
               {
                  if (!_evas_image_file_header(em, ie, &ret)) goto end;
               }
          }
        else INF("image loader '%s' is not enabled or missing!", loader);
        if (skip) goto end;
     }

   fdata.ie = ie;
   fdata.error = &ret;
   fdata.em = NULL;
   ret = EVAS_LOAD_ERROR_NONE;
   evas_module_foreach_image_loader(_evas_image_foreach_loader, &fdata);
   em = fdata.em;
   if (em) goto end;

   /* This is our last chance, try all known image loader. */
   /* FIXME: We could use eina recursive module search ability. */
   for (i = 0; i < sizeof (loaders_name) / sizeof (char *); i++)
     {
        em = evas_module_find_type(EVAS_MODULE_TYPE_IMAGE_LOADER, loaders_name[i]);
        if (em)
          {
             if (!ie->load_opts.skip_head)
               {
                  if (!_evas_image_file_header(em, ie, &ret)) goto end;
               }
          }
        else
          DBG("could not find module '%s'", loaders_name[i]);
     }
   INF("exhausted all means to load image '%s'", file);
   return EVAS_LOAD_ERROR_UNKNOWN_FORMAT;

end:
   DBG("loader '%s' used for file %s",
       (em && em->definition && em->definition->name) ?
       em->definition->name : "<UNKNOWN>",
       file);

   if (em)
     {
        ie->info.module = em;
        ie->info.loader = em->functions;
        evas_module_ref(em);
     }
   else
     {
        ie->info.module = NULL;
        ie->info.loader = NULL;
     }
   return ret;
}

static void
_timestamp_build(Image_Timestamp *tstamp, struct stat *st)
{
   tstamp->mtime = st->st_mtime;
   tstamp->size = st->st_size;
   tstamp->ino = st->st_ino;
#ifdef _STAT_VER_LINUX
   tstamp->mtime_nsec = (unsigned long int)st->st_mtim.tv_nsec;
#endif
}

EVAS_API int
evas_common_load_rgba_image_data_from_file(Image_Entry *ie)
{
   void *pixels;
   Evas_Image_Load_Func *evas_image_load_func = NULL;
   Evas_Image_Property property;
   int ret = EVAS_LOAD_ERROR_NONE;
   struct stat st;
   unsigned int i;

   if ((ie->flags.loaded) && (!ie->animated.animated)) return EVAS_LOAD_ERROR_GENERIC;

   if (!ie->info.module)
     {
        ie->load_failed = 1;
        return EVAS_LOAD_ERROR_GENERIC;
     }

   evas_image_load_func = ie->info.loader;
   evas_module_use(ie->info.module);

   if (!ie->loader_data)
     {
        Evas_Module *em = ie->info.module;

        if (_evas_image_file_header(em, ie, &ret))
          {
             em = NULL;
             if (!ie->load_opts.skip_head)
               {
                  for (i = 0; i < sizeof(loaders_name) / sizeof (char *); i++)
                    {
                       em = evas_module_find_type
                         (EVAS_MODULE_TYPE_IMAGE_LOADER, loaders_name[i]);
                       if (em)
                         {
                            if (!_evas_image_file_header(em, ie, &ret))
                              goto end;
                         }
                       else DBG("could not find module '%s'", loaders_name[i]);
                       em = NULL;
                    }
               }
          }
end:
        if (ie->info.module != em)
          {
             if (em) evas_module_ref(em);
             evas_module_unref(ie->info.module);
             ie->info.module = em;
          }
     }
   if ((!ie->f) || (!ie->info.module) || (!ie->loader_data))
     {
        ie->load_failed = 1;
        return EVAS_LOAD_ERROR_DOES_NOT_EXIST;
     }

   if ((ie->file) && (stat(ie->file, &st) == 0))
     _timestamp_build(&(ie->tstamp), &st);

   memset(&property, 0, sizeof (property));
   property.info.w = ie->w;
   property.info.h = ie->h;
   property.info.scale = ie->scale;
   property.info.rotated = ie->flags.rotated;
   property.info.flipped = ie->flags.flipped;
   property.info.premul = EINA_FALSE;
   property.info.alpha_sparse = EINA_FALSE;
   property.info.cspace = ie->space;

   evas_cache_image_surface_alloc(ie, ie->w, ie->h);
   property.info.borders.l = ie->borders.l;
   property.info.borders.r = ie->borders.r;
   property.info.borders.t = ie->borders.t;
   property.info.borders.b = ie->borders.b;

   pixels = evas_cache_image_pixels(ie);
   if (!pixels)
     {
        ie->load_failed = 1;
        return EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
     }

   if (ie->need_data)
     {
        evas_image_load_func->file_head_with_data(ie->loader_data, &property, pixels, &ret);
        memcpy(&ie->content, &property.content, sizeof (Eina_Rectangle));
        ie->stretch.horizontal.region = property.stretch.horizontal.region;
        ie->stretch.vertical.region = property.stretch.vertical.region;
     }
   else
     {
        evas_image_load_func->file_data(ie->loader_data, &property, pixels, &ret);
     }

   ie->flags.alpha_sparse = property.info.alpha_sparse;

   if (property.info.premul) evas_common_image_premul(ie);

   return ret;
}

EVAS_API double
evas_common_load_rgba_image_frame_duration_from_file(Image_Entry *ie, const int start, const int frame_num)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;

   if (!ie->info.module) return -1;

   evas_image_load_func = ie->info.loader;
   evas_module_use(ie->info.module);
   if (evas_image_load_func->frame_duration)
     {
        if (!ie->f) return -1;
        return evas_image_load_func->frame_duration(ie->loader_data, start, frame_num);
     }
   return -1;
}

EVAS_API Eina_Bool
evas_common_extension_can_load_get(const char *file)
{
   unsigned int length;
   unsigned int i;

   length = eina_stringshare_strlen(file) + 1;

   for (i = 0; i < sizeof (loaders) / sizeof (struct ext_loader_s); ++i)
     {
        if (loaders[i].length > length) continue;

        if (!strcasecmp(loaders[i].extension, file + length - loaders[i].length))
          {
             if ((file[length - loaders[i].length] != '/') ||
                 (length == loaders[i].length))
               return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}
