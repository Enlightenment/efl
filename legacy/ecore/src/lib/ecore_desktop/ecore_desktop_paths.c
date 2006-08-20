/*
 * This conforms with the freedesktop.org XDG Base Directory Specification version 0.6
 */

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <strings.h>

#include <Ecore.h>

#include "Ecore_Desktop.h"
#include "ecore_desktop_private.h"


/* This really slows things down and no doubt drags in some KDE bloat at start up.
 * To work around this, I add a few extra things to try in the _ecore_desktop_paths_get()
 * calls below.
 *
#define KDE_SUPPORT 1
 */

/* This is nowhere near as bloated and slow as the KDE stuff, but worthwhile 
 * making it optional anyway.
 *
#define GNOME_SUPPORT 1
 */

/* FIXME: remove those two defines, preload a cache with the guesses, whenever 
 * we have a few seconds of idle time, run the gnome and kde config proggies 
 * and correct those guesses.
 */

static int                  init_count = 0;


static Ecore_List  *_ecore_desktop_paths_get(char *before, char *env_home,
					     char *env, char *env_home_default,
					     char *env_default, char *type,
					     char *gnome_extra, char *kde);
static void         _ecore_desktop_paths_massage_path(char *path, char *home,
						      char *first,
						      char *second);
static void         _ecore_desktop_paths_check_and_add(Ecore_List * paths,
						       char *path);

static Ecore_List         *gnome_data = NULL;
static char               *home;


#if defined GNOME_SUPPORT || defined KDE_SUPPORT
struct _config_exe_data
{
   char               *home;
   Ecore_List         *paths, *types;
   int                 done;
};

static void         _ecore_desktop_paths_exec_config(Ecore_List * paths,
						     char *home,
						     Ecore_List * extras,
						     char *cmd);

static int          _ecore_desktop_paths_cb_exe_exit(void *data, int type,
						     void *event);

static Ecore_Event_Handler *exit_handler = NULL;
#endif


EAPI int
ecore_desktop_paths_init()
{
   if (++init_count != 1) return init_count;

   /* FIXME: Keep track of any loose strdups in a list, so that we can free them at shutdown time. */

#if defined GNOME_SUPPORT || defined KDE_SUPPORT
   exit_handler =
      ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
			      _ecore_desktop_paths_cb_exe_exit, NULL);
#endif

   gnome_data = ecore_list_new();
   home = ecore_desktop_home_get();
   if (home)
     {
	int                 last;

	/* Strip trailing slash of home. */
	last = strlen(home) - 1;
	if ((last >= 0) && (home[last] == '/'))
	   home[last] = '\0';
     }

   if (gnome_data)
      {
#ifdef GNOME_SUPPORT
         if (exit_handler)
            {
	       ecore_list_set_free_cb(gnome_data, free);
	       _ecore_desktop_paths_exec_config(gnome_data, home, NULL,
					 "gnome-config --datadir");
            }
#else
             Ecore_List         *config_list;

	     config_list = ecore_desktop_paths_to_list("/opt/gnome/share");
	     if (config_list)
	       {
		  char               *this_config;
                  char path[PATH_MAX];

		  ecore_list_goto_first(config_list);
		  while ((this_config = ecore_list_next(config_list)) != NULL)
		    {

			_ecore_desktop_paths_massage_path(path, home,
						      this_config,
						      NULL);
			_ecore_desktop_paths_check_and_add(gnome_data, path);
		    }
		  E_FN_DEL(ecore_list_destroy, config_list);
	       }
#endif
     }

   if (!ecore_desktop_paths_desktops)
     {
	ecore_desktop_paths_desktops =
	   _ecore_desktop_paths_get(NULL, "XDG_DATA_HOME", "XDG_DATA_DIRS",
				    "~/.local/share:~/.kde/share",
				    "/usr/local/share:/usr/share",
				    "applications:applnk",
				    "dist/desktop-files:dist/short-menu:gnome/apps",
				    "xdgdata-apps:apps");
	_ecore_desktop_paths_check_and_add(ecore_desktop_paths_desktops,
					   "/usr/local/share/update-desktop-files/templates");
	_ecore_desktop_paths_check_and_add(ecore_desktop_paths_desktops,
					   "/usr/share/update-desktop-files/templates");
     }
   if (!ecore_desktop_paths_kde_legacy)
     {
	char                temp[PATH_MAX], *path;
	Ecore_List         *temp_list;

#ifdef KDE_SUPPORT
	ecore_desktop_paths_kde_legacy =
	   _ecore_desktop_paths_get(NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				    "apps");
#else
	ecore_desktop_paths_kde_legacy =
	   _ecore_desktop_paths_get(NULL, "XDG_DATA_HOME", "XDG_DATA_DIRS",
				    "~/.local/share:~/.kde/share",
				    "/usr/local/share:/usr/share",
				    "applnk",
				    NULL,
				    "apps");
#endif
	if (ecore_desktop_paths_kde_legacy)
	  {
	     /* Copy it, cause Ecore_List walks can not be nested. */
	     temp_list = ecore_list_new();
	     if (temp_list)
	       {
		  ecore_list_goto_first(ecore_desktop_paths_kde_legacy);
		  while ((path =
			  ecore_list_next(ecore_desktop_paths_kde_legacy)) !=
			 NULL)
		     ecore_list_append(temp_list, path);

		  ecore_list_goto_first(temp_list);
		  while ((path = ecore_list_next(temp_list)) != NULL)
		    {
		       char               *t1, *t2;

		       t1 = rindex(path, '/');
		       *t1 = '\0';
		       t2 = rindex(path, '/');
		       *t2 = '\0';
		       sprintf(temp, "%s/apps/kappfinder/apps/", path);
		       *t2 = '/';
		       *t1 = '/';
		       _ecore_desktop_paths_check_and_add
			  (ecore_desktop_paths_kde_legacy, temp);
		    }
	       }
	  }
     }
   if (!ecore_desktop_paths_icons)
     {
	char               *gnome;

	ecore_desktop_paths_icons =
	   _ecore_desktop_paths_get("~/.icons", "XDG_DATA_HOME",
				    "XDG_DATA_DIRS", "~/.local/share:~/.kde/share",
				    "/usr/local/share:/usr/share", "icons:pixmaps",
				    "dist/icons", "icon:pixmap");
	_ecore_desktop_paths_check_and_add(ecore_desktop_paths_icons,
					   "/usr/local/share/pixmaps/");
	_ecore_desktop_paths_check_and_add(ecore_desktop_paths_icons,
					   "/usr/share/pixmaps/");
	gnome = getenv("GNOME_ICON_PATH");
	if (gnome)
	   _ecore_desktop_paths_check_and_add(ecore_desktop_paths_icons, gnome);
     }
   if (!ecore_desktop_paths_menus)
      ecore_desktop_paths_menus =
	 _ecore_desktop_paths_get(NULL, "XDG_CONFIG_HOME", "XDG_CONFIG_DIRS",
				  "~/.config", "/etc/xdg", "menus", NULL,
				  "xdgconf-menu");
   if (!ecore_desktop_paths_directories)
      ecore_desktop_paths_directories =
	 _ecore_desktop_paths_get(NULL, "XDG_DATA_HOME", "XDG_DATA_DIRS",
				  "~/.local/share:~/.kde/share",
				  "/usr/local/share:/usr/share",
				  "desktop-directories", "gnome/vfolders",
				  "xdgdata-dirs");
   if (!ecore_desktop_paths_config)
      ecore_desktop_paths_config =
	 _ecore_desktop_paths_get(NULL, "XDG_CONFIG_HOME", "XDG_CONFIG_DIRS",
				  "~/.config", "/etc/xdg", "", NULL, NULL);

#if defined GNOME_SUPPORT || defined KDE_SUPPORT
   if (exit_handler)
      ecore_event_handler_del(exit_handler);
#endif

   return init_count;
}

EAPI int
ecore_desktop_paths_shutdown()
{
   if (--init_count != 0) return init_count;

   E_FN_DEL(ecore_list_destroy, ecore_desktop_paths_menus);
   E_FN_DEL(ecore_list_destroy, ecore_desktop_paths_directories);
   E_FN_DEL(ecore_list_destroy, ecore_desktop_paths_desktops);
   E_FN_DEL(ecore_list_destroy, ecore_desktop_paths_icons);

   return init_count;
}

/** Search for a file in fdo compatible locations.
 *
 * This will search through all the diretories of a particular type, looking 
 * for the file.  It will recurse into subdirectories.  If func is NULL, then
 * only the first file found will be returned.  If func is defined, then each
 * file found will be passed to func, until func returns 1.
 *
 * The returned string will have to be freed eventually.
 *
 * @param   type The type of directories to search.
 * @param   file The file to search for.
 * @param   sub Levels of sub directories to search, -1 = all, 0 = none.
 * @param   func A function to call for each file found.
 * @param   data A pointer to pass on to func.
 */
char               *
ecore_desktop_paths_file_find(Ecore_List * paths, const char *file, int sub,
				    int (*func) (void *data, const char *path),
				    void *data)
{
   char               *path = NULL, *this_path;
   char                temp[PATH_MAX];
   struct stat         path_stat;

   if (paths)
     {
	ecore_list_goto_first(paths);
	while ((this_path = ecore_list_next(paths)) != NULL)
	  {
	     snprintf(temp, PATH_MAX, "%s%s", this_path, file);
	     if (stat(temp, &path_stat) == 0)
	       {
		   if (path)
		      free(path);
		  path = strdup(temp);
		  if (func)
		     if (func(data, temp))
			break;
	       }
	     else if (sub != 0)
		path =
		   ecore_desktop_paths_recursive_search(this_path, file, sub, NULL,
							func, data);
	     if (path && (!func))
		break;
	  }
     }

   return path;
}

/*  We need -
config file full of paths
menus=pathlist
desktops=pathlist
directories=pathlist
icons=pathlist
*/

static Ecore_List  *
_ecore_desktop_paths_get(char *before, char *env_home, char *env,
			 char *env_home_default, char *env_default, char *type,
			 char *gnome_extra, char *kde)
{
   Ecore_List         *paths = NULL;
   Ecore_List         *types = NULL;
   Ecore_List         *gnome_extras = NULL;
#ifdef KDE_SUPPORT
   Ecore_List         *kdes = NULL;
#endif

   /* Don't sort them, as they are in preferred order from each source. */
   /* Merge the results, there are probably some duplicates. */

   if (type)
      types = ecore_desktop_paths_to_list(type);
   if (gnome_extra)
      gnome_extras = ecore_desktop_paths_to_list(gnome_extra);
#ifdef KDE_SUPPORT
   if (kde)
      kdes = ecore_desktop_paths_to_list(kde);
#endif

   paths = ecore_list_new();
   if (paths)
     {
	char                path[PATH_MAX];
	Ecore_List         *env_list;

	ecore_list_set_free_cb(paths, free);
	if (before)
	  {
	     Ecore_List         *befores;

	     befores = ecore_desktop_paths_to_list(before);
	     if (befores)
	       {
		  char               *this_before;

		  ecore_list_goto_first(befores);
		  while ((this_before = ecore_list_next(befores)) != NULL)
		    {
		       _ecore_desktop_paths_massage_path(path, home,
							 this_before, NULL);
		       _ecore_desktop_paths_check_and_add(paths, path);
		    }
		  E_FN_DEL(ecore_list_destroy, befores);
	       }
	  }

	if (env_home)
	  {
	     char               *value;

	     value = getenv(env_home);
	     if ((value == NULL) || (value[0] == '\0'))
		value = env_home_default;
	     env_list = ecore_desktop_paths_to_list(value);
	     if (env_list && types)
	       {
		  char               *this_env, *this_type;

		  ecore_list_goto_first(env_list);
		  while ((this_env = ecore_list_next(env_list)) != NULL)
		    {
		       ecore_list_goto_first(types);
		       while ((this_type = ecore_list_next(types)) != NULL)
			 {
			    _ecore_desktop_paths_massage_path(path, home,
							      this_env,
							      this_type);
			    _ecore_desktop_paths_check_and_add(paths, path);
			 }
		    }
		  E_FN_DEL(ecore_list_destroy, env_list);
	       }
	  }

	if (env)
	  {
	     char               *value;

	     value = getenv(env);
	     if ((value == NULL) || (value[0] == '\0'))
		value = env_default;
	     env_list = ecore_desktop_paths_to_list(value);
	     if (env_list && types)
	       {
		  char               *this_env, *this_type;

		  ecore_list_goto_first(env_list);
		  while ((this_env = ecore_list_next(env_list)) != NULL)
		    {
		       ecore_list_goto_first(types);
		       while ((this_type = ecore_list_next(types)) != NULL)
			 {
			    _ecore_desktop_paths_massage_path(path, home,
							      this_env,
							      this_type);
			    _ecore_desktop_paths_check_and_add(paths, path);
			 }
		    }
		  E_FN_DEL(ecore_list_destroy, env_list);
	       }
	  }

	/*
	 * Get the pathlist from the config file - type=pathlist
	 *   for each path in config
	 *      if it is already in paths, skip it
	 *      if it exists, add it to end of paths
	 */

	if (gnome_data && types)
	  {
	     char               *this_gnome, *this_type;

	     ecore_list_goto_first(gnome_data);
	     while ((this_gnome = ecore_list_next(gnome_data)) != NULL)
	       {
		  ecore_list_goto_first(types);
		  while ((this_type = ecore_list_next(types)) != NULL)
		    {
		       _ecore_desktop_paths_massage_path(path, home, this_gnome,
							 this_type);
		       _ecore_desktop_paths_check_and_add(paths, path);
		    }
	       }
	  }
	if (gnome_data && gnome_extras)
	  {
	     char               *this_gnome, *this_type;

	     ecore_list_goto_first(gnome_data);
	     while ((this_gnome = ecore_list_next(gnome_data)) != NULL)
	       {
		  ecore_list_goto_first(gnome_extras);
		  while ((this_type = ecore_list_next(gnome_extras)) != NULL)
		    {
		       _ecore_desktop_paths_massage_path(path, home, this_gnome,
							 this_type);
		       _ecore_desktop_paths_check_and_add(paths, path);
		    }
	       }
	  }

#ifdef KDE_SUPPORT
	if ((exit_handler != NULL) && (kdes != NULL))
	  {
	     char               *this_kde;

	     ecore_list_goto_first(kdes);
	     while ((this_kde = ecore_list_next(kdes)) != NULL)
	       {
		  char                cmd[128];

		  sprintf(cmd, "kde-config --path %s", this_kde);
		  _ecore_desktop_paths_exec_config(paths, home, NULL, cmd);
	       }
	  }
#endif
     }

#ifdef KDE_SUPPORT
   E_FN_DEL(ecore_list_destroy, kdes);
#endif
   E_FN_DEL(ecore_list_destroy, gnome_extras);
   E_FN_DEL(ecore_list_destroy, types);

   return paths;
}

static void
_ecore_desktop_paths_massage_path(char *path, char *home, char *first,
				  char *second)
{
   int                 last;

   /* Strip traling slash of first. */
   last = strlen(first) - 1;
   if ((last >= 0) && (first[last] == '/'))
      first[last] = '\0';

   if (second)
     {
	/* Strip traling slash of second. */
	last = strlen(second) - 1;
	if ((last >= 0) && (second[last] == '/'))
	   second[last] = '\0';
     }

   if ((second) && (second[0] != '\0'))
     {
	if (first[0] == '~')
	   sprintf(path, "%s%s/%s/", home, &first[1],
		   &second[(second[0] == '/') ? 1 : 0]);
	else
	   sprintf(path, "%s/%s/", first, &second[(second[0] == '/') ? 1 : 0]);
     }
   else
     {
	if (first[0] == '~')
	   sprintf(path, "%s%s/", home, &first[1]);
	else
	   sprintf(path, "%s/", first);
     }
}

static void
_ecore_desktop_paths_check_and_add(Ecore_List * paths, char *path)
{
   struct stat         path_stat;
   char               *this_path;

   if (paths == NULL)
     {
	paths = ecore_list_new();
	if (paths == NULL)
	   return;
	ecore_list_set_free_cb(paths, free);
     }

   /* Check if we have it already. */
   ecore_list_goto_first(paths);
   while ((this_path = ecore_list_next(paths)) != NULL)
     {
	if (strcmp(path, this_path) == 0)
	   return;
     }

   /* Check if the path exists. */
   if ((stat(path, &path_stat) == 0) && (S_ISDIR(path_stat.st_mode)))
      ecore_list_append(paths, strdup(path));
}

char               *
ecore_desktop_paths_recursive_search(const char *path, const char *file, int sub,
				     int (*dir_func) (void *data,
						      const char *path),
				     int (*func) (void *data, const char *path),
				     void *data)
{
   char               *fpath = NULL;
   DIR                *dir = NULL;


   if ((sub != 0) && (sub != -1))
         sub -= 1;

   dir = opendir(path);

   if (dir != NULL)
     {
	struct dirent      *script;

	while ((script = readdir(dir)) != NULL)
	  {
	     struct stat         script_stat;
	     char                info_text[PATH_MAX];

	     sprintf(info_text, "%s%s", path, script->d_name);
	     if ((stat(info_text, &script_stat) == 0))
	       {
		  if (S_ISDIR(script_stat.st_mode))
		    {
		       if ((strcmp(basename(info_text), ".") != 0)
			   && (strcmp(basename(info_text), "..") != 0))
			 {
			    snprintf(info_text, PATH_MAX, "%s%s/", path, script->d_name);
			    if (dir_func)
			       if (dir_func(data, info_text))
				  break;
			    if (sub != 0)
			       fpath =
			          ecore_desktop_paths_recursive_search(info_text,
								       file, sub,
								       dir_func,
								       func, data);
			 }
		    }
		  else
		    {
		       if (file)
			 {
			    if (strcmp(basename(info_text), file) == 0)
			      {
			         if (fpath)
				    free(fpath);
				 fpath = strdup(info_text);
				 if (func)
				    if (func(data, path))
				       break;
			      }
			 }
		       else
			 {
			    if (func)
			       if (func(data, info_text))
				  break;
			 }
		    }
		  if (fpath && (!func))
		     break;
	       }
	  }
	closedir(dir);
     }

   return fpath;
}

#if defined GNOME_SUPPORT || defined KDE_SUPPORT
static void
_ecore_desktop_paths_exec_config(Ecore_List * paths, char *home,
				 Ecore_List * extras, char *cmd)
{
   Ecore_Exe          *exe;
   struct _config_exe_data ced;

   ced.home = home;
   ced.paths = paths;
   ced.types = extras;
   ced.done = 0;
   exe =
      ecore_exe_pipe_run(cmd,
			 ECORE_EXE_PIPE_AUTO | ECORE_EXE_PIPE_READ |
			 ECORE_EXE_PIPE_READ_LINE_BUFFERED, &ced);
   if (exe)
     {
	ecore_exe_tag_set(exe, "genmenu/fdo");
	while (ced.done == 0)
	  {
	     /* FIXME: raster is paranoid.  If too much time passes, give up.
	      * Or find a way to let the usual event loop shit do this without 
	      * spinning our wheels.  On the other hand, these are quick 
	      * commands, and we NEED this data before we can continue.  On 
	      * the gripping hand, some tweaking of the stuff searched for not 
	      * only gets rid of the need for this, but also speeds things up 
	      * drastically.
	      */
	     ecore_main_loop_iterate();
	     usleep(10);
	  }
     }
}

static int
_ecore_desktop_paths_cb_exe_exit(void *data, int type, void *event)
{
   Ecore_Exe_Event_Del *ev;
   Ecore_List         *paths;
   Ecore_List         *config_list;
   Ecore_Exe_Event_Data *read;
   struct _config_exe_data *ced;
   char               *value;
   char                path[PATH_MAX];

   ev = event;
   if (!ev->exe)
      return 1;
   value = ecore_exe_tag_get(ev->exe);
   if ((!value) || (strcmp(value, "genmenu/fdo")) != 0)
      return 1;
   ced = ecore_exe_data_get(ev->exe);
   if (!ced)
      return 1;
   paths = ced->paths;
   if (!paths)
      return 1;

   read = ecore_exe_event_data_get(ev->exe, ECORE_EXE_PIPE_READ);
   if ((read) && (read->lines[0].line))
     {
	value = read->lines[0].line;
	if (value)
	  {
	     config_list = ecore_desktop_paths_to_list(value);
	     if (config_list)
	       {
		  char               *this_config, *this_type;

		  ecore_list_goto_first(config_list);
		  while ((this_config = ecore_list_next(config_list)) != NULL)
		    {
		       if (ced->types)
			 {
			    ecore_list_goto_first(ced->types);
			    while ((this_type =
				    ecore_list_next(ced->types)) != NULL)
			      {
				 _ecore_desktop_paths_massage_path(path,
								   ced->home,
								   this_config,
								   this_type);
				 _ecore_desktop_paths_check_and_add(paths,
								    path);
			      }
			 }
		       else
			 {
			    _ecore_desktop_paths_massage_path(path, ced->home,
							      this_config,
							      NULL);
			    _ecore_desktop_paths_check_and_add(paths, path);
			 }
		    }
		  E_FN_DEL(ecore_list_destroy, config_list);
	       }
	  }
     }
   ced->done = 1;
   return 1;
}
#endif

/** Split a list of paths into an Ecore_Hash.
 *
 * The list of paths can use any one of ;:, to seperate the paths.
 * You can also escape the :;, with \.
 *
 * FIXME: The concept here is still buggy, but it should do for now.
 * Also, it writes to the input string, this may be bad.
 *
 * @param   paths A list of paths.
 */
Ecore_Hash         *
ecore_desktop_paths_to_hash(const char *paths)
{
   Ecore_Hash         *result;
   char               *path;

   result = ecore_hash_new(ecore_str_hash, ecore_str_compare);
   if (result)
     {
	ecore_hash_set_free_key(result, free);
	ecore_hash_set_free_value(result, free);

	if (paths)
	  {
	     char               *start, *end, temp;
	     int                 finished = 0;

	     path = strdup(paths);
	     if (path)
	        {
	           end = path;
	           while (!finished)
	             {
		        start = end;
		        do		/* FIXME: There is probably a better way to do this. */
		          {
		             while ((*end != ';') && (*end != ':') && (*end != ',')
			            && (*end != '\0'))
			        end++;
		          }
		        while ((end != path) && (*(end - 1) == '\\') && (*end != '\0'));	/* Ignore any escaped ;:, */
		        /* FIXME: We still need to unescape it now. */
		        temp = *end;
		        if (*end == '\0')
		           finished = 1;
		        else
		           *end = '\0';
		        ecore_hash_set(result, strdup(start), strdup(start));
		        if ((*end) != temp)
		           *end = temp;
		        end++;
	             }
	           free(path);
	       }
	  }
     }
   return result;
}

/** Split a list of paths into an Ecore_Hash.
 *
 * The list of paths can use any one of ;:, to seperate the paths.
 * You can also escape the :;, with \.
 *
 * FIXME: The concept here is still buggy, but it should do for now.
 * Also, it writes to the input string, this may be bad.
 *
 * @param   paths A list of paths.
 */
Ecore_List         *
ecore_desktop_paths_to_list(const char *paths)
{
   Ecore_List         *result;
   char               *path;

   result = ecore_list_new();
   if (result)
     {
	ecore_list_set_free_cb(result, free);

	if (paths)
	  {
	     char               *start, *end, temp;
	     int                 finished = 0;

	     path = strdup(paths);
	     if (path)
	        {
	           end = path;
	           while (!finished)
	             {
		        start = end;
		        do		/* FIXME: There is probably a better way to do this. */
		          {
		             while ((*end != ';') && (*end != ':') && (*end != ',')
			            && (*end != '\0'))
			        end++;
		          }
		        while ((end != path) && (*(end - 1) == '\\') && (*end != '\0'));	/* Ignore any escaped ;:, */
		        /* FIXME: We still need to unescape it now. */
		        temp = *end;
		        if (*end == '\0')
		           finished = 1;
		        else
		           *end = '\0';
		        ecore_list_append(result, strdup(start));
		        if ((*end) != temp)
		           *end = temp;
		        end++;
	             }
	           free(path);
		}
	  }
     }
   return result;
}
