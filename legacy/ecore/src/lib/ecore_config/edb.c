#include <Edb.h>
#include "Ecore_Config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int ecore_config_load(Ecore_Config_Bundle *b) {
  char* file = malloc(1024); /* ### fixme */
  sprintf(file,"%s/.e/apps/%s/config.db",getenv("HOME"),b->owner);
  return ecore_config_load_file(b,file);
  free(file);
  }

int ecore_config_save(Ecore_Config_Bundle *b) {
  char* file = malloc(1024); /* ### fixme */
  sprintf(file,"%s/.e/apps/%s/config.db",getenv("HOME"),b->owner);
  return ecore_config_save_file(b,file);
  free(file);
  }

int ecore_config_load_file(Ecore_Config_Bundle *b, char *file) {
  E_DB_File *db = NULL;
  char **keys;
  int key_count;
  int x;
  int itmp;
  float ftmp;
  char *type;
  char *data;
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
        ecore_config_set_int(b, keys[x], itmp);
      } else {
        E(0, "Could not read key %s!\n", keys[x]);
      }
    } else if (!strcmp(type, "float")) {
      if (e_db_float_get(db, keys[x], &ftmp)) {
        ecore_config_set_float(b, keys[x], ftmp);
      } else {
        E(0, "Could not read key %s!\n", keys[x]);
      }
    } else if (!strcmp(type, "str")) {
      if (data = e_db_str_get(db, keys[x])) {
        if (ecore_config_guess_type(data)==PT_RGB)
          ecore_config_set_rgb(b,keys[x],data);
        else
          ecore_config_set_string(b,keys[x],data);
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
  return 0;
}

int ecore_config_save_file(Ecore_Config_Bundle *b, char *file) {
  Ecore_Config_Prop *next=b->data;
  E_DB_File   *db = NULL;

  /* ### we may need to create a directory or two here! */
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
        e_db_int_set(db, next->key, ecore_config_get_int(b, next->key));
        break;
      case PT_FLT:
        e_db_float_set(db, next->key, ecore_config_get_float(b, next->key));
        break;
      case PT_RGB:
        e_db_str_set(db, next->key, ecore_config_get_rgbstr(b, next->key));
        break;
      case PT_STR:
        e_db_str_set(db, next->key, ecore_config_get_string(b, next->key));
        break;
      case PT_NIL:
        /* currently we do nothing for undefined ojects */
    }

    next=next->next;
  }
  
  e_db_close(db);
  return 1;
}

