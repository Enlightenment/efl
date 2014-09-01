#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;
   const char *key;
   Evas_Image_Load_Opts *opts;
};

static Eina_Bool
illegal_char(const char *str)
{
   const char *p;

   for (p = str; *p; p++)
     {
        if (*p <  '-')  return EINA_TRUE;
        if (*p == '/')  return EINA_TRUE;
        if (*p == ';')  return EINA_TRUE;
        if (*p == ':')  return EINA_TRUE;
        if (*p == '<')  return EINA_TRUE;
        if (*p == '>')  return EINA_TRUE;
        if (*p == '?')  return EINA_TRUE;
        if (*p == '[')  return EINA_TRUE;
        if (*p == '\\') return EINA_TRUE;
        if (*p == ']')  return EINA_TRUE;
        if (*p == '`')  return EINA_TRUE;
        if (*p == '{')  return EINA_TRUE;
        if (*p == '|')  return EINA_TRUE;
        if (*p == '}')  return EINA_TRUE;
        if (*p == '~')  return EINA_TRUE;
        if (*p == 0x7f) return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
escape_copy(const char *src, char *dst)
{
   const char *s;
   char *d;

   for (s = src, d = dst; *s; s++, d++)
     {
        // FIXME: escape tab, newline linefeed and friends
        if ((*s == ' ')  ||
            (*s == '!')  ||
            (*s == '"')  ||
            (*s == '#')  ||
            (*s == '$')  ||
            (*s == '%')  ||
            (*s == '&')  ||
            (*s == '\'') ||
            (*s == '(')  ||
            (*s == ')')  ||
            (*s == '*')  ||
            (*s == '[')  ||
            (*s == '\\') ||
            (*s == ']')  ||
            (*s == '`')  ||
            (*s == '{')  ||
            (*s == '|')  ||
            (*s == '}')  ||
            (*s == '~'))
          {
             *d = '\\';
             d++;
          }
        *d = *s;
     }
   *d = 0;
}

static void
dotcat(char *dest, const char *src)
{
   int len = strlen(dest);
   const char *s;
   char *d;

   for (d = dest + len, s = src; *s; d++, s++) *d = tolower(*s);
   *d = 0;
}

static Eina_Bool
_load(Eina_File *ef, const char *key,
      Evas_Image_Property *prop,
      Evas_Image_Load_Opts *opts,
      void *pixels,
      int *error, Eina_Bool get_data)
{
   Eina_Bool res = EINA_FALSE;
   int w = 0, h = 0, alpha = 0;
   const char *dot1 = NULL, *dot2 = NULL, *end, *p;
   char *cmd = NULL, decoders[3][128], buf[4096];
   char *loader = "/evas/utils/evas_image_loader";
   char *img_loader = NULL;
   const char *libdir;
   // eg $libdir/evas/generic_loaders
   int cmd_len, len, decoders_num = 0, try_count = 0;
   int read_data = 0;
   char *tmpfname = NULL, *shmfname = NULL;
   DATA32 *body;
   FILE *f = NULL;

   libdir = _evas_module_libdir_get();
   cmd_len = strlen(libdir);
   cmd_len += strlen(loader);
   img_loader = alloca(cmd_len + 1);
   strcpy(img_loader, libdir);
   strcat(img_loader, loader);

   // params excluding file, key and loadopts
   cmd_len += 1024;
   cmd_len += strlen(eina_file_filename_get(ef)) * 2;
   if (key) cmd_len += strlen(key) * 2;
   cmd = alloca(cmd_len + 1);

   len = strlen(eina_file_filename_get(ef));
   if (len < 1)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   end = eina_file_filename_get(ef) + len;
   for (p = end - 1; p >= eina_file_filename_get(ef); p--)
     {
        if      ((!dot1) && (*p == '.')) dot1 = p;
        else if ((!dot2) && (*p == '.')) dot2 = p;
        else if ((dot1) && (dot2)) break;
    }
   if (dot2)
     {
        // double extn not too long
        if (((end - dot2) <= 10) && (!illegal_char(dot2)))
          {
             strncpy(&(decoders[decoders_num][0]), img_loader, 127);
             decoders[decoders_num][127] = 0;
             dotcat(&(decoders[decoders_num][0]), dot2);
             decoders_num++;
          }
        // single extn not too long
        if (((end - dot1) <= 5) && (!illegal_char(dot1)))
          {
             strncpy(&(decoders[decoders_num][0]), img_loader, 127);
             decoders[decoders_num][127] = 0;
             dotcat(&(decoders[decoders_num][0]), dot1);
             decoders_num++;
          }
        strncpy(decoders[decoders_num], img_loader, 127);
        decoders[decoders_num][127] = 0;
        decoders_num++;
     }
   else if (dot1)
     {
        // single extn not too long
        if (((end - dot1) <= 5) && (!illegal_char(dot1)))
          {
             strcpy(&(decoders[decoders_num][0]), img_loader);
             dotcat(&(decoders[decoders_num][0]), dot1);
             decoders_num++;
          }
        strncpy(decoders[decoders_num], img_loader, 127);
        decoders[decoders_num][127] = 0;
        decoders_num++;
     }
   else
     {
        strncpy(decoders[decoders_num], img_loader, 127);
        decoders[decoders_num][127] = 0;
        decoders_num++;
     }

   for (try_count = 0; try_count < decoders_num; try_count++)
     {
        // FIXME: strcats could be more efficient, not that it matters much
        // here as we are about to build a cmd to exec via a shell that
        // will interpret shell stuff and path hunt that will then exec the
        // program itself that will dynamically link that will again
        // parse the arguments and finally do something...
        if (access(decoders[try_count], X_OK)) continue;

        strcpy(cmd, decoders[try_count]);
        strcat(cmd, " ");
        // filename first arg
        len = strlen(cmd);
        escape_copy(eina_file_filename_get(ef), cmd + len);
        if (!get_data)
          {
             strcat(cmd, " -head ");
          }
        if (key)
          {
             strcat(cmd, " -key ");
             len = strlen(cmd);
             escape_copy(key, cmd + len);
          }
        if (opts->scale_down_by > 1)
          {
             strcat(cmd, " -opt-scale-down-by ");
             snprintf(buf, sizeof(buf), "%i", opts->scale_down_by);
             strcat(cmd, buf);
          }
        if (opts->dpi > 0.0)
          {
             strcat(cmd, " -opt-dpi ");
             snprintf(buf, sizeof(buf), "%i", (int)(opts->dpi * 1000.0));
             strcat(cmd, buf);
          }
        if ((opts->w > 0) &&
            (opts->h > 0))
          {
             strcat(cmd, " -opt-size ");
             snprintf(buf, sizeof(buf), "%i %i", opts->w, opts->h);
             strcat(cmd, buf);
         }
        f = popen(cmd, "r");
        if (f) break;
     }
   if (!f)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   while (fgets(buf, sizeof(buf), f))
     {
        len = strlen(buf);
        if (len > 0)
          {
             if (buf[len - 1] == '\n') buf[len - 1] = 0;
             if (!strncmp(buf, "size ", 5))
               {
                  int tw = 0, th = 0;

                  len = sscanf(buf, "%*s %i %i", &tw, &th);
                  if (len == 2)
                    {
                       if ((tw > 0) && (th > 0))
                         {
                            w = tw;
                            h = th;
                         }
                    }
               }
             else if (!strncmp(buf, "alpha ", 6))
               {
                  int ta;

                  len = sscanf(buf, "%*s %i", &ta);
                  if (len == 1)
                    {
                       alpha = ta;
                    }
               }
             else if (!strncmp(buf, "tmpfile ", 8))
               {
                  tmpfname = buf + 8;
                  goto getdata;
               }
#ifdef HAVE_SHM_OPEN
             else if (!strncmp(buf, "shmfile ", 8))
               {
                  shmfname = buf + 8;
                  goto getdata;
               }
#endif
             else if (!strncmp(buf, "data", 4))
               {
                  read_data = 1;
                  goto getdata;
               }
             else if (!strncmp(buf, "done", 4))
               {
                  read_data = 2;
                  goto getdata;
               }
          }
     }
getdata:
   if ((!read_data) && (!tmpfname) && (!shmfname))
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	goto on_error;
     }
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto on_error;
     }

   if (!get_data)
     {
        if (alpha) prop->alpha = 1;
        prop->w = w;
        prop->h = h;
     }
   else
     {
        if ((int)prop->w != w ||
            (int)prop->h != h)
          {
             *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             goto on_error;
          }
        body = pixels;

        if ((tmpfname) || (shmfname))
          {
             int fd = -1;

             // open
             if (tmpfname)
                fd = open(tmpfname, O_RDONLY, S_IRUSR);
#ifdef HAVE_SHM_OPEN
             else if (shmfname)
                fd = shm_open(shmfname, O_RDONLY, S_IRUSR);
#endif
             if (fd >= 0)
               {
                  void *addr;

                  eina_mmap_safety_enabled_set(EINA_TRUE);

                  // mmap
                  addr = mmap(NULL, w * h * sizeof(DATA32),
                              PROT_READ, MAP_SHARED, fd, 0);
                  if (addr != MAP_FAILED)
                    {
                       memcpy(body, addr, w * h * sizeof(DATA32));
                       munmap(addr, w * h * sizeof(DATA32));
                    }
                  // close
                  if (tmpfname)
                    {
                       close(fd);
                       unlink(tmpfname);
                    }
#ifdef HAVE_SHM_OPEN
                  else if (shmfname)
                    {
                       close(fd);
                       shm_unlink(shmfname);
                    }
#endif
               }
             else
               {
                  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                  goto on_error;
               }
          }
        else if (read_data)
          {
             if (fread(body, w * h * sizeof(DATA32), 1, f) != 1)
               {
                  *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
                  goto on_error;
               }
          }
     }

   res = EINA_TRUE;
   *error = EVAS_LOAD_ERROR_NONE;

 on_error:
   if (f) pclose(f);
   return res;
}

static void *
evas_image_load_file_open_generic(Eina_File *f, Eina_Stringshare *key,
				  Evas_Image_Load_Opts *opts,
				  Evas_Image_Animated *animated EINA_UNUSED,
				  int *error)
{
   Evas_Loader_Internal *loader;

   loader = calloc(1, sizeof (Evas_Loader_Internal));
   if (!loader)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   loader->f = f;
   loader->key = eina_stringshare_ref(key);
   loader->opts = opts;
   return loader;
}

static void
evas_image_load_file_close_generic(void *loader_data)
{
   Evas_Loader_Internal *loader = loader_data;

   eina_stringshare_del(loader->key);
   free(loader);
}

static Eina_Bool
evas_image_load_file_head_generic(void *loader_data,
                                  Evas_Image_Property *prop,
                                  int *error)
{
   Evas_Loader_Internal *loader = loader_data;

   return _load(loader->f, loader->key, prop, loader->opts, NULL, error, EINA_FALSE);
}

static Eina_Bool
evas_image_load_file_data_generic(void *loader_data,
                                  Evas_Image_Property *prop,
                                  void *pixels,
                                  int *error)
{
   Evas_Loader_Internal *loader = loader_data;

   return _load(loader->f, loader->key, prop, loader->opts, pixels, error, EINA_TRUE);
}

Evas_Image_Load_Func evas_image_load_generic_func =
{
  evas_image_load_file_open_generic,
  evas_image_load_file_close_generic,
  evas_image_load_file_head_generic,
  evas_image_load_file_data_generic,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_generic_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "generic",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, generic);

#ifndef EVAS_STATIC_BUILD_GENERIC
EVAS_EINA_MODULE_DEFINE(image_loader, generic);
#endif
