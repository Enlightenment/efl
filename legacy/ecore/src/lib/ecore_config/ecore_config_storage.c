/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Ecore_Config.h"
#include "ecore_config_private.h"

/**
 * Loads the default configuration.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.  @c ECORE_CONFIG_ERR_NODATA
 *          is returned if the file cannot be loaded.
 * @ingroup Ecore_Config_File_Group
 */
EAPI int
ecore_config_load(void)
{
   char                file[PATH_MAX];

   if (!__ecore_config_app_name)
     return ECORE_CONFIG_ERR_FAIL;

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
EAPI int
ecore_config_save(void)
{
   char                file[PATH_MAX];

   if (!__ecore_config_app_name)
     return ECORE_CONFIG_ERR_FAIL;

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
EAPI int
ecore_config_file_load(const char *file)
{
   Ecore_Config_DB_File  *db;
   char                 **keys;
   int                    key_count;
   int                    x;
   // double                 ftmp;	UNUSED
   // int                    pt;	UNUSED
   // int                    itmp; 	UNUSED
   // Ecore_Config_Type      type;	UNUSED
   char                   *data;

   db = NULL;
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
	    _ecore_config_db_read(db, keys[x]);
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
_ecore_config_recurse_mkdir(const char *file)
{
   char               *file_ptr;
   char               *file_tmp;
   struct stat         status;

   file_tmp = strdup(file);
   file_ptr = file_tmp + strlen(file_tmp);
   while (*file_ptr != '/' && file_ptr > file_tmp)
      file_ptr--;
   *file_ptr = '\0';

   if ((file_tmp[0] != 0) && stat(file_tmp, &status))
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
EAPI int
ecore_config_file_save(const char *file)
{
   Ecore_Config_Prop    *next;
   Ecore_Config_DB_File *db;
   struct stat           status;

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
	if (!(next->flags & ECORE_CONFIG_FLAG_MODIFIED) || next->flags & ECORE_CONFIG_FLAG_CMDLN)
	  {
	     next = next->next;
	     continue;
	  }

	_ecore_config_db_write(db, next);

	next = next->next;
     }

   _ecore_config_db_close(db);
   return ECORE_CONFIG_ERR_SUCC;
}
