#include <Edb.h>
#include "Ecore_Config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int ecore_config_load(void) {
  char file[PATH_MAX];
  snprintf(file, PATH_MAX, "%s/.e/apps/%s/config.db",getenv("HOME"),__ecore_config_app_name);
  return ecore_config_load_file(file);
}

int ecore_config_save(void) {
  char file[PATH_MAX];
  snprintf(file, PATH_MAX, "%s/.e/apps/%s/config.db",getenv("HOME"),__ecore_config_app_name);
  return ecore_config_save_file(file);
}

int ecore_config_load_file(char *file) {
  E_DB_File *db;
  char **keys;
  int key_count;
  int x;
  int itmp;
  float ftmp;
  char *type;
  char *data;
  db=NULL;
  type = NULL;
  data = NULL;
  
  db = e_db_open_read(file);
  if (!db) {
    E(0, "Cannot open database from file %s!\n", file);
    return 1;
  }

  keys = e_db_dump_key_list(file, &key_count);
  for (x = 0; x < key_count; x++) {

    type = e_db_type_get(db, keys[x]);
    if (!type) type = "?";

    if (!strcmp(type, "int")) {
      if (e_db_int_get(db, keys[x], &itmp)) {
        ecore_config_set_int(keys[x], itmp);
      } else {
        E(0, "Could not read key %s!\n", keys[x]);
      }
    } else if (!strcmp(type, "float")) {
      if (e_db_float_get(db, keys[x], &ftmp)) {
        ecore_config_set_float(keys[x], ftmp);
      } else {
        E(0, "Could not read key %s!\n", keys[x]);
      }
    } else if (!strcmp(type, "str")) {
      data = e_db_str_get(db, keys[x]);
      if (data) {
        if (ecore_config_guess_type(data)==PT_RGB)
          ecore_config_set_rgb(keys[x],data);
        else
          ecore_config_set_string(keys[x],data);
        free(data);
      } else {
        E(0, "Could not read key %s!\n", keys[x]);
      }
    } else {
      E(1, "Unexpected type: %s\n", type);
      continue;
    }
    
    if (type) free(type);
  }
  e_db_close(db);
  free(keys);
  return 0;
}
static void 
_ecore_config_recurse_mkdir(char *file) {
  char              *file_ptr;
  char              *file_tmp;
  struct stat        status;

  file_tmp = strdup(file);
  file_ptr = file_tmp + strlen(file_tmp);
  while (*file_ptr != '/' && file_ptr > file_tmp)
    file_ptr--;
  *file_ptr = '\0';

  if(stat(file_tmp, &status)) {
    _ecore_config_recurse_mkdir(file_tmp);
    mkdir(file_tmp, S_IRUSR | S_IWUSR | S_IXUSR);
  }
  free(file_tmp);
}

int ecore_config_save_file(char *file) {
  Ecore_Config_Prop *next;
  E_DB_File         *db;
  struct stat        status;
  next=__ecore_config_bundle_local->data;
  db = NULL;

  /* if file does not exist check to see if the dirs exist, creating if not */
  if(stat(file, &status))
    _ecore_config_recurse_mkdir(file);

  db = e_db_open(file);
  if (!db) {
    E(0, "Cannot open database from file %s!\n", file);
    return 1;
  } 
  
  while (next) {
    if (!(next->flags&PF_MODIFIED)) {
      next=next->next;
      continue;
    }
    switch (next->type) {
      case PT_INT:
        e_db_int_set(db, next->key, ecore_config_get_int(next->key));
        break;
      case PT_FLT:
        e_db_float_set(db, next->key, ecore_config_get_float(next->key));
        break;
      case PT_RGB:
        e_db_str_set(db, next->key, ecore_config_get_rgbstr(next->key));
        break;
      case PT_STR:
        e_db_str_set(db, next->key, ecore_config_get_string(next->key));
        break;
      case PT_NIL:
        /* currently we do nothing for undefined ojects */
        break;
    }

    next=next->next;
  }
  
  e_db_close(db);
  e_db_flush();
  return 1;
}

