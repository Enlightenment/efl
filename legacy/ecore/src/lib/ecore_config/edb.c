#include <Edb.h>
#include "Ecore_Config.h"

#include "ecore_config_private.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Loads the default configuration.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.  @c ECORE_CONFIG_ERR_NODATA
 *          is returned if the file cannot be loaded.
 * @ingroup Ecore_Config_File_Group
 */
int
ecore_config_load(void)
{
   char                file[PATH_MAX];

   snprintf(file, PATH_MAX, "%s/.e/apps/%s/config.db", getenv("HOME"),
	    __ecore_config_app_name);
   return ecore_config_file_load(file);
}

/**
 * Saves the current configuration to the default file.
 * @return  @c ECORE_CONFIG_ERR_SUCC is returned on success.
 *          @c ECORE_CONFIG_ERR_FAIL is returned if the data cannot be
 *          saved.
 * @ingroup Ecore_Config_File_Group
 */
int
ecore_config_save(void)
{
   char                file[PATH_MAX];

   snprintf(file, PATH_MAX, "%s/.e/apps/%s/config.db", getenv("HOME"),
	    __ecore_config_app_name);
   return ecore_config_file_save(file);
}

/**
 * Load the given configuration file to the local configuration.
 * @param   file Name of the file to load.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.  @c ECORE_CONFIG_ERR_NODATA
 *          is returned if the file cannot be loaded.
 * @ingroup Ecore_Config_File_Group
 */
int
ecore_config_file_load(char *file)
{
   E_DB_File          *db;
   char              **keys;
   int                 key_count;
   int                 x;
   int                 itmp;
   float               ftmp;
   char               *type;
   char               *data;

   db = NULL;
   type = NULL;
   data = NULL;

   db = e_db_open_read(file);
   if (!db)
     {
	E(0, "Cannot open database from file %s!\n", file);
	return ECORE_CONFIG_ERR_NODATA;
     }

   keys = e_db_dump_key_list(file, &key_count);
   for (x = 0; x < key_count; x++)
     {

	type = e_db_type_get(db, keys[x]);
	if (!type)
	   type = "?";

	if (!strcmp(type, "int"))
	  {
	     if (e_db_int_get(db, keys[x], &itmp))
	       {
		  ecore_config_int_set(keys[x], itmp);
	       }
	     else
	       {
		  E(0, "Could not read key %s!\n", keys[x]);
	       }
	  }
	else if (!strcmp(type, "float"))
	  {
	     if (e_db_float_get(db, keys[x], &ftmp))
	       {
		  ecore_config_float_set(keys[x], ftmp);
	       }
	     else
	       {
		  E(0, "Could not read key %s!\n", keys[x]);
	       }
	  }
	else if (!strcmp(type, "str"))
	  {
	     data = e_db_str_get(db, keys[x]);
	     if (data)
	       {
		  itmp = ecore_config_type_guess(keys[x], data);
		  switch (itmp)
		    {
		    case PT_RGB:
		       ecore_config_argb_set(keys[x], data);
		       break;
		    case PT_THM:
		       ecore_config_theme_set(keys[x], data);
		       break;
		    default:
		       ecore_config_string_set(keys[x], data);
		    }
		  free(data);
	       }
	     else
	       {
		  E(0, "Could not read key %s!\n", keys[x]);
	       }
	  }
	else
	  {
	     E(1, "Unexpected type: %s\n", type);
	     continue;
	  }

	if (type)
	   free(type);
     }
   e_db_close(db);
   free(keys);
   return ECORE_CONFIG_ERR_SUCC;
}

static void
_ecore_config_recurse_mkdir(char *file)
{
   char               *file_ptr;
   char               *file_tmp;
   struct stat         status;

   file_tmp = strdup(file);
   file_ptr = file_tmp + strlen(file_tmp);
   while (*file_ptr != '/' && file_ptr > file_tmp)
      file_ptr--;
   *file_ptr = '\0';

   if (stat(file_tmp, &status))
     {
	_ecore_config_recurse_mkdir(file_tmp);
	mkdir(file_tmp, S_IRUSR | S_IWUSR | S_IXUSR);
     }
   free(file_tmp);
}

/**
 * Saves the local configuration to the given file.
 * @param   file Name of the file to save to.
 * @return  @c ECORE_CONFIG_ERR_SUCC is returned on success.
 *          @c ECORE_CONFIG_ERR_FAIL is returned if the data cannot be
 *          saved.
 * @ingroup Ecore_Config_File_Group
 */
int
ecore_config_file_save(char *file)
{
   Ecore_Config_Prop  *next;
   E_DB_File          *db;
   struct stat         status;
   char               *tmp;

   next = __ecore_config_bundle_local->data;
   db = NULL;

   /* if file does not exist check to see if the dirs exist, creating if not */
   if (stat(file, &status))
      _ecore_config_recurse_mkdir(file);

   db = e_db_open(file);
   if (!db)
     {
	E(0, "Cannot open database from file %s!\n", file);
	return ECORE_CONFIG_ERR_FAIL;
     }

   while (next)
     {
	if (!(next->flags & PF_MODIFIED))
	  {
	     next = next->next;
	     continue;
	  }

	tmp = NULL;

	switch (next->type)
	  {
	  case PT_INT:
    case PT_BLN:
	     e_db_int_set(db, next->key, ecore_config_int_get(next->key));
	     break;
	  case PT_FLT:
	     e_db_float_set(db, next->key, ecore_config_float_get(next->key));
	     break;
	  case PT_RGB:
	     tmp = ecore_config_argbstr_get(next->key);
	     break;
	  case PT_STR:
	     tmp = ecore_config_string_get(next->key);
	     break;
	  case PT_THM:
	     tmp = ecore_config_theme_get(next->key);
	     break;
	  case PT_NIL:
	     /* currently we do nothing for undefined ojects */
	     break;
	  }

	if (tmp) {
	   e_db_str_set(db, next->key, tmp);
	   free(tmp);
	}

	next = next->next;
     }

   e_db_close(db);
   e_db_flush();
   return ECORE_CONFIG_ERR_SUCC;
}
