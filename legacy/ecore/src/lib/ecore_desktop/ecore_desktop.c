/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Ecore_Desktop.h"
#include "ecore_desktop_private.h"

Ecore_List         *ecore_desktop_paths_config = NULL;
Ecore_List         *ecore_desktop_paths_menus = NULL;
Ecore_List         *ecore_desktop_paths_directories = NULL;
Ecore_List         *ecore_desktop_paths_desktops = NULL;
Ecore_List         *ecore_desktop_paths_icons = NULL;
Ecore_List         *ecore_desktop_paths_kde_legacy = NULL;
Ecore_List         *ecore_desktop_paths_xsessions = NULL;

extern int          reject_count, not_over_count;

static int          init_count = 0;

static Ecore_Hash  *ini_file_cache;
static Ecore_Hash  *desktop_cache;

void _ecore_desktop_destroy(Ecore_Desktop * desktop);


/**
 * @defgroup Ecore_Desktop_Main_Group .desktop file Functions
 *
 * Functions that deal with freedesktop.org desktop files.
 *
 * This conforms with the freedesktop.org XDG Desktop Entry Specification version 0.9.4
 */

/**
 * Get the contents of a .ini style file.
 *
 * The Ecore_Hash returned is a two level hash, the first level
 * is the groups in the file, one per group, keyed by the name 
 * of that group.  The value of each of those first level hashes
 * is the second level Ecore_Hash, the contents of each group.
 *
 * @param   file Full path to the .ini style file.
 * @return  An Ecore_Hash of the files contents.
 * @ingroup Ecore_Desktop_Main_Group
 */
Ecore_Hash         *
ecore_desktop_ini_get(const char *file)
{
   Ecore_Hash         *result;

/* FIXME: should probably look in ini_file_cache first. */
   result = ecore_hash_new(ecore_str_hash, ecore_str_compare);
   if (result)
     {
	FILE               *f;
	char                buffer[PATH_MAX];
	Ecore_Hash         *current = NULL;

	f = fopen(file, "r");
	if (!f)
	  {
	     fprintf(stderr, "ERROR: Cannot Open File %s\n", file);
	     ecore_hash_destroy(result);
	     return NULL;
	  }
	ecore_hash_set_free_key(result, free);
	ecore_hash_set_free_value(result, (Ecore_Free_Cb) ecore_hash_destroy);
	*buffer = '\0';
#ifdef DEBUG
	fprintf(stdout, "PARSING INI %s\n", file);
#endif
	while (fgets(buffer, sizeof(buffer), f) != NULL)
	  {
	     char               *c;
	     char               *key;
	     char               *value;

	     c = buffer;
	     /* Strip preceeding blanks. */
	     while (((*c == ' ') || (*c == '\t')) && (*c != '\n')
		    && (*c != '\0'))
		c++;
	     /* Skip blank lines and comments */
	     if ((*c == '\0') || (*c == '\n') || (*c == '#'))
		continue;
	     if (*c == '[')	/* New group. */
	       {
		  key = c + 1;
		  while ((*c != ']') && (*c != '\n') && (*c != '\0'))
		     c++;
		  *c++ = '\0';
		  current = ecore_hash_new(ecore_str_hash, ecore_str_compare);
		  if (current)
		    {
		       ecore_hash_set_free_key(current, free);
		       ecore_hash_set_free_value(current, free);
		       ecore_hash_set(result, strdup(key), current);
#ifdef DEBUG
		       fprintf(stdout, "  GROUP [%s]\n", key);
#endif
		    }
	       }
	     else if (current)	/* key=value pair of current group. */
	       {
		  key = c;
		  /* Find trailing blanks or =. */
		  while ((*c != '=') && (*c != ' ') && (*c != '\t')
			 && (*c != '\n') && (*c != '\0'))
		     c++;
		  if (*c != '=')	/* Find equals. */
		    {
		       *c++ = '\0';
		       while ((*c != '=') && (*c != '\n') && (*c != '\0'))
			  c++;
		    }
		  if (*c == '=')	/* Equals found. */
		    {
		       *c++ = '\0';
		       /* Strip preceeding blanks. */
		       while (((*c == ' ') || (*c == '\t')) && (*c != '\n')
			      && (*c != '\0'))
			  c++;
		       value = c;
		       /* Find end. */
		       while ((*c != '\n') && (*c != '\0'))
			  c++;
		       *c++ = '\0';
		       /* FIXME: should strip space at end, then unescape value. */
		       ecore_hash_set(current, strdup(key), strdup(value));
#ifdef DEBUG
		       fprintf(stdout, "    %s=%s\n", key, value);
#endif
		    }
	       }

	  }
	buffer[0] = (char)0;

	fclose(f);
	ecore_hash_set(ini_file_cache, strdup(file), result);
     }
   return result;
}

/**
 * Get the contents of a .desktop file.
 *
 * Everything that is in the .desktop file is returned in the
 * data member of the Ecore_Desktop structure, it's an Ecore_Hash 
 * as returned by ecore_desktop_ini_get().  Some of the data in the
 * .desktop file is decoded into specific members of the returned 
 * structure.
 *
 * Use ecore_desktop_destroy() to free this structure.
 *
 * @param   file Full path to the .desktop file.
 * @param   lang Language to use, or NULL for default.
 * @return  An Ecore_Desktop containing the files contents.
 * @ingroup Ecore_Desktop_Main_Group
 */
Ecore_Desktop      *
ecore_desktop_get(const char *file, const char *lang)
{
   Ecore_Desktop      *result;

   result = (Ecore_Desktop *) ecore_hash_get(desktop_cache, (char *) file);
   if (!result)
     {
	result = calloc(1, sizeof(Ecore_Desktop));
	if (result)
	  {
	     result->original_path = strdup(file);
	     if (lang)
	        result->original_lang = strdup(lang);
	     result->data = ecore_desktop_ini_get(result->original_path);
	     if (result->data)
	       {
		  result->group =
		     (Ecore_Hash *) ecore_hash_get(result->data,
						   "Desktop Entry");
		  if (!result->group)
		     result->group =
			(Ecore_Hash *) ecore_hash_get(result->data,
						      "KDE Desktop Entry");
		  if (result->group)
		    {
		       char               *value;
		       char               *eap_name = NULL;
		       char               *exe = NULL;
		       char               *categories = NULL;
		       int                 size = 0;

                       value = (char *) ecore_file_get_file(result->original_path);
		       if (value)
		          {
			     char *temp = strrchr(value, '.');
			     if (temp)
			        *temp = '\0';
			     result->eap_name = malloc(strlen(value) + 5);
			     if (result->eap_name)
			        sprintf(result->eap_name, "%s.eap", value);
			     if (temp)
			        *temp = '.';
			  }
		       eap_name = result->eap_name;

		       result->name =
			  (char *)ecore_hash_get(result->group, "Name");
		       result->generic =
			  (char *)ecore_hash_get(result->group, "GenericName");
		       result->comment =
			  (char *)ecore_hash_get(result->group, "Comment");
		       result->type =
			  (char *)ecore_hash_get(result->group, "Type");

		       result->path =
		          (char *)ecore_hash_get(result->group, "Path");
		       result->URL =
		          (char *)ecore_hash_get(result->group, "URL");
		       result->file =
		          (char *)ecore_hash_get(result->group, "File");

		       result->exec =
			  (char *)ecore_hash_get(result->group, "Exec");
// FIXME: Handle the fdo %x replacable params.  Some should be stripped, some should be expanded.
                       exe = result->exec;
		       value =
			  (char *)ecore_hash_get(result->group,
						 "StartupWMClass");
		       if (value)
		          result->window_class = strdup(value);
		       else if (result->exec)
		          {
	                     char *tmp;

	                     /* Guess - exe name with first letter capitalized. */
                             tmp = strdup(result->exec);
	                     if (tmp)
	                       {
	                          char *p;

	                          value = (char *) ecore_file_get_file(tmp);  /* In case the exe included a path. */
				  p = value;
	                          while ((*p != '\0') && (*p != ' '))
	                            {
	                               *p = tolower(*p);
		                       p++;
	                            }
	                          *p = '\0';
	                          *value = toupper(*value);
                                  result->window_class = strdup(value);
	                          free(tmp);
	                       }
			  }


		       result->icon =
			  (char *)ecore_hash_get(result->group, "Icon");
		       result->icon_class =
			  (char *)ecore_hash_get(result->group, "X-Enlightenment-IconClass");
		       value =
			  (char *)ecore_hash_get(result->group, "X-Enlightenment-IconPath");
		       if (value)
		          result->icon_path = strdup(value);

                       if ((result->icon != NULL) && (result->icon_path == NULL) && (strchr(result->icon, '/') != NULL))
		          {
			     if(result->icon[0] == '/')
			        {
				   result->icon_path = strdup(result->icon);
				   result->icon = NULL;
				}
			     else   /* It's a relative path. */
			        {
				   char *temp;

				   size = strlen(result->original_path) + strlen(result->icon) + 2;
				   temp = malloc(size);
				   if (temp)
				      {
				         char *dir;

                                         dir = ecore_file_get_dir(result->original_path);
					 if (dir)
					    {
				               sprintf(temp, "%s/%s", dir, result->icon);
                                               result->icon_path = ecore_file_realpath(temp);
				               result->icon = NULL;
					       free(dir);
					    }
					 free(temp);
				      }
				}
			  }

		       result->categories =
			  (char *)ecore_hash_get(result->group, "Categories");
		       if (result->categories)
			  result->Categories =
			     ecore_desktop_paths_to_hash(result->categories);
		       categories = result->categories;
		       value =
			  (char *)ecore_hash_get(result->group, "OnlyShowIn");
		       if (value)
			  result->OnlyShowIn =
			     ecore_desktop_paths_to_hash(value);
		       value =
			  (char *)ecore_hash_get(result->group, "NotShowIn");
		       if (value)
			  result->NotShowIn =
			     ecore_desktop_paths_to_hash(value);
		       value =
			  (char *)ecore_hash_get(result->group,
						 "X-KDE-StartupNotify");
		       if (value)
			  result->startup =
			     (!strcmp(value, "true")) ? "1" : "0";
		       value =
			  (char *)ecore_hash_get(result->group,
						 "StartupNotify");
		       if (value)
			  result->startup =
			     (!strcmp(value, "true")) ? "1" : "0";

/*
 *    icon/class is a list of standard icons from the theme that can override the icon created above.
 *    Use (from .desktop) eap name,exe name,categories.  It's case sensitive, the reccomendation is to lowercase it.
 *    It should be most specific to most generic.  firefox,browser,internet for instance
*/

		      /* If the icon in the file is not a full path, just put it first in the class, greatly simplifies things. 
		       * Otherwise, put that full path into the icon_path member.
		       */
		      size = 0;
		      if ((result->icon) && (result->icon[0] != '/'))
                         size += strlen(result->icon) + 1;
                      if (eap_name)  size += strlen(eap_name) + 1;
                      if (exe)  size += strlen(exe) + 1;
                      if (categories)  size += strlen(categories) + 1;
		      result->icon_class = malloc(size + 1);
		      if (result->icon_class)
		         {
	                    char *p;
			    int done = 0;

		            result->icon_class[0] = '\0';
			    if ((result->icon) && (result->icon[0] != '/') && (result->icon[0] != '\0'))
			       {
			          strcat(result->icon_class, result->icon);
				  done = 1;
				  result->icon = NULL;
			       }
			    /* We do this here coz we don't want to lower case the result->icon part later. */
			    p = result->icon_class;
			    p += strlen(result->icon_class);
		            if ((eap_name) && (eap_name[0] != '\0'))
		               {
			          if (done)
			             strcat(result->icon_class, ",");
			          strcat(result->icon_class, eap_name);
				  done = 1;
			       }
		            if ((exe) && (exe[0] != '\0'))
		               {
	                           char *tmp;

                                   tmp = strdup(ecore_file_get_file(exe));
	                           if (tmp)
	                             {
				        p = tmp;
	                                while (*p != '\0')
	                                   {
			                      if (*p == ' ')
					      {
				                 *p = '\0';
						 break;
					      }
		                              p++;
	                                   }
			                if (done)
			                   strcat(result->icon_class, ",");
			                strcat(result->icon_class, tmp);
				        done = 1;
					free(tmp);
				     }
			       }
		            if ((categories) && (categories[0] != '\0'))
		               {
			          if (done)
			             strcat(result->icon_class, ",");
			          strcat(result->icon_class, categories);
			          done = 1;
			       }
	                    while (*p != '\0')
	                       {
			          if (*p == ';')
				     *p = ',';
				  else
	                             *p = tolower(*p);
		                  p++;
	                       }
			 }
		    }
		  else
		    {
		       /*Maybe it's a 'trash' file - which also follows the Desktop FDO spec */
		       result->group =
			  (Ecore_Hash *) ecore_hash_get(result->data,
							"Trash Info");
		       if (result->group)
			 {
			    result->path =
			       (char *)ecore_hash_get(result->group, "Path");
			    result->deletiondate =
			       (char *)ecore_hash_get(result->group,
						      "DeletionDate");
			 }
		    }

		  ecore_hash_set(desktop_cache, strdup(result->original_path), result);
	       }
	     else
	       {
		  free(result);
		  result = NULL;
	       }
	  }
     }
   return result;
}

void
ecore_desktop_save(Ecore_Desktop * desktop)
{
   if (!desktop->group)
      {
         desktop->group = ecore_hash_new(ecore_str_hash, ecore_str_compare);
         if (desktop->group)
	    {
               ecore_hash_set_free_key(desktop->group, free);
	       ecore_hash_set_free_value(desktop->group, free);
	    }
      }

   if (desktop->group)
      {
         if (desktop->original_path)
	    {
               struct stat st;

               if (stat(desktop->original_path, &st) >= 0)
	          {
	             char *real;

                     real = ecore_file_readlink(desktop->original_path);
	             if (real)
	                ecore_hash_set(desktop->group, strdup("X-Enlightenment-OriginalPath"), real);
		  }
	    }

         if (desktop->name)   ecore_hash_set(desktop->group, strdup("Name"), strdup(desktop->name));
         if (desktop->generic)   ecore_hash_set(desktop->group, strdup("GenericName"), strdup(desktop->generic));
         if (desktop->comment)   ecore_hash_set(desktop->group, strdup("Comment"), strdup(desktop->comment));
         if (desktop->type)   ecore_hash_set(desktop->group, strdup("Type"), strdup(desktop->type));
         if (desktop->URL)   ecore_hash_set(desktop->group, strdup("URL"), strdup(desktop->URL));
         if (desktop->file)   ecore_hash_set(desktop->group, strdup("File"), strdup(desktop->file));
         if (desktop->exec)   ecore_hash_set(desktop->group, strdup("Exec"), strdup(desktop->exec));
         if (desktop->icon)   ecore_hash_set(desktop->group, strdup("Icon"), strdup(desktop->icon));
         if (desktop->icon_class)   ecore_hash_set(desktop->group, strdup("X-Enlightenment-IconClass"), strdup(desktop->icon_class));
         if (desktop->icon_path)   ecore_hash_set(desktop->group, strdup("X-Enlightenment-IconPath"), strdup(desktop->icon_path));
         if (desktop->window_class)   ecore_hash_set(desktop->group, strdup("StartupWMClass"), strdup(desktop->window_class));
         if (desktop->categories)   ecore_hash_set(desktop->group, strdup("Categories"), strdup(desktop->categories));
	 ecore_hash_remove(desktop->group, "X-KDE-StartupNotify");
         if (desktop->startup)
	    {
               if (desktop->startup[0] == '1')
	          ecore_hash_set(desktop->group, strdup("StartupNotify"), strdup("true"));
	       else
	          ecore_hash_set(desktop->group, strdup("StartupNotify"), strdup("false"));
	    }

         /* FIXME: deal with the ShowIn's. */

         if (desktop->path)   ecore_hash_set(desktop->group, strdup("Path"), strdup(desktop->path));
         if (desktop->deletiondate)   ecore_hash_set(desktop->group, strdup("DeletionDate"), strdup(desktop->deletiondate));

	 if (desktop->original_path)
	    {
               FILE *f;
               Ecore_List *list;
               char *key;

               ecore_file_unlink(desktop->original_path);
               f = fopen(desktop->original_path, "wb");
               list = ecore_hash_keys(desktop->group);
               if ((!f) || (!list)) return;

               fprintf(f, "[Desktop Entry]\n");
	       ecore_list_goto_first(list);
               while ((key = (char *) ecore_list_next(list)))
	          fprintf(f, "%s=%s\n", key, (char *) ecore_hash_get(desktop->group, key));
               fclose(f);
	    }
      }
}



/**
 * Setup what ever needs to be setup to support Ecore_Desktop.
 *
 * There are internal structures that are needed for Ecore_Desktop
 * functions to operate, this sets them up.
 *
 * @ingroup Ecore_Desktop_Main_Group
 */
EAPI int
ecore_desktop_init()
{
   if (++init_count != 1) return init_count;

   if (!ecore_desktop_paths_init()) return --init_count;

   if (!ini_file_cache)
     {
	ini_file_cache = ecore_hash_new(ecore_str_hash, ecore_str_compare);
	if (ini_file_cache)
	  {
	     ecore_hash_set_free_key(ini_file_cache, free);
	     ecore_hash_set_free_value(ini_file_cache,
				       (Ecore_Free_Cb) ecore_hash_destroy);
	  }
     }
   if (!desktop_cache)
     {
	desktop_cache = ecore_hash_new(ecore_str_hash, ecore_str_compare);
	if (desktop_cache)
	  {
	     ecore_hash_set_free_key(desktop_cache, free);
	     ecore_hash_set_free_value(desktop_cache,
				       (Ecore_Free_Cb) _ecore_desktop_destroy);
	  }
     }

   if (!ecore_desktop_icon_init()) return --init_count;

   return init_count;   
}

/**
 * Tear down what ever needs to be torn down to support Ecore_Desktop.
 *
 * There are internal structures that are needed for Ecore_Desktop
 * functions to operate, this tears them down.
 *
 * @ingroup Ecore_Desktop_Main_Group
 */
EAPI int
ecore_desktop_shutdown()
{
   if (--init_count != 0) return init_count;

   ecore_desktop_icon_shutdown();

   if (ini_file_cache)
     {
	ecore_hash_destroy(ini_file_cache);
	ini_file_cache = NULL;
     }
   if (desktop_cache)
     {
	ecore_hash_destroy(desktop_cache);
	desktop_cache = NULL;
     }

   ecore_desktop_paths_shutdown();

   return init_count;   
}

/**
 * Free whatever resources are used by an Ecore_Desktop.
 *
 * There are internal resources used by each Ecore_Desktop
 * This releases those resources.
 *
 * @param  desktop  An Ecore_Desktop that was previously returned by ecore_desktop_get().
 * @ingroup Ecore_Desktop_Main_Group
 */
void
ecore_desktop_destroy(Ecore_Desktop * desktop)
{
  /* This is just a dummy, because these structures are cached. */
  /* Later versions of the cache may reference count, then this will be useful. */
}

void
_ecore_desktop_destroy(Ecore_Desktop * desktop)
{
   if (desktop->original_path)
      free(desktop->original_path);
   if (desktop->icon_path)
      free(desktop->icon_path);
   if (desktop->original_lang)
      free(desktop->original_lang);
   if (desktop->eap_name)
      free(desktop->eap_name);
   if (desktop->icon_class)
      free(desktop->icon_class);
   if (desktop->window_class)
      free(desktop->window_class);
   if (desktop->NotShowIn)
      ecore_hash_destroy(desktop->NotShowIn);
   if (desktop->OnlyShowIn)
      ecore_hash_destroy(desktop->OnlyShowIn);
   if (desktop->Categories)
      ecore_hash_destroy(desktop->Categories);
   free(desktop);
}

/**
 * Get and massage the users home directory.
 *
 * This is an internal function that may be useful elsewhere.
 *
 * @return  The users howe directory.
 * @ingroup Ecore_Desktop_Main_Group
 */
char *
ecore_desktop_home_get()
{
   char   home[PATH_MAX];
   int    len;

   /* Get Home Dir, check for trailing '/', strip it */
   strncpy(home, getenv("HOME"), PATH_MAX);
   len = strlen(home) - 1;
   while ((len >= 0) && (home[len] == '/'))
     {
	home[len] = '\0';
	len--;
     }

   return strdup(home);
}

