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

   snprintf(file, PATH_MAX, "%s/.e/apps/%s/config.eet", getenv("HOME"),
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

   snprintf(file, PATH_MAX, "%s/.e/apps/%s/config.eet", getenv("HOME"),
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
   Ecore_Config_DB_File  *db;
   char                 **keys;
   int                    key_count;
   int                    x, pt;
   int                    itmp;
   double                 ftmp;
   char                   *type;
   char                   *data;

   db = NULL;
   type = NULL;
   data = NULL;

   db = _ecore_config_db_open_read(file);
   if (!db)
     {
	E(0, "Cannot open database from file %s!\n", file);
	return ECORE_CONFIG_ERR_NODATA;
     }
   key_count = 0;   
   keys = _ecore_config_db_keys_get(db, &key_count);
   if (keys)
     {
	for (x = 0; x < key_count; x++)
	  {
	     type = _ecore_config_db_key_type_get(db, keys[x]);
	     if (!type) type = "?";
	     if (!strcmp(type, "int"))
	       {
		  if (_ecore_config_db_key_int_get(db, keys[x], &itmp))
		    {
		       Ecore_Config_Prop  *p;
		       
		       pt = PT_INT;
		       if ((p = ecore_config_get(keys[x]))) pt = p->type;
		       switch (pt)
			 {
			  case PT_BLN:
			    ecore_config_boolean_set(keys[x], itmp);
			    break;
			  default:
			    ecore_config_int_set(keys[x], itmp);
			    break;
			 }
		    }
		  else
		    {
		       E(0, "Could not read key %s!\n", keys[x]);
		    }
	       }
	     else if (!strcmp(type, "float"))
	       {
		  if (_ecore_config_db_key_float_get(db, keys[x], &ftmp))
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
		  data = _ecore_config_db_key_str_get(db, keys[x]);
		  if (data)
		    {
		       pt = ecore_config_type_guess(keys[x], data);
		       switch (pt)
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
	     if (type) free(type);
	  }
     }
   _ecore_config_db_close(db);
   if (keys)
     {
	for (x = 0; x < key_count; x++)
	  {
	     free(keys[x]);
	  }
	free(keys);
     }
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
   Ecore_Config_Prop    *next;
   Ecore_Config_DB_File *db;
   struct stat           status;
   char                 *tmp;

   next = __ecore_config_bundle_local->data;
   db = NULL;

   /* if file does not exist check to see if the dirs exist, creating if not */
   if (stat(file, &status))
      _ecore_config_recurse_mkdir(file);

   db = _ecore_config_db_open_write(file);
   if (!db)
     {
	E(0, "Cannot open database from file %s!\n", file);
	return ECORE_CONFIG_ERR_FAIL;
     }

   while (next)
     {
	/* let the config_db deal with this
	 * handyande: hmm, not sure that it ever does - reinstating until
	 * further discussions satisfy me!
	 */
	if (!(next->flags & PF_MODIFIED) || next->flags & PF_CMDLN)
	  {
	     next = next->next;
	     continue;
	  }

	tmp = NULL;

	switch (next->type)
	  {
	  case PT_INT:
	     _ecore_config_db_key_int_set(db, next->key, ecore_config_int_get(next->key));
	     break;
	  case PT_BLN:
	     _ecore_config_db_key_int_set(db, next->key, ecore_config_boolean_get(next->key));
	     break;
	  case PT_FLT:
	     _ecore_config_db_key_float_set(db, next->key, ecore_config_float_get(next->key));
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
	   _ecore_config_db_key_str_set(db, next->key, tmp);
	   free(tmp);
	}

	next = next->next;
     }

   _ecore_config_db_close(db);
   return ECORE_CONFIG_ERR_SUCC;
}
