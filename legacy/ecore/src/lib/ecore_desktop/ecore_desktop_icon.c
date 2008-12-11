/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <limits.h>
#include <sys/stat.h>

#include "Ecore_Desktop.h"
#include "ecore_desktop_private.h"
#include "ecore_private.h"

//#define DEBUG 1

static char        *_ecore_desktop_icon_find0(const char *icon,
					      const char *icon_size,
					      const char *icon_theme, 
					      int *in_cache);

static int          _ecore_desktop_icon_theme_list_add(void *data,
						       const char *path);
static void         _ecore_desktop_icon_theme_destroy(Ecore_Desktop_Icon_Theme *
						      icon_theme);
static void        
_ecore_desktop_icon_theme_directory_destroy(Ecore_Desktop_Icon_Theme_Directory *
					    icon_theme_directory);
static inline void
_ecore_desktop_icon_theme_cache_check(Ecore_Desktop_Icon_Theme *icon_theme);

/* FIXME: We need a way for the client to disable searching for any of these that they don't support. */
static const char  *ext[] =
   { "", ".edj", ".png", ".svgz", ".svg", ".xpm", NULL };  /* "" is in case the icon already has an extension, search for that first. */
static int          init_count = 0;
static Eina_Hash  *icon_theme_cache = NULL;

/**
 * @defgroup Ecore_Desktop_Icon_Group icon theme Functions
 *
 * Functions that deal with freedesktop.org icon themes.
 *
 * This conforms with the freedesktop.org XDG Icon Theme Specification version 0.11
 */

/**
 * Find the path to an icon.
 *
 * Using the search algorithm specified by freedesktop.org,
 * search for an icon in the currently installed set of icon themes.
 *
 * The returned string needs to be freed eventually.
 *
 * @param   icon The name of the required icon.
 * @param   icon_size The size of the required icon.
 * @param   icon_theme The theme of the required icon.
 * @return  The full path to an icon file, or NULL.
 * @ingroup Ecore_Desktop_Icon_Group
 */

EAPI char         *
ecore_desktop_icon_find(const char *icon, const char *icon_size,
			const char *icon_theme)
{
   char           *result = NULL, *icn;
   Ecore_List     *icons;
   int             in_cache = 0;
   double          begin;

   begin = ecore_time_get();
   if (icon)
     {
        /* Easy check first, was a full path supplied? */
        if ((icon[0] == '/') && (ecore_file_exists(icon)))
           result = strdup(icon);
	else
	  {
             icons = ecore_desktop_paths_to_list(icon);
             if (icons)
	       {

                  if (icon_size == NULL)
                     icon_size = "48x48";
                  if (icon_theme == NULL)
                     icon_theme = "hicolor";
                  ecore_list_first_goto(icons);
                  while ((icn = ecore_list_next(icons)))
                    {
	               char *ext;
#ifdef DEBUG
	               fprintf(stderr, "\tTrying To Find Icon %s\n", icn);
#endif
	               ext = strrchr(icn, '.');
	               /* Check for unsupported extension */
	               if ((ext) && (!strcmp(ext, ".ico")))
		          continue;

	               result = _ecore_desktop_icon_find0(icn, icon_size, icon_theme, &in_cache);
	               if (result)
	                  break;
                    }
                  ecore_list_destroy(icons);

	       } /* if (icons) */
	  } /* if ((icon[0] == '/') && (ecore_file_exists(icon))) ; else */
     } /* if (icon) */

   if (result)
     {
        if (in_cache)
          {
             instrumentation.icons_in_cache_time += ecore_time_get() - begin;
             instrumentation.icons_in_cache++;
	  }
	else
	  {
             instrumentation.icons_time += ecore_time_get() - begin;
             instrumentation.icons++;
	  }
     }
   else
     {
        instrumentation.icons_not_found_time += ecore_time_get() - begin;
        instrumentation.icons_not_found++;
     }

   return result;
}

/** Search for an icon the fdo way.
 *
 * This complies with the freedesktop.org Icon Theme Specification version 0.7
 *
 * @param   icon The icon to search for.
 * @param   icon_size The icon size to search for.
 * @param   icon_theme The icon theme to search in.
 * @return  The full path to the found icon.
 */
static char  *
_ecore_desktop_icon_find0(const char *icon, const char *icon_size,
			  const char *icon_theme, int *in_cache)
{
   Ecore_Desktop_Icon_Theme *theme;
   char                path[PATH_MAX];
   char               *found = NULL;
   int                 wanted_size;
   int                 minimal_size = INT_MAX;
   int                 has_ext = 0;
   int                 has_icon_ext = 0;
   int                 i;
   char               *closest = NULL;
   Ecore_Desktop_Icon_Theme_Directory *directory;

   if ((icon == NULL) || (icon[0] == '\0'))
      return NULL;

   /* Check the file extension, if any. */
   found = strrchr(icon, '.');
   if (found != NULL)
     {
        has_ext = 1;
        for (i = 0; ext[i] != NULL; i++)
	  {
	     if (strcmp(found, ext[i]) == 0)
	       {
	          has_icon_ext = 1;
	          break;
	       }
	  }
        found = NULL;
     }

#ifdef DEBUG
   fprintf(stderr, "\tTrying To Find Icon %s (%s) in theme %s\n", icon,
	   icon_size, icon_theme);
#endif

   /* Get the theme description file. */
   theme = ecore_desktop_icon_theme_get(icon_theme, NULL);
#ifdef DEBUG
   printf("SEARCHING FOR %s\n", icon_theme);
#endif

   if (!theme) return NULL;
   if (!theme->Directories) goto done;

   wanted_size = atoi(icon_size);

   /* Loop through the themes directories. */
   ecore_list_first_goto(theme->Directories);
   while ((directory = ecore_list_next(theme->Directories)) != NULL)
     {
	if (directory->size)
	  {
	     int                 match = 0;
	     int                 result_size = 0;

	     /* Does this theme directory match the required icon size? */
	     switch (directory->type[0])
	       {
		case 'F':	/* Fixed. */
		   match = (wanted_size == directory->size);
		   result_size = abs(directory->size - wanted_size);
		   break;
		case 'S':	/* Scaled. */
		   match = ((directory->minimum <= wanted_size) &&
			    (wanted_size <= directory->maximum));
		   if (wanted_size < directory->minimum)
		     result_size = directory->minimum - wanted_size;
		   if (wanted_size > directory->maximum)
		     result_size = wanted_size - directory->maximum;
		   break;
		default:	/* Threshold. */
		   match = (((directory->size - directory->threshold) <= wanted_size) &&
			     (wanted_size <= (directory->size + directory->threshold)));
		   if (wanted_size < (directory->size - directory->threshold))
		     result_size = directory->minimum - wanted_size;
		   if (wanted_size > (directory->size + directory->threshold))
		     result_size = wanted_size - directory->maximum;
		   break;
	       }

             /* Do we need to check this directory? */
             if ((match) || (result_size < minimal_size))
	       {
	          /* Look for icon with all extensions. */
	          for (i = 0; ext[i] != NULL; i++)
	            {
		       /* Check if there will be an extension to check. */
		       if ((ext[i][0] == '\0') && (!has_ext))
		          continue;
		       if ((ext[i][0] != '\0') && (has_icon_ext))
		          continue;
                       if (directory->icons)
		         {
		            snprintf(path, PATH_MAX, "%s%s", icon, ext[i]);
#ifdef DEBUG
		            printf("FDO icon = %s\n", path);
#endif
                            found = eina_hash_find(directory->icons, path);
			    if (found)
			      {
			         found = strdup(found);
				 if (match)
			            *in_cache = 1;
			      }
			 }
		       else
		         {
		            snprintf(path, PATH_MAX, "%s/%s%s", directory->full_path, icon, ext[i]);
#ifdef DEBUG
		            printf("FDO icon = %s\n", path);
#endif
                            if (ecore_file_exists(path))
		               found = strdup(path);
			 }
		       if (found)
		         {
		            if (ecore_file_is_dir(found))
			      {
			         free(found);
			         found = NULL;
			      }
		            else if (match)	/* If there is a match in sizes, return the icon. */
			       goto done;
		            else if (result_size < minimal_size)	/* While we are here, figure out our next fallback strategy. */
			      {
			         minimal_size = result_size;
			         if (closest) free(closest);
			         closest = found;
			         found = NULL;
			      }
		            else
			      {
			         free(found);
			         found = NULL;
			      }
		         }
	            }   /* for (i = 0; ext[i] != NULL; i++) */
	       }   /* if ((match) || (result_size < minimal_size)) */
	  }   /* if (directory->size) */
     }   /* while ((directory = ecore_list_next(directory_paths)) != NULL) */

   if (!found)
     {
	/* Fall back strategy #1, look for closest size in this theme. */
	found = closest;
	if (found)
	  {
	    closest = NULL;
	    goto done;
	  }

	/* Fall back strategy #2, Try again with the parent themes. */
        if (!theme->hicolor)
	  {
	     if (theme->Inherits)
	       {
	          char *inherits;

	          ecore_list_first_goto(theme->Inherits);
	          while ((inherits = ecore_list_next(theme->Inherits)) != NULL)
	            {
		       found = _ecore_desktop_icon_find0(icon, icon_size, inherits, in_cache);
		       if (found) goto done;
	            }
	       }
	     else   /* Fall back strategy #3, Try the default hicolor theme. */
	       {
	          found = _ecore_desktop_icon_find0(icon, icon_size, "hicolor", in_cache);
	          if (found) goto done;
	       }
	  }

	/* Fall back strategy #4, Just search in the base of the icon directories. */
	for (i = 0; ext[i] != NULL; i++)
	  {
	     /* Check if there will be an extension to check. */
	     if ((ext[i][0] == '\0') && (!has_ext))
	        continue;
             if ((ext[i][0] != '\0') && (has_icon_ext))
	        continue;
	     snprintf(path, PATH_MAX, "%s%s", icon, ext[i]);
#ifdef DEBUG
	     printf("FDO icon = %s\n", path);
#endif
	     found = ecore_desktop_paths_file_find(ecore_desktop_paths_icons, path, 0, NULL, NULL);
	     if (found)
	       {
		  if (ecore_file_is_dir(found))
		    {
		       free(found);
		       found = NULL;
		    }
		  else
		    goto done;
	       }
	  }
     }

done:
   if (closest) free(closest);
   ecore_desktop_icon_theme_destroy(theme);

   return found;
}

Eina_Hash*
ecore_desktop_icon_theme_list(void)
{
   static int          loaded = 0;
   if (!loaded)
     {
	char *tmp;
	tmp = ecore_desktop_paths_file_find(ecore_desktop_paths_icons, "index.theme", 2,
					    _ecore_desktop_icon_theme_list_add, NULL);
	loaded = 1;
	free(tmp);
     }
   return icon_theme_cache;
}

static int
_ecore_desktop_icon_theme_list_add(void *data __UNUSED__, const char *path)
{
   char                icn[PATH_MAX];

   snprintf(icn, PATH_MAX, "%sindex.theme", path);
   if (ecore_desktop_icon_theme_get(icn, NULL))
      return 1;			/* Should stop it from recursing this directory, but let it continue searching the next. */
   return 0;
}

/**
 * Setup what ever needs to be setup to support ecore_desktop_icon.
 *
 * There are internal structures that are needed for ecore_desktop_icon
 * functions to operate, this sets them up.
 *
 * @ingroup Ecore_Desktop_Icon_Group
 */
EAPI int
ecore_desktop_icon_init()
{
   if (++init_count != 1)
      return init_count;

   if (!icon_theme_cache)
     icon_theme_cache = eina_hash_string_superfast_new(_ecore_desktop_icon_theme_destroy);

   return init_count;
}

/**
 * Tear down what ever needs to be torn down to support ecore_desktop_ycon.
 *
 * There are internal structures that are needed for ecore_desktop_icon
 * functions to operate, this tears them down.
 *
 * @ingroup Ecore_Desktop_Icon_Group
 */
EAPI int
ecore_desktop_icon_shutdown()
{
   if (--init_count != 0)
      return init_count;

   if (icon_theme_cache)
     {
	eina_hash_free(icon_theme_cache);
	icon_theme_cache = NULL;
     }

   return init_count;
}

/**
 * Get the contents of an index.theme file.
 *
 * Everything that is in the index.theme file is returned in the
 * data member of the Ecore_Desktop_Icon_Theme structure, it's an Eina_Hash 
 * as returned by ecore_desktop_ini_get().  Some of the data in the
 * index.theme file is decoded into specific members of the returned 
 * structure.
 *
 * Use ecore_desktop_icon_theme_destroy() to free this structure.
 * 
 * @param   icon_theme Name of the icon theme, or full path to the index.theme file.
 * @param   lang Language to use, or NULL for default.
 * @return  An Ecore_Desktop_Icon_Theme containing the files contents.
 * @ingroup Ecore_Desktop_Icon_Group
 */
Ecore_Desktop_Icon_Theme *
ecore_desktop_icon_theme_get(const char *icon_theme, const char *lang __UNUSED__)
{
   Ecore_Desktop_Icon_Theme *result = NULL;
   char *theme_path = NULL, *theme_dir = NULL;
   const char         *value;
   Ecore_List         *Directories;
   char               *directory;

   if (icon_theme[0] == '/')
      {
         theme_path = strdup(icon_theme);
	 theme_dir = ecore_file_dir_get(theme_path);
	 if (theme_dir)
	    icon_theme = ecore_file_file_get(theme_dir);
#ifdef DEBUG
	 printf("LOADING THEME %s  -   %s\n", icon_theme, theme_path);
#endif
      }

   result = eina_hash_find(icon_theme_cache, icon_theme);
   if (result) goto done;
   if (!theme_dir)
     {
	char icn[PATH_MAX];

	snprintf(icn, PATH_MAX, "%s/index.theme", icon_theme);
#ifdef DEBUG
	printf("SEARCHING FOR %s\n", icn);
#endif
	theme_path = ecore_desktop_paths_file_find(ecore_desktop_paths_icons, icn,
						   2, NULL, NULL);
	if (!theme_path)  goto error;
        theme_dir = ecore_file_dir_get(theme_path);
     }
   if (!theme_path) goto error;
   result = calloc(1, sizeof(Ecore_Desktop_Icon_Theme));
   if (!result) goto error;
   result->data = ecore_desktop_ini_get(theme_path);
   if (!result->data) goto error;
   result->group = eina_hash_find(result->data, "Icon Theme");
   if (!result->group) goto error;


   if ((strcmp(icon_theme, "hicolor") == 0))
      result->hicolor = 1;

   /* According to the spec, name and comment are required, but we can fake those easily enough. */
   value = eina_hash_find(result->group, "Name");
   if (!value) value = icon_theme;
   result->name = strdup(value);
   value = eina_hash_find(result->group, "Comment");
   if (!value) value = "No comment provided.";
   result->comment = strdup(value);
   value = eina_hash_find(result->group, "Inherits");
   if (value)
     {
	result->inherits = strdup(value);
	if (result->inherits)
	  result->Inherits = ecore_desktop_paths_to_list(result->inherits);
     }
   value = eina_hash_find(result->group, "Example");
   if (!value) value = "exec";
   result->example = strdup(value);
   value = eina_hash_find(result->group, "Directories");
   /* FIXME: Directories is also required, don't feel like faking it for now. */
   if (!value) goto error;
   result->directories = strdup(value);
   Directories = ecore_desktop_paths_to_list(result->directories);
   if (!Directories) goto error;
   result->Directories = ecore_list_new();
   if (!result->Directories) goto error;
   ecore_list_free_cb_set(result->Directories,
			  ECORE_FREE_CB(_ecore_desktop_icon_theme_directory_destroy));
   ecore_list_first_goto(Directories);
   while ((directory = ecore_list_next(Directories)) != NULL)
     {
	Eina_Hash         *sub_group;
	Ecore_Desktop_Icon_Theme_Directory *dir;

	/* Get the details for this theme directory. */
	sub_group = eina_hash_find(result->data, directory);
	dir = calloc(1, sizeof (Ecore_Desktop_Icon_Theme_Directory));
	if ((dir) && (sub_group))
	  {
	     const char *size, *minsize, *maxsize, *threshold;
             char full_path[PATH_MAX];

             dir->path = strdup(directory);
	     snprintf(full_path, PATH_MAX, "%s/%s", theme_dir, directory);
             dir->full_path = strdup(full_path);
	     value = eina_hash_find(sub_group, "Type");
	     if (!value)
	       value = "Threshold";
	     dir->type = strdup(value);
	     size = eina_hash_find(sub_group, "Size");
	     minsize = eina_hash_find(sub_group, "MinSize");
	     maxsize = eina_hash_find(sub_group, "MaxSize");
	     threshold = eina_hash_find(sub_group, "Threshold");
	     if (size)
	       {
		  if (!minsize)
		    minsize = size;
		  if (!maxsize)
		    maxsize = size;
		  if (!threshold)
		    threshold = "2";
		  dir->minimum = atoi(minsize);
		  dir->maximum = atoi(maxsize);
		  dir->threshold = atoi(threshold);

		  dir->size = atoi(size);
		  ecore_list_append(result->Directories, dir);
	       }
	     else
	       _ecore_desktop_icon_theme_directory_destroy(dir);
	  }
	else if (dir)
	  _ecore_desktop_icon_theme_directory_destroy(dir);
     }
   ecore_list_destroy(Directories);

   /* This passes the basic validation tests, mark it as real and cache it. */
   result->path = strdup(theme_path);
   eina_hash_add(icon_theme_cache, icon_theme, result);
   eina_hash_free(result->data);
   result->data = NULL;
   result->group = NULL;

done:
   if (theme_dir)  free(theme_dir);
   if (theme_path) free(theme_path);

   /* Cache the directories. */
   _ecore_desktop_icon_theme_cache_check(result);
   return result;

error:
   if (theme_dir)  free(theme_dir);
   if (theme_path) free(theme_path);
   if (result)
     {
	if (result->data) eina_hash_free(result->data);
	_ecore_desktop_icon_theme_destroy(result);
     }
   return NULL;
}

/**
 * Free whatever resources are used by an Ecore_Desktop_Icon_Theme.
 *
 * There are internal resources used by each Ecore_Desktop_Icon_Theme
 * This releases those resources.
 *
 * @param  icon_theme  An Ecore_Desktop_Icon_Theme.
 * @ingroup Ecore_Desktop_Icon_Group
 */
void
ecore_desktop_icon_theme_destroy(Ecore_Desktop_Icon_Theme * icon_theme)
{
   /* This is just a dummy, because these structures are cached. */
   /* Later versions of the cache may reference count, then this will be useful. */

   icon_theme = NULL;
}

static void
_ecore_desktop_icon_theme_destroy(Ecore_Desktop_Icon_Theme * icon_theme)
{
   if (icon_theme->path)
      free(icon_theme->path);
   if (icon_theme->name)
      free(icon_theme->name);
   if (icon_theme->comment)
      free(icon_theme->comment);
   if (icon_theme->example)
      free(icon_theme->example);
   if (icon_theme->inherits)
      free(icon_theme->inherits);
   if (icon_theme->directories)
      free(icon_theme->directories);
   if (icon_theme->Directories)
      ecore_list_destroy(icon_theme->Directories);
   if (icon_theme->Inherits)
      ecore_list_destroy(icon_theme->Inherits);
   free(icon_theme);
}

static void
_ecore_desktop_icon_theme_directory_destroy(Ecore_Desktop_Icon_Theme_Directory *
					    icon_theme_directory)
{
   if (icon_theme_directory->path)
      free(icon_theme_directory->path);
   if (icon_theme_directory->full_path)
      free(icon_theme_directory->full_path);
   if (icon_theme_directory->type)
      free(icon_theme_directory->type);
   if (icon_theme_directory->icons)
      eina_hash_free(icon_theme_directory->icons);
   free(icon_theme_directory);
}

static inline void
_ecore_desktop_icon_theme_cache_check(Ecore_Desktop_Icon_Theme *icon_theme)
{
   /* The spec has this to say -
    *
    * "The algorithm as described in this document works by always looking up 
    * filenames in directories (a stat in unix terminology). A good 
    * implementation is expected to read the directories once, and do all 
    * lookups in memory using that information.
    *
    * "This caching can make it impossible for users to add icons without having 
    * to restart applications. In order to handle this, any implementation that 
    * does caching is required to look at the mtime of the toplevel icon 
    * directories when doing a cache lookup, unless it already did so less than 
    * 5 seconds ago. This means that any icon editor or theme installation 
    * program need only to change the mtime of the the toplevel directory where 
    * it changed the theme to make sure that the new icons will eventually get 
    * used."
    *
    * The phrase "toplevel icon directories" is ambigous, but I guess they mean 
    * the directory where the index.theme file lives.
    */

   struct stat         st;
   int                 clear = 0;

   if (ecore_time_get() > (icon_theme->last_checked + 5.0))
     {
	if (stat(icon_theme->path, &st) >= 0)
	  {
	     icon_theme->last_checked = ecore_time_get();
	     if (st.st_mtime > icon_theme->mtime)
	       {
	          clear = 1;
		  icon_theme->mtime = st.st_mtime;
	       }
	  }
     }

   if (clear)
     {
        Ecore_Desktop_Icon_Theme_Directory *dir;
        char full_path[PATH_MAX];

        ecore_list_first_goto(icon_theme->Directories);
        while ((dir = ecore_list_next(icon_theme->Directories)) != NULL)
          {
             if (dir->icons)
	       {
                   eina_hash_free(dir->icons);
		   dir->icons = NULL;
	       }
             dir->icons = eina_hash_string_superfast_new(NULL);
             if (dir->icons)
               {
	          Ecore_List *files;

                  files = ecore_file_ls(dir->full_path);
                  if (files)
                    {
                       const char *file;

                       while ((file = ecore_list_next(files)))
                         {
                            snprintf(full_path, PATH_MAX, "%s/%s", dir->full_path, file);
			    eina_hash_add(dir->icons, file, strdup(full_path));
                         }
                       ecore_list_destroy(files);
                    }
	       }
          }
     }
}
