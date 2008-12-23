#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <dirent.h>	/* DIR, dirent */
#include <dlfcn.h>	/* dlopen,dlclose,etc */

#include <evas_common.h>
#include <evas_private.h>

Eina_List *evas_modules = NULL;
static Eina_List *evas_module_paths = NULL;

static void
_evas_module_path_append(Evas_Module_Type type, char *path, const char *subdir)
{
   Evas_Module_Path *mp;
   char *buf;

   buf = evas_file_path_join(path, subdir);
   if (!buf) return;
   if (evas_file_path_exists(buf))
     {
	mp = malloc(sizeof(Evas_Module_Path));
	mp->type = type;
	mp->path = buf;
	evas_module_paths = eina_list_append(evas_module_paths, mp);
     }
   else
     free(buf);
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

   char *prefix;
   char *path;
   Eina_List *paths = NULL;

   /* 1. ~/.evas/modules/ */
   prefix = getenv("HOME");
   if (prefix)
     {
	path = malloc(strlen(prefix) + 1 + strlen("/.evas/modules"));
	if (path)
	  {
	     path[0] = 0;
	     strcpy(path, prefix);
	     strcat(path, "/.evas/modules");
	     if (evas_file_path_exists(path))
	       paths = eina_list_append(paths, path);
	     else
	       free(path);
	  }
     }

   /* 2. $(EVAS_MODULE_DIR)/evas/modules/ */
   prefix = getenv("EVAS_MODULES_DIR");
   if (prefix)
     {
	path = malloc(strlen(prefix) + 1 + strlen("/evas/modules"));
	if (path)
	  {
	     path[0] = 0;
	     strcpy(path, prefix);
	     strcat(path, "/evas/modules");
	     if (evas_file_path_exists(path))
	       paths = eina_list_append(paths, path);
	     else
	       free(path);
	  }
     }

#ifdef HAVE_DLADDR
     {
	Dl_info evas_dl;
	/* 3. libevas.so/../evas/modules/ */
	if (dladdr(evas_module_paths_init, &evas_dl))
	  {
	     int length;

	     if (strrchr(evas_dl.dli_fname, '/'))
	       {
		  length = strlen(strrchr(evas_dl.dli_fname, '/'));
		  path = malloc(strlen(evas_dl.dli_fname) - length +
				strlen("/evas/modules") + 1);
		  if (path)
		    {
		       strncpy(path, evas_dl.dli_fname,
			       strlen(evas_dl.dli_fname) - length);
		       path[strlen(evas_dl.dli_fname) - length] = 0;
		       strcat(path, "/evas/modules");
		       if (evas_file_path_exists(path))
			 paths = eina_list_append(paths, path);
		       else
			 free(path);
		    }
	       }
	  }
     }
#else
   /* 4. PREFIX/evas/modules/ */
   prefix = PACKAGE_LIB_DIR;
   path = malloc(strlen(prefix) + 1 + strlen("/evas/modules"));
   if (path)
     {
	strcpy(path, prefix);
	strcat(path, "/evas/modules");
	if (evas_file_path_exists(path))
	  paths = eina_list_append(paths, path);
	else
	  free(path);
     }
#endif

   /* append all the module types subdirs */
   while (paths)
     {
	/* here must be added all the types of modules and their subdirectory */
	/* do this on a separate function */
	/* 1. engines */
	_evas_module_path_append(EVAS_MODULE_TYPE_ENGINE, paths->data, "engines");
	_evas_module_path_append(EVAS_MODULE_TYPE_IMAGE_LOADER, paths->data, "loaders");
	_evas_module_path_append(EVAS_MODULE_TYPE_IMAGE_SAVER, paths->data, "savers");
	_evas_module_path_append(EVAS_MODULE_TYPE_OBJECT, paths->data, "objects");
	free(paths->data);
	paths = eina_list_remove_list(paths, paths);
     }
}

/* this will alloc an Evas_Module struct for each module
 * it finds on the paths */
void
evas_module_init(void)
{
   Eina_List *l;
   Evas_Module_Path *mp;
   int new_id_engine = 1;

/*    printf("[init modules]\n"); */
   evas_module_paths_init();
   EINA_LIST_FOREACH(evas_module_paths, l, mp)
     {
	DIR *dir;
	struct dirent *de;

	if (!(dir = opendir(mp->path))) break;
/* 	printf("[evas module] searching modules on %s\n", mp->path); */
	while ((de = readdir(dir)))
	  {
	     char *buf;

	     if ((!strcmp(de->d_name, ".")) || (!strcmp(de->d_name, "..")))
	       continue;
	     buf = malloc(strlen(mp->path) + 1 + strlen(de->d_name) + 1);
	     sprintf(buf, "%s/%s", mp->path, de->d_name);
	     if (evas_file_path_is_dir(buf))
	       {
		  Evas_Module *em;

		  em = calloc(1, sizeof(Evas_Module));
		  if (!em) continue;
		  em->name = strdup(de->d_name);
		  em->path = strdup(mp->path);
		  em->type = mp->type;
		  em->handle = NULL;
		  em->data = NULL;
                  em->loaded = 0;
#if defined(BUILD_PTHREAD) && defined(BUILD_ASYNC_PRELOAD)
		  pthread_spin_init(&em->lock, PTHREAD_PROCESS_PRIVATE);
#endif
		  if (em->type == EVAS_MODULE_TYPE_ENGINE)
		    {
		       Evas_Module_Engine *eme;

		       eme = malloc(sizeof(Evas_Module_Engine));
		       if (eme)
			 {
			    eme->id = new_id_engine;
			    em->data = eme;
			    new_id_engine++;
			 }
		    }
                  else if (em->type == EVAS_MODULE_TYPE_IMAGE_LOADER)
		    {
		    }
                  else if (em->type == EVAS_MODULE_TYPE_IMAGE_SAVER)
		    {
		    }
/* 		  printf("[evas module] including module path %s/%s of type %d\n",em->path, em->name, em->type); */
		  evas_modules = eina_list_append(evas_modules, em);
	       }
	     free(buf);
	  }
	closedir(dir);
     }
}

Evas_Module *
evas_module_find_type(Evas_Module_Type type, const char *name)
{
   Eina_List *l;
   Evas_Module *em;

   EINA_LIST_FOREACH(evas_modules, l, em)
     {
	if ((type == em->type) && (!strcmp(name,em->name)))
	  {
             if (evas_modules != l)
	       {
		  evas_modules = eina_list_promote_list(evas_modules, l);
	       }
	     return em;
	  }
     }
   return NULL;
}

int
evas_module_load(Evas_Module *em)
{
   char buf[4096];
   void *handle;

   if (em->loaded) return 1;

/*   printf("LOAD %s\n", em->name); */
#if defined(__CEGCC__) || defined(__MINGW32CE__)
   switch (em->type) {
     case EVAS_MODULE_TYPE_IMAGE_SAVER:
        snprintf(buf, sizeof(buf), "%s/%s/%s/saver_%s.dll", em->path, em->name, MODULE_ARCH, em->name);
        break;
     case EVAS_MODULE_TYPE_IMAGE_LOADER:
        snprintf(buf, sizeof(buf), "%s/%s/%s/loader_%s.dll", em->path, em->name, MODULE_ARCH, em->name);
        break;
     case EVAS_MODULE_TYPE_ENGINE:
        snprintf(buf, sizeof(buf), "%s/%s/%s/engine_%s.dll", em->path, em->name, MODULE_ARCH, em->name);
        break;
     default:
        snprintf(buf, sizeof(buf), "%s/%s/%s/object_%s.dll", em->path, em->name, MODULE_ARCH, em->name);
   }
#elif _WIN32
   snprintf(buf, sizeof(buf), "%s/%s/%s/module.dll", em->path, em->name, MODULE_ARCH);
#else
   snprintf(buf, sizeof(buf), "%s/%s/%s/module.so", em->path, em->name, MODULE_ARCH);
#endif
   if (!evas_file_path_exists(buf))
     {
	printf("[evas module] error loading the module %s. It doesnt exists\n", buf);
	return 0;
     }

   handle = dlopen(buf, RTLD_LAZY);
   if (handle)
     {
	em->handle = handle;
	em->func.open = dlsym(em->handle, "module_open");
	em->func.close = dlsym(em->handle, "module_close");
	em->api = dlsym(em->handle, "evas_modapi");

	if ((!em->func.open) || (!em->api) || (!em->func.close))
	  goto error_dl;
	/* check the api */
	if (em->api->version != EVAS_MODULE_API_VERSION)
	  {
	     printf("[evas module] error loading the modules %s. The version doesnt match\n", buf);
	     goto error_dl;
	  }
	em->func.open(em);
	em->loaded = 1;
	return 1;
     }
   error_dl:
     {
	const char *err;

	err = dlerror();
	printf("[evas module] error loading the module %s. %s\n", buf, err);
	if (handle)
	  dlclose(handle);
	em->handle = NULL;
	em->func.open = NULL;
	em->func.close = NULL;
	em->api = NULL;
     }
   return 0;
}

void
evas_module_unload(Evas_Module *em)
{
   if (!em->loaded)
     return;
   if (em->handle)
     {
	em->func.close(em);
	dlclose(em->handle);
     }
   em->handle = NULL;
   em->func.open = NULL;
   em->func.close = NULL;
   em->api = NULL;
   em->loaded = 0;
#if defined(HAVE_PTHREAD_H) && defined(BUILD_ASYNC_PRELOAD)
   pthread_spin_destroy(&em->lock);
#endif
}

void
evas_module_ref(Evas_Module *em)
{
#if defined(HAVE_PTHREAD_H) && defined(BUILD_ASYNC_PRELOAD)
   pthread_spin_lock(&em->lock);
#endif
   em->ref++;
/*   printf("M: %s ref++ = %i\n", em->name, em->ref); */
#if defined(HAVE_PTHREAD_H) && defined(BUILD_ASYNC_PRELOAD)
   pthread_spin_unlock(&em->lock);
#endif
}

void
evas_module_unref(Evas_Module *em)
{
#if defined(HAVE_PTHREAD_H) && defined(BUILD_ASYNC_PRELOAD)
   pthread_spin_lock(&em->lock);
#endif
   em->ref--;
/*   printf("M: %s ref-- = %i\n", em->name, em->ref); */
#if defined(HAVE_PTHREAD_H) && defined(BUILD_ASYNC_PRELOAD)
   pthread_spin_unlock(&em->lock);
#endif
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
   int ago;
   int noclean = -1;
   Eina_List *l;
   Evas_Module *em;

   /* only clean modules every 256 calls */
   call_count++;
   if (call_count <= 256) return;
   call_count = 0;

   if (noclean == -1)
     {
	if (getenv("EVAS_NOCLEAN"))
	  {
	     noclean = 1;
	  }
	noclean = 0;
     }
   if (noclean == 1) return;

   /* disable module cleaning for now - may cause instability with some modules */
   return;

   /* incriment use counter = 28bits */
   use_count++;
   if (use_count > 0x0fffffff) use_count = 0;

   /* printf("CLEAN!\n"); */
   /* go through all modules */
   EINA_LIST_FOREACH(evas_modules, l, em)
     {
        /* printf("M %s %i %i\n", em->name, em->ref, em->loaded); */
	/* if the module is refernced - skip */
	if ((em->ref > 0) || (!em->loaded)) continue;
	/* how many clean cycles ago was this module last used */
	ago = use_count - em->last_used;
	if (em->last_used > use_count) ago += 0x10000000;
	/* if it was used last more than N clean cycles ago - unload */
	if (ago > 5)
	  {
            /* printf("  UNLOAD %s\n", em->name); */
	     evas_module_unload(em);
	  }
     }
}

/* will dlclose all the modules loaded and free all the structs */
void
evas_module_shutdown(void)
{
   Evas_Module *em;

/*    printf("[shutdown modules]\n"); */
   while (evas_modules)
     {
	em = (Evas_Module *)evas_modules->data;
	evas_module_unload(em);
	if (em->name) free(em->name);
	if (em->path) free(em->path);
	if (em->type == EVAS_MODULE_TYPE_ENGINE)
	  {
	     if (em->data) free(em->data);
	  }
	else if (em->type == EVAS_MODULE_TYPE_IMAGE_LOADER)
	  {
	  }
	else if (em->type == EVAS_MODULE_TYPE_IMAGE_SAVER)
	  {
	  }
	free(evas_modules->data);
	evas_modules = eina_list_remove_list(evas_modules, evas_modules);
     }
   while (evas_module_paths)
     {
	Evas_Module_Path *mp;

	mp = evas_module_paths->data;
	evas_module_paths = eina_list_remove_list(evas_module_paths, evas_module_paths);
	free(mp->path);
	free(mp);
     }
}

EAPI int
_evas_module_engine_inherit(Evas_Func *funcs, char *name)
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
	     return 1;
	  }
     }
   return 0;
}
