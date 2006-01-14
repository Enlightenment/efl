#ifdef __GNUC__
# ifndef __USE_GNU
#  define __USE_GNU
# endif
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# ifndef _BSD_SOURCE
#  define _BSD_SOURCE
# endif
#endif

#include <dirent.h>	// DIR, dirent
#include <dlfcn.h> 	// dlopen,dlclose,etc

#include <evas_common.h>
#include <evas_private.h>

Evas_List *evas_modules = NULL;

static Evas_List *evas_module_paths = NULL;

static void
_evas_module_path_append(Evas_Module_Type type, char *path, char *subdir)
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
	evas_module_paths = evas_list_append(evas_module_paths, mp);
     }
   else
     free(buf);
}


/* this will alloc a list of paths to search for the modules */
/* by now these are:  */
/* 1. ~/.evas/modules/ */
/* 2. PREFIX/evas/modules/ */
/* 3. dladdr/evas/modules/ */
void
evas_module_paths_init(void)
{
   
   char *prefix;
   char *path;
   int i;
   Evas_List *paths = NULL;
   
   /* 1. ~/.evas/modules/ */
   prefix = getenv("HOME");
   path = malloc(strlen(prefix) + 1 + strlen("/.evas/modules"));
   path[0] = 0;
   strcpy(path, prefix);
   strcat(path, "/.evas/modules");
   if (evas_file_path_exists(path))
     paths = evas_list_append(paths,path);
   else
     free(path);

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
			 paths = evas_list_append(paths, path);
		       else
			 free(path);
		    }
	       }
	  }
     }
#else
   /* 3. PREFIX/evas/modules/ */
   prefix = PACKAGE_LIB_DIR; 
   path = malloc(strlen(prefix) + 1 + strlen("/evas/modules"));
   strcpy(path, prefix);
   strcat(path, "/evas/modules");
   if (evas_file_path_exists(path))
     paths = evas_list_append(paths, path);
   else
     free(path);
#endif
   
   /* append all the module types subdirs */
   while (paths)
     {
	/* here must be added all the types of modules and their subdirectory */
	/* do this on a separate function */
	/* 1. engines */
	_evas_module_path_append(EVAS_MODULE_TYPE_ENGINE, paths->data, "engines");
	_evas_module_path_append(EVAS_MODULE_TYPE_OBJECT, paths->data, "objects");
	free(paths->data);
	paths = evas_list_remove_list(paths, paths);
     }
}

/* this will alloc an Evas_Module struct for each module 
 * it finds on the paths */
void
evas_module_init(void)
{
   Evas_List *l;
   int new_id = 1;
   
//   printf("[init modules]\n");
   evas_module_paths_init();
   for (l = evas_module_paths; l; l = l->next)
     {
	Evas_Module_Path *mp;
	DIR *dir;
	struct dirent *de;
	
	mp = l->data;
	
	if (!(dir = opendir(mp->path))) break;
//	printf("[evas module] searching modules on %s\n", mp->path);
	while (de = readdir(dir))
	  {
	     char *buf;
	     
	     if ((!strcmp(de->d_name, ".")) || (!strcmp(de->d_name, "..")))
	       continue;
	     buf = malloc(strlen(mp->path) + 1 + strlen(de->d_name) + 1);
	     sprintf(buf,"%s/%s", mp->path,de->d_name);
	     if (evas_file_path_is_dir(buf))
	       {
		  Evas_Module *em;
		  
		  em = malloc(sizeof(Evas_Module));
		  if (!em) continue;
		  em->name = strdup(de->d_name);
		  em->path = strdup(mp->path);
		  em->type = mp->type;
		  em->handle = NULL;
		  em->data = NULL;
		  if (em->type == EVAS_MODULE_TYPE_ENGINE)
		    {
		       Evas_Module_Engine *eme;
		       
		       eme = malloc(sizeof(Evas_Module_Engine));
		       if (eme)
			 {
			    eme->id = new_id;
			    em->data = eme;
			    new_id++;
			 }
		    }
//		  printf("[evas module] including module path %s/%s of type %d\n",em->path, em->name, em->type);
		  evas_modules = evas_list_append(evas_modules, em);
	       }
	     free(buf);
	  }
     }
}

Evas_Module *
evas_module_find_type(Evas_Module_Type type, const char *name)
{
   Evas_List *l;
   
   for (l = evas_modules; l; l = l->next)
     {
	Evas_Module *em;
	
	em = (Evas_Module*)l->data;
	if ((type == em->type) && (!strcmp(name,em->name)))
	  return em;
     }
   return NULL;
}

int
evas_module_load(Evas_Module *em)
{
   char buf[4096];
   void *handle;
   
   snprintf(buf, sizeof(buf), "%s/%s/%s/module.so", 
	    em->path, em->name, MODULE_ARCH);
   if (!evas_file_path_exists(buf))
     {
	printf("[evas module] error loading the module %s. It doesnt exists\n", buf);
	free(buf);
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
	     free(buf);
	  }
	
	em->func.open(em);
	em->loaded = 1;
	return 1;
     }
   error_dl:	
     {
	char *err;
	err = dlerror();
	printf("[evas module] error loading the module %s. %s\n", buf, err);
	dlclose(handle);
	em->handle = NULL;
	free(buf);
	return 0;
     }
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
	em->loaded = 0;
     }
}

/* will dlclose all the modules loaded and free all the structs */
void
evas_module_shutdown(void)
{
   Evas_Module *em;
   
   while (evas_modules)
     {
	em = (Evas_Module *)evas_modules->data;
	evas_module_unload(em);
	if (em->name) free(em->name);
	if (em->path) free(em->path);
	if (em->data) free(em->data);
	free(evas_modules->data);
	evas_modules = evas_list_remove_list(evas_modules, evas_modules);
     }
}
