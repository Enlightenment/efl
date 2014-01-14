#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "evas_cserve2.h"
#include "evas_cserve2_slave.h"

#define LK(x) Eina_Lock x
#include "file/evas_module.h"
#include "Evas_Loader.h"

static Eina_Hash *loaders = NULL;
static Eina_List *modules = NULL;
static Eina_Prefix *pfx = NULL;

struct ext_loader_s
{
   unsigned int length;
   const char *extension;
   const char *loader;
};

#define MATCHING(Ext, Module)                   \
  { sizeof (Ext), Ext, Module }

/* The following list comes from evas_image_load.c
 * TODO: Don't duplicate, reuse common functions
 */

static const struct ext_loader_s map_loaders[] =
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
   MATCHING(".arw", "generic"),
   MATCHING(".cr2", "generic"),
   MATCHING(".crw", "generic"),
   MATCHING(".dcr", "generic"),
   MATCHING(".dng", "generic"),
   MATCHING(".erf", "generic"),
   MATCHING(".k25", "generic"),
   MATCHING(".kdc", "generic"),
   MATCHING(".mrw", "generic"),
   MATCHING(".nef", "generic"),
   MATCHING(".nrf", "generic"),
   MATCHING(".nrw", "generic"),
   MATCHING(".orf", "generic"),
   MATCHING(".pef", "generic"),
   MATCHING(".raf", "generic"),
   MATCHING(".raw", "generic"),
   MATCHING(".rw2", "generic"),
   MATCHING(".sr2", "generic"),
   MATCHING(".srf", "generic"),
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
  "bmp", "tga", "wbmp", "ico", "psd", "jp2k", "generic"
};


static Eina_Bool
command_read(int fd, Slave_Command *cmd, void **params)
{
   ssize_t ret;
   int ints[2], size, got = 0;
   char *buf;

   ret = read(fd, ints, sizeof(int) * 2);
   if (ret < (int)sizeof(int) * 2)
     return EINA_FALSE;

   EINA_SAFETY_ON_FALSE_RETURN_VAL((ints[0] > 0) && (ints[0] <= 0xFFFF), EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ints[1] >= 0) && (ints[1] < SLAVE_COMMAND_LAST), EINA_FALSE);

   size = ints[0];
   buf = malloc(size);
   if (!buf) return EINA_FALSE;

   do {
        ret = read(fd, buf + got, size - got);
        if (ret < 0)
          {
             /* EINTR means we were interrupted by a signal before anything
              * was sent, and if we are back here it means that signal was
              * not meant for us to die. Any other error here is fatal and
              * should result in the slave terminating.
              */
             if (errno == EINTR)
               continue;
             free(buf);
             return EINA_FALSE;
          }
        got += ret;
   } while (got < size);

   *cmd = ints[1];
   *params = buf;

   return EINA_TRUE;
}

static Eina_Bool
response_send(int fd, Slave_Command cmd, void *resp, int size)
{
   int sent = 0, ints[2];
   const char *data = resp;
   ssize_t ret;

   ints[0] = size;
   ints[1] = cmd;
   ret = write(fd, ints, sizeof(int) * 2);
   if (ret < 0)
     return EINA_FALSE;
   if (!size)
     return EINA_TRUE;
   do {
        ret = write(fd, data + sent, size - sent);
        if (ret < 0)
          {
             /* EINTR means we were interrupted by a signal before anything
              * was sent, and if we are back here it means that signal was
              * not meant for us to die. Any other error here is fatal and
              * should result in the slave terminating.
              */
             if (errno == EINTR)
               continue;
             return EINA_FALSE;
          }
        sent += ret;
   } while (sent < size);

   return EINA_TRUE;
}

static Eina_Bool
error_send(int fd, Error_Type err)
{
   return response_send(fd, ERROR, &err, sizeof(Error_Type));
}

static void *
_cserve2_shm_map(const char *name, size_t length, off_t offset)
{
   void *map;
   int fd;

   fd = shm_open(name, O_RDWR, S_IWUSR);
   if (fd == -1)
     return MAP_FAILED;

   map = mmap(NULL, length, PROT_WRITE, MAP_SHARED, fd, offset);

   close(fd);

   return map;
}

static void
_cserve2_shm_unmap(void *map, size_t length)
{
   munmap(map, length);
}

static void *
_image_file_open(Eina_File *fd, Eina_Stringshare *key, Evas_Image_Load_Opts *load_opts,
                 Evas_Module *module, Evas_Image_Property *property,
                 Evas_Image_Animated *animated, Evas_Image_Load_Func **pfuncs)
{
   int error = EVAS_LOAD_ERROR_NONE;
   void *loader_data = NULL;
   Evas_Image_Load_Func *funcs = NULL;

   *pfuncs = NULL;
   if (!evas_module_load(module))
     goto unload;

   funcs = module->functions;
   if (!funcs)
     goto unload;

   evas_module_use(module);
   memset(animated, 0, sizeof (*animated));

   loader_data = funcs->file_open(fd, key, load_opts, animated, &error);
   if (!loader_data || (error != EVAS_LOAD_ERROR_NONE))
     goto unload;

   if (!funcs->file_head(loader_data, property, &error))
     goto unload;

   if (error != EVAS_LOAD_ERROR_NONE)
     goto unload;

   *pfuncs = funcs;
   return loader_data;

unload:
   if (funcs && loader_data)
     funcs->file_close(loader_data);
   evas_module_unload(module);

   return NULL;
}

static Eina_Bool
_image_file_header(Eina_File *fd, Eina_Stringshare *key, Evas_Image_Load_Opts *load_opts,
                   Slave_Msg_Image_Opened *result, Evas_Module *module)
{
   Evas_Image_Property property;
   Evas_Image_Animated animated;
   Evas_Image_Load_Func *funcs;
   void *loader_data;

   memset(&property, 0, sizeof (property));
   loader_data = _image_file_open(fd, key, load_opts, module, &property, &animated, &funcs);
   if (!loader_data)
     return EINA_FALSE;

   memset(result, 0, sizeof (*result));
   result->w = property.w;
   result->h = property.h;
   //result->degree = opts->
   result->scale = property.scale;
   result->alpha = property.alpha;
   result->rotated = property.rotated;

   result->animated = animated.animated;
   if (result->animated)
     {
        result->frame_count = animated.frame_count;
        result->loop_count = animated.loop_count;
        result->loop_hint = animated.loop_hint;
     }
   result->has_loader_data = EINA_TRUE;

   // FIXME: We need to close as we this slave might not be used for data loading
   funcs->file_close(loader_data);
   return EINA_TRUE;
}

static Error_Type
image_open(const char *file, const char *key,
           Slave_Msg_Image_Opened *result, const char **use_loader,
           Evas_Image_Load_Opts *load_opts)
{
   Evas_Module *module;
   Eina_File *fd;
   const char *loader = NULL;
   const int filelen = strlen(file);
   unsigned int i;
   Error_Type ret = CSERVE2_NONE;
   Eina_Stringshare *skey = eina_stringshare_add(key);

   fd = eina_file_open(file, EINA_FALSE);
   if (!fd)
     {
        return CSERVE2_DOES_NOT_EXIST; // FIXME: maybe check errno
     }

   if (!*use_loader)
     goto try_extension;

   loader = *use_loader;
   module = evas_module_find_type(EVAS_MODULE_TYPE_IMAGE_LOADER, loader);
   if (module)
     {
        if (_image_file_header(fd, skey, load_opts, result, module))
          goto success;
     }

try_extension:
   loader = NULL;
   for (i = 0; i < sizeof(map_loaders) / sizeof(map_loaders[0]); i++)
     {
        int extlen = strlen(map_loaders[i].extension);
        if (extlen > filelen) continue;
        if (!strcasecmp(map_loaders[i].extension, file + filelen - extlen))
          {
             loader = map_loaders[i].loader;
             break;
          }
     }

   if (loader)
     {
        module = evas_module_find_type(EVAS_MODULE_TYPE_IMAGE_LOADER, loader);
        if (module && _image_file_header(fd, skey, load_opts, result, module))
          goto success;
        loader = NULL;
        module = NULL;
     }

   // Try all known modules
   for (i = 0; i < sizeof(loaders_name) / sizeof(loaders_name[0]); i++)
     {
        loader = loaders_name[i];
        module = evas_module_find_type(EVAS_MODULE_TYPE_IMAGE_LOADER, loader);
        if (!module) continue;
        if (_image_file_header(fd, skey, load_opts, result, module))
          goto success;
     }

   ret = CSERVE2_UNKNOWN_FORMAT;
   goto end;

success:
   ret = CSERVE2_NONE;
   *use_loader = loader;

   // FIXME: Do we really need to unload the module now?
   evas_module_unload(module);

end:
   eina_file_close(fd);
   eina_stringshare_del(skey);
   return ret;
}

static Error_Type
image_load(const char *file, const char *key, const char *shmfile,
           Slave_Msg_Image_Load *params, Slave_Msg_Image_Loaded *result,
           const char *loader)
{
   Evas_Module *module;
   Eina_File *fd;
   Evas_Image_Load_Func *funcs = NULL;
   Evas_Image_Load_Opts *opts = &params->opts;
   Evas_Image_Property property;
   Evas_Image_Animated animated;
   Error_Type ret = CSERVE2_GENERIC;
   void *loader_data = NULL;
   Eina_Bool ok;
   Eina_Stringshare *skey = NULL;
   int error;

   fd = eina_file_open(file, EINA_FALSE);
   if (!fd)
     {
        return CSERVE2_DOES_NOT_EXIST; // FIXME: maybe check errno
     }

   char *map = _cserve2_shm_map(shmfile, params->shm.mmap_size,
                                params->shm.mmap_offset);
   if (map == MAP_FAILED)
     {
        eina_file_close(fd);
        return CSERVE2_RESOURCE_ALLOCATION_FAILED;
     }

   module = evas_module_find_type(EVAS_MODULE_TYPE_IMAGE_LOADER, loader);
   if (!module)
     {
        printf("LOAD failed at %s:%d: no module found for loader %s\n",
               __FUNCTION__, __LINE__, loader);
        goto done;
     }

   memset(&property, 0, sizeof (property));
   property.w = params->opts.w; // Should we rather use params->w ?
   property.h = params->opts.h;

   skey = eina_stringshare_add(key);
   loader_data = _image_file_open(fd, skey, opts, module, &property, &animated, &funcs);
   if (!loader_data)
     {
        printf("LOAD failed at %s:%d: could not open image %s:%s\n",
               __FUNCTION__, __LINE__, file, skey);
        goto done;
     }

   if (params->shm.mmap_size < (int) (property.w * property.h * 4))
     {
        printf("LOAD failed at %s:%d: shm map is too small (%d) for this image (%ux%u)\n",
               __FUNCTION__, __LINE__,
               params->shm.mmap_size, property.w , property.h);
        goto done;
     }

   ok = funcs->file_data(loader_data, &property, map, &error);
   if (!ok || (error != EVAS_LOAD_ERROR_NONE))
     {
        printf("LOAD failed at %s:%d: file_data failed for loader %s: error %d\n",
               __FUNCTION__, __LINE__, loader, error);
        goto done;
     }

   result->w = property.w;
   result->h = property.h;
   result->alpha = property.alpha;

   if (property.alpha && property.premul)
     {
        result->alpha_sparse = evas_cserve2_image_premul_data((unsigned int *) map,
                                                              result->w * result->h);
     }

   //printf("LOAD successful: %dx%d alpha_sparse %d\n",
   //       result->w, result->h, result->alpha_sparse);

   ret = CSERVE2_NONE;

done:
   eina_file_close(fd);
   _cserve2_shm_unmap(map, params->shm.mmap_size);
   if (funcs)
     {
        if (loader_data)
          funcs->file_close(loader_data);
        evas_module_unload(module);
     }
   eina_stringshare_del(skey);

   return ret;
}

static void
handle_image_open(int wfd, void *params)
{
   Slave_Msg_Image_Open *msg = params;
   Slave_Msg_Image_Opened result;
   Evas_Image_Load_Opts load_opts;
   Error_Type err;
   const char *loader = NULL, *file, *key, *ptr;
   char *resp;
   size_t resp_size;

   memset(&load_opts, 0, sizeof(load_opts));
   load_opts.region.x = msg->lo.region.x;
   load_opts.region.y = msg->lo.region.y;
   load_opts.region.w = msg->lo.region.w;
   load_opts.region.h = msg->lo.region.h;
   load_opts.dpi = msg->lo.dpi;
   load_opts.w = msg->lo.w;
   load_opts.h = msg->lo.h;
   load_opts.scale_down_by = msg->lo.scale_down_by;
   load_opts.orientation = msg->lo.orientation;

   file = (const char *) (msg + 1);
   key = file + strlen(file) + 1;
   ptr = key + strlen(key) + 1;
   loader = ptr;

   memset(&result, 0, sizeof(result));
   if ((err = image_open(file, key, &result, &loader, &load_opts))
       != CSERVE2_NONE)
     {
        printf("OPEN failed at %s:%d\n", __FUNCTION__, __LINE__);
        error_send(wfd, err);
        return;
     }

   result.has_loader_data = EINA_TRUE;

   resp_size = sizeof(Slave_Msg_Image_Opened) + sizeof(int) + strlen(loader) + 1;
   resp = alloca(resp_size);
   memcpy(resp, &result, sizeof(Slave_Msg_Image_Opened));
   memcpy(resp + sizeof(Slave_Msg_Image_Opened), loader, strlen(loader) + 1);
   response_send(wfd, IMAGE_OPEN, resp, resp_size);
}

static void
handle_image_load(int wfd, void *params)
{
   Slave_Msg_Image_Load *load_args = params;
   Slave_Msg_Image_Loaded resp;
   Error_Type err;
   const char *shmfile;
   const char *file, *key, *loader;

   if (!load_args->has_loader_data)
     {
        printf("LOAD failed at %s:%d: no loader data\n", __FUNCTION__, __LINE__);
        error_send(wfd, CSERVE2_UNKNOWN_FORMAT);
        return;
     }

   memset(&resp, 0, sizeof(resp));

   shmfile = ((const char *)params) + sizeof(Slave_Msg_Image_Load);
   file = shmfile + strlen(shmfile) + 1;
   key = file + strlen(file) + 1;
   loader = key + strlen(key) + 1;
   if ((err = image_load(file, key, shmfile, load_args, &resp, loader))
       != CSERVE2_NONE)
     {
        printf("LOAD failed at %s:%d: load failed with error %d\n",
               __FUNCTION__, __LINE__, err);
        error_send(wfd, err);
        return;
     }

   response_send(wfd, IMAGE_LOAD, &resp, sizeof(resp));
}

int main(int c, char **v)
{
   int wfd, rfd;
   Slave_Command cmd;
   void *params = NULL;
   Eina_Module *m;
   Eina_Bool quit = EINA_FALSE;

   if (c < 3)
     return 1;

   eina_init();
   pfx =  eina_prefix_new(v[0], main,
                          "EVAS", "evas", "checkme",
                          PACKAGE_BIN_DIR,
                          PACKAGE_LIB_DIR,
                          PACKAGE_DATA_DIR,
                          PACKAGE_DATA_DIR);

   loaders = eina_hash_string_superfast_new(NULL);
   evas_module_init();

   wfd = atoi(v[1]);
   rfd = atoi(v[2]);

   while (!quit)
     {
        if (!command_read(rfd, &cmd, &params))
          {
             error_send(wfd, CSERVE2_INVALID_COMMAND);
             return 1;
          }
        switch (cmd)
          {
           case IMAGE_OPEN:
             handle_image_open(wfd, params);
             break;
           case IMAGE_LOAD:
             handle_image_load(wfd, params);
             break;
           case SLAVE_QUIT:
             quit = EINA_TRUE;
             break;
           default:
             error_send(wfd, CSERVE2_INVALID_COMMAND);
          }
     }

   evas_module_shutdown();
   eina_hash_free(loaders);

   EINA_LIST_FREE(modules, m)
      eina_module_free(m);

   eina_prefix_free(pfx);
   eina_shutdown();

   return 0;
}
