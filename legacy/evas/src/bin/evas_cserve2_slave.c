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

static Eina_Hash *loaders = NULL;
static Eina_List *modules = NULL;

struct ext_loader_s
{
   unsigned int length;
   const char *extension;
   const char *loader;
};

#define MATCHING(Ext, Module)                   \
  { sizeof (Ext), Ext, Module }

static const struct ext_loader_s map_loaders[] =
{ /* map extensions to loaders to use for good first-guess tries */
   MATCHING(".png", "png"),
   MATCHING(".jpg", "jpeg"),
   MATCHING(".jpeg", "jpeg"),
   MATCHING(".jfif", "jpeg"),
   MATCHING(".eet", "eet"),
   MATCHING(".edj", "eet"),
   MATCHING(".eap", "eet"),
   MATCHING(".edb", "edb"),
   MATCHING(".xpm", "xpm"),
   MATCHING(".tiff", "tiff"),
   MATCHING(".tif", "tiff"),
   MATCHING(".svg", "svg"),
   MATCHING(".svgz", "svg"),
   MATCHING(".svg.gz", "svg"),
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
   MATCHING(".psd", "psd")
};

static const char *loaders_name[] =
{ /* in order of most likely needed */
  "png", "jpeg", "eet", "xpm", "tiff", "gif", "svg", "webp", "pmaps", "bmp", "tga", "wbmp", "ico", "psd", "edb"
};

Eina_Bool
evas_cserve2_loader_register(Evas_Loader_Module_Api *api)
{
   eina_hash_direct_add(loaders, api->type, api);
   return EINA_TRUE;
}

#if defined(__CEGCC__) || defined(__MINGW32CE__)
# define EVAS_MODULE_NAME_IMAGE_LOADER "loader_%s.dll"
#elif _WIN32
# define EVAS_MODULE_NAME_IMAGE_LOADER "module.dll"
#else
# define EVAS_MODULE_NAME_IMAGE_LOADER "module.so"
#endif

static Evas_Loader_Module_Api *
loader_module_find(const char *type)
{
   Evas_Loader_Module_Api *l;
   Eina_Module *em;
   char buf[PATH_MAX];

   l = eina_hash_find(loaders, type);
   if (l) return l;

   /* FIXME: Look in every possible path, but what will those be? */
   snprintf(buf, sizeof(buf), PACKAGE_LIB_DIR "/evas/cserve2/loaders/%s/%s/%s",
            type, MODULE_ARCH, EVAS_MODULE_NAME_IMAGE_LOADER);

   em = eina_module_new(buf);
   if (!em) return NULL;

   if (!eina_module_load(em))
     {
        eina_module_free(em);
        return NULL;
     }

   l = eina_hash_find(loaders, type);
   if (l)
     {
        modules = eina_list_append(modules, em);
        return l;
     }

   eina_module_free(em);

   return NULL;
}

static Eina_Bool
command_read(int fd, Slave_Command *cmd, void **params)
{
   ssize_t ret;
   int ints[2], size, got = 0;
   char *buf;

   ret = read(fd, ints, sizeof(int) * 2);
   if (ret < (int)sizeof(int) * 2)
     return EINA_FALSE;

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

static Error_Type
image_open(const char *file, const char *key, Image_Load_Opts *opts, Slave_Msg_Image_Opened *result, const char **use_loader)
{
   Evas_Img_Load_Params ilp;
   Evas_Loader_Module_Api *api;
   const char *loader = NULL, *end;
   unsigned int i;
   int len;
   int err;

   memset(&ilp, 0, sizeof(ilp));

   if (opts)
     {
#define SETOPT(v) ilp.opts.v = opts->v
        SETOPT(w);
        SETOPT(h);
        SETOPT(rx);
        SETOPT(ry);
        SETOPT(rw);
        SETOPT(rh);
        SETOPT(scale_down_by);
        SETOPT(dpi);
        SETOPT(orientation);
#undef SETOPT
        ilp.has_opts = EINA_TRUE;
     }

   if (!*use_loader)
     goto try_extension;

   loader = *use_loader;
   api = loader_module_find(loader);
   if (!api)
     goto try_extension;

   if (api->head_load(&ilp, file, key, &err))
     goto done;

try_extension:
   len = strlen(file);
   end = file + len;
   for (i = 0; i < (sizeof (map_loaders) / sizeof(struct ext_loader_s)); i++)
     {
        int len2 = strlen(map_loaders[i].extension);
        if (len2 > len) continue;
        if (!strcasecmp(end - len2, map_loaders[i].extension))
          {
             loader = map_loaders[i].loader;
             break;
          }
     }

   if (!loader)
     goto try_all_known;

   api = loader_module_find(loader);
   if (!api)
     goto try_all_known;

   if (api->head_load(&ilp, file, key, &err))
     goto done;

try_all_known:
   for (i = 0; i < (sizeof(loaders_name) / sizeof(loaders_name[0])); i++)
     {
        loader = loaders_name[i];
        api = loader_module_find(loader);
        if (!api)
          continue;
        if (api->head_load(&ilp, file, key, &err))
          goto done;
     }

   /* find every module available and try them, even if we don't know they
    * exist. That will be our generic loader */

   return err;

done:
   *use_loader = loader;

   result->w = ilp.w;
   result->h = ilp.h;
   if ((result->rotated = ilp.rotated))
     {
        result->degree = ilp.degree;
     }
   if ((result->animated = ilp.animated))
     {
        result->frame_count = ilp.frame_count;
        result->loop_count = ilp.loop_count;
        result->loop_hint = ilp.loop_hint;
     }
   result->scale = ilp.scale;
   result->alpha = ilp.alpha;
   return CSERVE2_NONE;
}

static Error_Type
image_load(const char *file, const char *key, const char *shmfile, Slave_Msg_Image_Load *params, Slave_Msg_Image_Loaded *result, const char *loader)
{
   Evas_Img_Load_Params ilp;
   Evas_Loader_Module_Api *api;
   int err;
   Error_Type ret = CSERVE2_NONE;
   char *map = _cserve2_shm_map(shmfile, params->shm.mmap_size,
                                params->shm.mmap_offset);
   if (map == MAP_FAILED)
     return CSERVE2_RESOURCE_ALLOCATION_FAILED;

   memset(&ilp, 0, sizeof(ilp));

   api = loader_module_find(loader);
   if (!api)
     {
        ret = CSERVE2_GENERIC;
        goto done;
     }

   ilp.w = params->w;
   ilp.h = params->h;
   ilp.alpha = params->alpha;
#define SETOPT(v) ilp.opts.v = params->opts.v
     SETOPT(w);
     SETOPT(h);
     SETOPT(rx);
     SETOPT(ry);
     SETOPT(rw);
     SETOPT(rh);
     SETOPT(scale_down_by);
     SETOPT(dpi);
     SETOPT(orientation);
#undef SETOPT

   ilp.buffer = map + params->shm.image_offset;
   if (!api->data_load(&ilp, file, key, &err))
     ret = err;

   result->alpha_sparse = ilp.alpha_sparse;

done:
   _cserve2_shm_unmap(map, params->shm.mmap_size);

   return ret;
}

static void
handle_image_open(int wfd, void *params)
{
   Slave_Msg_Image_Open *p;
   Slave_Msg_Image_Opened result;
   Image_Load_Opts *load_opts = NULL;
   Error_Type err;
   const char *loader = NULL, *file, *key, *ptr;
   char *resp;
   size_t resp_size;

   p = params;
   file = (const char *)(p + sizeof(Slave_Msg_Image_Open));
   key = file + strlen(file) + 1;
   ptr = key + strlen(key) + 1;
   if (p->has_opts)
     {
        load_opts = (Image_Load_Opts *)ptr;
        ptr += sizeof(Image_Load_Opts);
     }
   if (p->has_loader_data)
     loader = ptr;

   memset(&result, 0, sizeof(result));
   if ((err = image_open(file, key, load_opts, &result, &loader))
       != CSERVE2_NONE)
     {
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

   loaders = eina_hash_string_superfast_new(NULL);

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

   eina_hash_free(loaders);

   EINA_LIST_FREE(modules, m)
      eina_module_free(m);

   eina_shutdown();

   return 0;
}
