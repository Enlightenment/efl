#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#include "evas_common_private.h"
#include "evas_private.h"


#ifndef EVAS_MODULE_NO_ENGINES
#define EVAS_MODULE_NO_ENGINES 0
#endif

#ifndef EVAS_MODULE_NO_IMAGE_LOADERS
#define EVAS_MODULE_NO_IMAGE_LOADERS 0
#endif

#ifndef EVAS_MODULE_NO_IMAGE_SAVERS
#define EVAS_MODULE_NO_IMAGE_SAVERS 0
#endif

#ifndef EVAS_MODULE_NO_VG_LOADERS
#define EVAS_MODULE_NO_VG_LOADERS 0
#endif

#ifndef EVAS_MODULE_NO_VG_SAVERS
#define EVAS_MODULE_NO_VG_SAVERS 0
#endif

typedef struct _Evas_Module_Task Evas_Module_Task;
struct _Evas_Module_Task
{
   Eina_Bool (*cancelled)(void *data);
   void *data;
};

static Eina_TLS task = 0;

EAPI Eina_Bool
evas_module_task_cancelled(void)
{
   Evas_Module_Task *t;

   t = eina_tls_get(task);
   if (!t) return EINA_FALSE;

   return t->cancelled(t->data);
}

EAPI void
evas_module_task_register(Eina_Bool (*cancelled)(void *data), void *data)
{
   Evas_Module_Task *t;

   t = malloc(sizeof (Evas_Module_Task));
   if (!t) return ;

   t->cancelled = cancelled;
   t->data = data;

   eina_tls_set(task, t);
}

EAPI void
evas_module_task_unregister(void)
{
    Evas_Module_Task *t;

    t = eina_tls_get(task);
    if (!t) return ;

    eina_tls_set(task, NULL);
    free(t);
}

static Eina_Hash *evas_modules[6] = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

static Eina_List *evas_module_paths = NULL;
static Eina_Array *evas_engines = NULL;

static Eina_List *
_evas_module_append(Eina_List *list, char *path)
{
   if (path)
     {
        if (evas_file_path_exists(path))
          list = eina_list_append(list, path);
        else
          free(path);
     }
   return list;
}

/* this will alloc a list of paths to search for the modules */
/* by now these are:  */
/* 1. ~/.evas/modules/ */
/* 2. $(EVAS_MODULE_DIR)/evas/modules/ */
/* 3. dladdr/evas/modules/ */
/* 4. PREFIX/evas/modules/ */
void
evas_module_paths_init(void)
{
   char *libdir, *path;

#ifdef NEED_RUN_IN_TREE
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        if (getenv("EFL_RUN_IN_TREE"))
          {
             struct stat st;
             const char mp[] = PACKAGE_BUILD_DIR"/src/modules/evas";
             if (stat(mp, &st) == 0)
               {
                  evas_module_paths = _evas_module_append(evas_module_paths, strdup(mp));
                  return;
               }
          }
     }
#endif

   /* 1. libevas.so/../evas/modules/ */
   libdir = (char *)_evas_module_libdir_get();
   if (!libdir)
     path = eina_module_symbol_path_get(evas_module_paths_init, "/evas/modules");
   else
     {
        path = malloc(strlen(libdir) + strlen("/evas/modules") + 1);
        if (path)
          {
             strcpy(path, libdir);
             strcat(path, "/evas/modules");
          }
     }
   if (eina_list_search_unsorted(evas_module_paths, (Eina_Compare_Cb) strcmp, path))
     free(path);
   else
     evas_module_paths = _evas_module_append(evas_module_paths, path);

   /* 2. PREFIX/lib/evas/modules/ */
#ifndef _MSC_VER
   path = PACKAGE_LIB_DIR "/evas/modules";
   if (!eina_list_search_unsorted(evas_module_paths, (Eina_Compare_Cb) strcmp, path))
     {
        path = strdup(path);
        if (path)
          evas_module_paths = _evas_module_append(evas_module_paths, path);
     }
#endif
}

#define EVAS_EINA_STATIC_MODULE_DEFINE(Tn, Name) \
  Eina_Bool evas_##Tn##_##Name##_init(void); \
  void evas_##Tn##_##Name##_shutdown(void);

#define EVAS_EINA_STATIC_MODULE_USE(Tn, Name) \
  { evas_##Tn##_##Name##_init, evas_##Tn##_##Name##_shutdown }

#if !EVAS_MODULE_NO_ENGINES
EVAS_EINA_STATIC_MODULE_DEFINE(engine, buffer);
EVAS_EINA_STATIC_MODULE_DEFINE(engine, fb);
EVAS_EINA_STATIC_MODULE_DEFINE(engine, gl_generic);
EVAS_EINA_STATIC_MODULE_DEFINE(engine, gl_x11);
EVAS_EINA_STATIC_MODULE_DEFINE(engine, gl_sdl);
EVAS_EINA_STATIC_MODULE_DEFINE(engine, psl1ght);
EVAS_EINA_STATIC_MODULE_DEFINE(engine, software_8);
EVAS_EINA_STATIC_MODULE_DEFINE(engine, software_8_x11);
EVAS_EINA_STATIC_MODULE_DEFINE(engine, software_ddraw);
EVAS_EINA_STATIC_MODULE_DEFINE(engine, software_gdi);
EVAS_EINA_STATIC_MODULE_DEFINE(engine, software_generic);
EVAS_EINA_STATIC_MODULE_DEFINE(engine, software_x11);
#endif

#if !EVAS_MODULE_NO_VG_LOADERS
EVAS_EINA_STATIC_MODULE_DEFINE(vg_loader, eet);
EVAS_EINA_STATIC_MODULE_DEFINE(vg_loader, svg);
#endif

#if !EVAS_MODULE_NO_IMAGE_LOADERS
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, bmp);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, dds);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, eet);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, generic);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, gif);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, ico);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, jpeg);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, jp2k);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, pmaps);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, png);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, psd);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, svg);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, tga);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, tiff);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, wbmp);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, webp);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, xpm);
EVAS_EINA_STATIC_MODULE_DEFINE(image_loader, tgv);
#endif

#if !EVAS_MODULE_NO_VG_SAVERS
EVAS_EINA_STATIC_MODULE_DEFINE(vg_saver, eet);
#endif

#if !EVAS_MODULE_NO_IMAGE_SAVERS
EVAS_EINA_STATIC_MODULE_DEFINE(image_saver, eet);
EVAS_EINA_STATIC_MODULE_DEFINE(image_saver, jpeg);
EVAS_EINA_STATIC_MODULE_DEFINE(image_saver, png);
EVAS_EINA_STATIC_MODULE_DEFINE(image_saver, tiff);
EVAS_EINA_STATIC_MODULE_DEFINE(image_saver, webp);
EVAS_EINA_STATIC_MODULE_DEFINE(image_saver, tgv);
#endif

static const struct {
   Eina_Bool (*init)(void);
   void (*shutdown)(void);
} evas_static_module[] = {
#if !EVAS_MODULE_NO_ENGINES
#ifdef EVAS_STATIC_BUILD_BUFFER
  EVAS_EINA_STATIC_MODULE_USE(engine, buffer),
#endif
#ifdef EVAS_STATIC_BUILD_FB
  EVAS_EINA_STATIC_MODULE_USE(engine, fb),
#endif
#ifdef EVAS_STATIC_BUILD_GL_COMMON
  EVAS_EINA_STATIC_MODULE_USE(engine, gl_generic),
#endif
#ifdef EVAS_STATIC_BUILD_GL_X11
  EVAS_EINA_STATIC_MODULE_USE(engine, gl_x11),
#endif
#ifdef EVAS_STATIC_BUILD_GL_SDL
  EVAS_EINA_STATIC_MODULE_USE(engine, gl_sdl),
#endif
#ifdef EVAS_STATIC_BUILD_PSL1GHT
  EVAS_EINA_STATIC_MODULE_USE(engine, psl1ght),
#endif
#ifdef EVAS_STATIC_BUILD_SOFTWARE_DDRAW
  EVAS_EINA_STATIC_MODULE_USE(engine, software_ddraw),
#endif
#ifdef EVAS_STATIC_BUILD_SOFTWARE_GDI
  EVAS_EINA_STATIC_MODULE_USE(engine, software_gdi),
#endif
#ifdef EVAS_STATIC_BUILD_SOFTWARE_GENERIC
  EVAS_EINA_STATIC_MODULE_USE(engine, software_generic),
#endif
#ifdef EVAS_STATIC_BUILD_SOFTWARE_X11
  EVAS_EINA_STATIC_MODULE_USE(engine, software_x11),
#endif
#endif
#if !EVAS_MODULE_NO_VG_LOADERS
#ifdef EVAS_STATIC_BUILD_VG_SVG
  EVAS_EINA_STATIC_MODULE_USE(vg_loader, svg),
#endif
#ifdef EVAS_STATIC_BUILD_VG_EET
  EVAS_EINA_STATIC_MODULE_USE(vg_loader, eet),
#endif
#endif
#if !EVAS_MODULE_NO_IMAGE_LOADERS
#ifdef EVAS_STATIC_BUILD_BMP
  EVAS_EINA_STATIC_MODULE_USE(image_loader, bmp),
#endif
#ifdef EVAS_STATIC_BUILD_DDS
  EVAS_EINA_STATIC_MODULE_USE(image_loader, dds),
#endif
#ifdef EVAS_STATIC_BUILD_EET
  EVAS_EINA_STATIC_MODULE_USE(image_loader, eet),
#endif
#ifdef EVAS_STATIC_BUILD_GENERIC
  EVAS_EINA_STATIC_MODULE_USE(image_loader, generic),
#endif
#ifdef EVAS_STATIC_BUILD_GIF
  EVAS_EINA_STATIC_MODULE_USE(image_loader, gif),
#endif
#ifdef EVAS_STATIC_BUILD_ICO
  EVAS_EINA_STATIC_MODULE_USE(image_loader, ico),
#endif
#ifdef EVAS_STATIC_BUILD_JPEG
  EVAS_EINA_STATIC_MODULE_USE(image_loader, jpeg),
#endif
#ifdef EVAS_STATIC_BUILD_JP2K
  EVAS_EINA_STATIC_MODULE_USE(image_loader, jp2k),
#endif
#ifdef EVAS_STATIC_BUILD_PMAPS
  EVAS_EINA_STATIC_MODULE_USE(image_loader, pmaps),
#endif
#ifdef EVAS_STATIC_BUILD_PNG
  EVAS_EINA_STATIC_MODULE_USE(image_loader, png),
#endif
#ifdef EVAS_STATIC_BUILD_PSD
  EVAS_EINA_STATIC_MODULE_USE(image_loader, psd),
#endif
#ifdef EVAS_STATIC_BUILD_SVG
  EVAS_EINA_STATIC_MODULE_USE(image_loader, svg),
#endif
#ifdef EVAS_STATIC_BUILD_TGA
  EVAS_EINA_STATIC_MODULE_USE(image_loader, tga),
#endif
#ifdef EVAS_STATIC_BUILD_TIFF
  EVAS_EINA_STATIC_MODULE_USE(image_loader, tiff),
#endif
#ifdef EVAS_STATIC_BUILD_WBMP
  EVAS_EINA_STATIC_MODULE_USE(image_loader, wbmp),
#endif
#ifdef EVAS_STATIC_BUILD_WEBP
  EVAS_EINA_STATIC_MODULE_USE(image_loader, webp),
#endif
#ifdef EVAS_STATIC_BUILD_XPM
  EVAS_EINA_STATIC_MODULE_USE(image_loader, xpm),
#endif
#ifdef EVAS_STATIC_BUILD_TGV
  EVAS_EINA_STATIC_MODULE_USE(image_loader, tgv),
#endif
#endif
#if !EVAS_MODULE_NO_VG_SAVERS
#ifdef EVAS_STATIC_BUILD_VG_EET
  EVAS_EINA_STATIC_MODULE_USE(vg_saver, eet),
#endif
#endif
#if !EVAS_MODULE_NO_IMAGE_SAVERS
#ifdef EVAS_STATIC_BUILD_EET
  EVAS_EINA_STATIC_MODULE_USE(image_saver, eet),
#endif
#ifdef EVAS_STATIC_BUILD_JPEG
  EVAS_EINA_STATIC_MODULE_USE(image_saver, jpeg),
#endif
#ifdef EVAS_STATIC_BUILD_PNG
  EVAS_EINA_STATIC_MODULE_USE(image_saver, png),
#endif
#ifdef EVAS_STATIC_BUILD_TIFF
  EVAS_EINA_STATIC_MODULE_USE(image_saver, tiff),
#endif
#ifdef EVAS_STATIC_BUILD_WEBP
  EVAS_EINA_STATIC_MODULE_USE(image_saver, webp),
#endif
#ifdef EVAS_STATIC_BUILD_TGV
  EVAS_EINA_STATIC_MODULE_USE(image_saver, tgv),
#endif
#endif
  { NULL, NULL }
};

static void _evas_module_hash_free_cb(void *data);

/* this will alloc an Evas_Module struct for each module
 * it finds on the paths */
void
evas_module_init(void)
{
   int i;

   evas_module_paths_init();

   evas_modules[EVAS_MODULE_TYPE_ENGINE] = eina_hash_string_small_new(_evas_module_hash_free_cb);
   evas_modules[EVAS_MODULE_TYPE_IMAGE_LOADER] = eina_hash_string_small_new(_evas_module_hash_free_cb);
   evas_modules[EVAS_MODULE_TYPE_IMAGE_SAVER] = eina_hash_string_small_new(_evas_module_hash_free_cb);
   evas_modules[EVAS_MODULE_TYPE_OBJECT] = eina_hash_string_small_new(_evas_module_hash_free_cb);
   evas_modules[EVAS_MODULE_TYPE_VG_LOADER] = eina_hash_string_small_new(_evas_module_hash_free_cb);
   evas_modules[EVAS_MODULE_TYPE_VG_SAVER] = eina_hash_string_small_new(_evas_module_hash_free_cb);

   evas_engines = eina_array_new(4);

   eina_tls_cb_new(&task, (Eina_TLS_Delete_Cb) evas_module_task_unregister);

   for (i = 0; evas_static_module[i].init; ++i)
     evas_static_module[i].init();
}

Eina_Bool
evas_module_register(const Evas_Module_Api *module, Evas_Module_Type type)
{
   Evas_Module *em;

   if ((unsigned int)type > 5) return EINA_FALSE;
   if (!module) return EINA_FALSE;
   if (module->version != EVAS_MODULE_API_VERSION) return EINA_FALSE;

   em = eina_hash_find(evas_modules[type], module->name);
   if (em) return EINA_FALSE;

   em = calloc(1, sizeof (Evas_Module));
   if (!em) return EINA_FALSE;

   LKI(em->lock);
   em->definition = module;

   if (type == EVAS_MODULE_TYPE_ENGINE)
     {
        eina_array_push(evas_engines, em);
        em->id_engine = eina_array_count(evas_engines);
     }

   eina_hash_direct_add(evas_modules[type], module->name, em);

   return EINA_TRUE;
}

Eina_List *
evas_module_engine_list(void)
{
   Evas_Module *em;
   Eina_List *r = NULL, *l, *ll;
   Eina_Array_Iterator iterator;
   Eina_Iterator *it, *it2;
   unsigned int i;
   const char *s, *s2;
   char buf[PATH_MAX];
#ifdef NEED_RUN_IN_TREE
   Eina_Bool run_in_tree;

   run_in_tree = !!getenv("EFL_RUN_IN_TREE");
#endif

   EINA_LIST_FOREACH(evas_module_paths, l, s)
     {
        snprintf(buf, sizeof(buf), "%s/engines", s);
        it = eina_file_direct_ls(buf);
        if (it)
          {
             Eina_File_Direct_Info *fi;

             EINA_ITERATOR_FOREACH(it, fi)
               {
                  const char *fname = fi->path + fi->name_start;

#ifdef NEED_RUN_IN_TREE
                  buf[0] = '\0';
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
                  if (getuid() == geteuid())
#endif
                    {
                       if (run_in_tree)
                         {
                            snprintf(buf, sizeof(buf), "%s/engines/%s/.libs",
                                     s, fname);
                            if (!evas_file_path_exists(buf))
                            buf[0] = '\0';
                         }
                    }

                  if (buf[0] == '\0')
#endif
                    snprintf(buf, sizeof(buf), "%s/engines/%s/%s",
                             s, fname, MODULE_ARCH);

                  it2 = eina_file_ls(buf);
                  if (it2)
                    {
                       EINA_LIST_FOREACH(r, ll, s2)
                         {
                            if (!strcmp(fname, s2)) break;
                         }
                       if (!ll)
                         r = eina_list_append(r, eina_stringshare_add(fname));
                       eina_iterator_free(it2);
                    }
               }
             eina_iterator_free(it);
          }
     }

   EINA_ARRAY_ITER_NEXT(evas_engines, i, em, iterator)
     {
        EINA_LIST_FOREACH(r, ll, s2)
          {
             if (!strcmp(em->definition->name, s2)) break;
          }
        if (!ll)
          r = eina_list_append(r, eina_stringshare_add(em->definition->name));
     }

   return r;
}

Eina_Bool
evas_module_unregister(const Evas_Module_Api *module, Evas_Module_Type type)
{
   Evas_Module *em;

   if ((unsigned int)type > 5) return EINA_FALSE;
   if (!module) return EINA_FALSE;

   em = eina_hash_find(evas_modules[type], module->name);
   if (!em || em->definition != module) return EINA_FALSE;

   eina_hash_del(evas_modules[type], module->name, em);

   return EINA_TRUE;
}

static void
_evas_module_hash_free_cb(void *data)
{
   Evas_Module *em = data;

   // Note: This free callback leaks the Eina_Module, and does not call
   // dlclose(). This is by choice as dlclose() leads to other issues.

   if (!em) return;
   if (em->id_engine > 0)
     eina_array_data_set(evas_engines, em->id_engine - 1, NULL);
   if (em->loaded)
     {
        em->definition->func.close(em);
        em->loaded = 0;
     }

   LKD(em->lock);
   free(em);
}

#if defined(_WIN32) || defined(__CYGWIN__)
# define EVAS_MODULE_NAME "module.dll"
#else
# define EVAS_MODULE_NAME "module.so"
#endif

Evas_Module *
evas_module_find_type(Evas_Module_Type type, const char *name)
{
   const char *path;
   char buffer[PATH_MAX];
   Evas_Module *em;
   Eina_Module *en;
   Eina_List *l;
#ifdef NEED_RUN_IN_TREE
   Eina_Bool run_in_tree;
#endif

   if ((unsigned int)type > 5) return NULL;

   em = eina_hash_find(evas_modules[type], name);
   if (em)
     {
        if (evas_module_load(em)) return em;
        return NULL;
     }

#ifdef NEED_RUN_IN_TREE
   run_in_tree = !!getenv("EFL_RUN_IN_TREE");
#endif

   EINA_LIST_FOREACH(evas_module_paths, l, path)
     {
        const char *type_str = "unknown";
        switch (type)
          {
           case EVAS_MODULE_TYPE_ENGINE: type_str = "engines"; break;
           case EVAS_MODULE_TYPE_IMAGE_LOADER: type_str = "image_loaders"; break;
           case EVAS_MODULE_TYPE_IMAGE_SAVER: type_str = "image_savers"; break;
           case EVAS_MODULE_TYPE_OBJECT: type_str = "object"; break;
           case EVAS_MODULE_TYPE_VG_LOADER: type_str = "vg_loaders"; break;
           case EVAS_MODULE_TYPE_VG_SAVER: type_str = "vg_savers"; break;
          }

        buffer[0] = '\0';
#if NEED_RUN_IN_TREE
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
        if (getuid() == geteuid())
#endif
          {
             if (run_in_tree)
               {
                  snprintf(buffer, sizeof(buffer), "%s/%s/%s/.libs/%s",
                           path, type_str, name, EVAS_MODULE_NAME);
                  if (!evas_file_path_exists(buffer))
                  buffer[0] = '\0';
               }
          }
#endif

        if (buffer[0] == '\0')
          snprintf(buffer, sizeof(buffer), "%s/%s/%s/%s/%s",
                   path, type_str, name, MODULE_ARCH, EVAS_MODULE_NAME);

        if (!evas_file_path_is_file(buffer)) continue;

        en = eina_module_new(buffer);
        if (!en) continue;

        if (type == EVAS_MODULE_TYPE_ENGINE)
          eina_module_symbol_global_set(en, EINA_TRUE);

        if (!eina_module_load(en))
          {
             eina_module_free(en);
             continue;
          }
        // this is intentional. the above module load if it succeeds
        // registers the evas module below in the table that we then
        // lookup in the hash... and then load that as a 2nd stage.
        // since we will never unload a module once used it doesnt matter
        // what happens to the handle anyway.
        em = eina_hash_find(evas_modules[type], name);
        if (em)
          {
             if (evas_module_load(em)) return em;
          }

        eina_module_free(en);
     }

   return NULL;
}

Evas_Module *
evas_module_engine_get(int render_method)
{
   if ((render_method <= 0) ||
       ((unsigned int)render_method > eina_array_count(evas_engines)))
     return NULL;
   return eina_array_data_get(evas_engines, render_method - 1);
}

void
evas_module_foreach_image_loader(Eina_Hash_Foreach cb, const void *fdata)
{
   eina_hash_foreach(evas_modules[EVAS_MODULE_TYPE_IMAGE_LOADER], cb, fdata);
}

int
evas_module_load(Evas_Module *em)
{
   if (em->loaded) return 1;
   if (!em->definition) return 0;

   if (!em->definition->func.open(em)) return 0;
   em->loaded = 1;

   return 1;
}

void
evas_module_unload(Evas_Module *em)
{
   if (!em->loaded)
     return;
   if (!em->definition)
     return;

// for now lets not unload modules - they may still be in use.   
//   em->definition->func.close(em);
//   em->loaded = 0;

}

void
evas_module_ref(Evas_Module *em)
{
   LKL(em->lock);
   em->ref++;
   LKU(em->lock);
}

void
evas_module_unref(Evas_Module *em)
{
   LKL(em->lock);
   em->ref--;
   LKU(em->lock);
}

static int use_count = 0;

void
evas_module_use(Evas_Module *em)
{
   em->last_used = use_count;
}

void
evas_module_clean(void)
{
   static int call_count = 0;
/*    int ago; */
   int noclean = -1;
/*    Eina_List *l; */
/*    Evas_Module *em; */

   /* only clean modules every 256 calls */
   call_count++;
   if (call_count <= 256) return;
   call_count = 0;

   if (noclean == -1)
     {
        if (getenv("EVAS_NOCLEAN"))
          noclean = 1;
        else
          noclean = 0;
     }
   if (noclean == 1) return;

   /* disable module cleaning for now - may cause instability with some modules */
   return;

   /* FIXME: Don't know what it is supposed to do. */
/*    /\* incriment use counter = 28bits *\/ */
/*    use_count++; */
/*    if (use_count > 0x0fffffff) use_count = 0; */

/*    /\* printf("CLEAN!\n"); *\/ */
/*    /\* go through all modules *\/ */
/*    EINA_LIST_FOREACH(evas_modules, l, em) */
/*      { */
/*         /\* printf("M %s %i %i\n", em->name, em->ref, em->loaded); *\/ */
/* 	/\* if the module is refernced - skip *\/ */
/* 	if ((em->ref > 0) || (!em->loaded)) continue; */
/* 	/\* how many clean cycles ago was this module last used *\/ */
/* 	ago = use_count - em->last_used; */
/* 	if (em->last_used > use_count) ago += 0x10000000; */
/* 	/\* if it was used last more than N clean cycles ago - unload *\/ */
/* 	if (ago > 5) */
/* 	  { */
/*             /\* printf("  UNLOAD %s\n", em->name); *\/ */
/* 	     evas_module_unload(em); */
/* 	  } */
/*      } */
}

static Eina_Prefix *pfx = NULL;

/* will dlclose all the modules loaded and free all the structs */
void
evas_module_shutdown(void)
{
   char *path;
   int i;

   for (i = 0; evas_static_module[i].shutdown; ++i)
     evas_static_module[i].shutdown();

   eina_hash_free(evas_modules[EVAS_MODULE_TYPE_ENGINE]);
   evas_modules[EVAS_MODULE_TYPE_ENGINE] = NULL;
   eina_hash_free(evas_modules[EVAS_MODULE_TYPE_IMAGE_LOADER]);
   evas_modules[EVAS_MODULE_TYPE_IMAGE_LOADER] = NULL;
   eina_hash_free(evas_modules[EVAS_MODULE_TYPE_IMAGE_SAVER]);
   evas_modules[EVAS_MODULE_TYPE_IMAGE_SAVER] = NULL;
   eina_hash_free(evas_modules[EVAS_MODULE_TYPE_OBJECT]);
   evas_modules[EVAS_MODULE_TYPE_OBJECT] = NULL;
   eina_hash_free(evas_modules[EVAS_MODULE_TYPE_VG_LOADER]);
   evas_modules[EVAS_MODULE_TYPE_VG_LOADER] = NULL;
   eina_hash_free(evas_modules[EVAS_MODULE_TYPE_VG_SAVER]);
   evas_modules[EVAS_MODULE_TYPE_VG_SAVER] = NULL;

   eina_tls_free(task);

   EINA_LIST_FREE(evas_module_paths, path)
     free(path);

   eina_array_free(evas_engines);
   evas_engines = NULL;
   if (pfx)
     {
        eina_prefix_free(pfx);
        pfx = NULL;
     }
}

EAPI int
_evas_module_engine_inherit(Evas_Func *funcs, char *name, size_t info)
{
   Evas_Module *em;

   em = evas_module_find_type(EVAS_MODULE_TYPE_ENGINE, name);
   if (em)
     {
        if (evas_module_load(em))
          {
             /* FIXME: no way to unref */
             evas_module_ref(em);
             evas_module_use(em);
             *funcs = *((Evas_Func *)(em->functions));
             funcs->info_size = info;
             return 1;
          }
     }
   return 0;
}

EAPI const char *
_evas_module_libdir_get(void)
{
   if (!pfx) pfx = eina_prefix_new
      (NULL, _evas_module_libdir_get, "EVAS", "evas", "checkme",
       PACKAGE_BIN_DIR, PACKAGE_LIB_DIR, PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);
   if (!pfx) return NULL;
   return eina_prefix_lib_get(pfx);
}

const char *
_evas_module_datadir_get(void)
{
   if (!pfx) pfx = eina_prefix_new
      (NULL, _evas_module_libdir_get, "EVAS", "evas", "checkme",
       PACKAGE_BIN_DIR, PACKAGE_LIB_DIR, PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);
   if (!pfx) return NULL;
   return eina_prefix_data_get(pfx);
}

EAPI const char *
evas_cserve_path_get(void)
{
   static char buf[PATH_MAX];
   const char *lib;
   Eina_Bool shutdown = EINA_FALSE;

   if (!pfx)
     {
        shutdown = EINA_TRUE;
        eina_init();
        pfx = eina_prefix_new
          (NULL, _evas_module_libdir_get, "EVAS", "evas", "checkme",
              PACKAGE_BIN_DIR, PACKAGE_LIB_DIR,
              PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);
        if (!pfx)
          {
             eina_shutdown();
             return NULL;
          }
     }
   lib = eina_prefix_lib_get(pfx);
   if (!lib)
     {
        if (shutdown)
          {
             eina_prefix_free(pfx);
             pfx = NULL;
             eina_shutdown();
          }
        return NULL;
     }
   snprintf(buf, sizeof(buf), "%s/evas/cserve2/bin/%s/evas_cserve2",
            lib, MODULE_ARCH);
   if (shutdown)
     {
        eina_prefix_free(pfx);
        pfx = NULL;
        eina_shutdown();
     }
   return buf;
}
