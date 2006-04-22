#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>

#include "Ecore_Config.h"
#include "ecore_config_private.h"
#include "ecore_config_util.h"
#include <Eet.h>

struct _Ecore_Config_DB_File
{
   Eet_File *ef;
};

Ecore_Config_DB_File *
_ecore_config_db_open_read(const char *file)
{
   Eet_File *ef;
   Ecore_Config_DB_File *db;
   
   eet_init();
   db = malloc(sizeof(Ecore_Config_DB_File));
   if (!db) return NULL;
   ef = eet_open((char*)file, EET_FILE_MODE_READ);
   if (!ef)
     {
	free(db);
	return NULL;
     }
   db->ef = ef;
   return db;
}

Ecore_Config_DB_File *
_ecore_config_db_open_write(const char *file)
{
   Eet_File *ef;
   Ecore_Config_DB_File *db;
   
   eet_init();
   db = malloc(sizeof(Ecore_Config_DB_File));
   if (!db) return NULL;
   ef = eet_open((char*)file, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	free(db);
	return NULL;
     }
   db->ef = ef;
   return db;
}

void
_ecore_config_db_close(Ecore_Config_DB_File *db)
{
   eet_close(db->ef);
   free(db);
   eet_shutdown();
}

char **
_ecore_config_db_keys_get(Ecore_Config_DB_File *db, int *num_ret)
{
   char **keys;
   int key_count;
   int i;
   
   keys = eet_list(db->ef, (char*)"*", &key_count);
   if (!keys)
     {
	*num_ret = 0;
	return NULL;
     }
   /* make keys freeable - this is safe to do */
   for (i = 0; i < key_count; i++) keys[i] = strdup(keys[i]);
   *num_ret = key_count;
   return keys;
}

Ecore_Config_Type
_ecore_config_db_key_type_get(Ecore_Config_DB_File *db, const char *key)
{
   char *data;
   int size;
   
   data = eet_read(db->ef, (char*)key, &size);
   if (data)
     {
	if (size <= 2)
	  {
	     free(data);
	     return ECORE_CONFIG_NIL;
	  }
	if (data[size - 1] != 0)
	  {
	     free(data);
	     return ECORE_CONFIG_NIL;
	  }
	return (Ecore_Config_Type) data[0];
     }
   return ECORE_CONFIG_NIL;
}

int
_ecore_config_db_read(Ecore_Config_DB_File *db, const char *key)
{
   char *data, *value;
   int size;
   Ecore_Config_Prop *prop;
   Ecore_Config_Type type;
   
   data = eet_read(db->ef, (char*)key, &size);
   if (data)
     {
	int l;
	char *prev_locale;

	if (size <= 2)
	  {
	     free(data);
	     return 0;
	  }
	if (data[size - 1] != 0)
	  {
	     free(data);
	     return 0;
	  }
	/* "type" NIL 1242 NIL */
	l = strlen(data);
	if (l >= (size - 1))
	  {
	     free(data);
	     return 0;
	  }
   
	type = data[0];
	value = data + l + 1;
	prop = ecore_config_get(key);
	
	switch (type) 
	  {
	     case ECORE_CONFIG_INT:
	     case ECORE_CONFIG_BLN:
	       {
		  int tmp;
		  prev_locale = setlocale(LC_NUMERIC, "C");
		  tmp = atoi(value);
		  if (prev_locale) setlocale(LC_NUMERIC, prev_locale);

		  ecore_config_typed_set(key, (void *)&tmp, type);
		  break;
	       }
	     case ECORE_CONFIG_FLT:
	       {
		  float tmp;
		  prev_locale = setlocale(LC_NUMERIC, "C");
		  tmp = atof(value);
		  if (prev_locale) setlocale(LC_NUMERIC, prev_locale);
	       
		  ecore_config_typed_set(key, (void *)&tmp, type);
		  break;
	       }
	     case ECORE_CONFIG_RGB:
	       ecore_config_argbstr_set(key, value);
	       break;
	     case ECORE_CONFIG_STR:
	     case ECORE_CONFIG_THM:
	       ecore_config_typed_set(key, (void *)value, type);
	       break;
	     case ECORE_CONFIG_SCT:
	       printf("loading struct %s\n", key);
	       break;
	     default:
	       E(0, "Type %d not handled\n", type);
	  }
	free(data);
	return 1;
     }
   return 0;
}

/*
void *
_ecore_config_db_key_data_get(Ecore_Config_DB_File *db, const char *key, int *size_ret)
{
   char *data;
   int size;
   
   data = eet_read(db->ef, (char*)key, &size);
   if (data)
     {
	int l;
	char *dat;
	
	if (size <= 2)
	  {
	     free(data);
	     return NULL;
	  }
	if (data[size - 1] != 0)
	  {
	     free(data);
	     return NULL;
	  }
	* "type" NIL data_goes_here NIL *
	l = strlen(data);
	if (l >= (size - 1))
	  {
	     free(data);
	     return NULL;
	  }
	dat = malloc(size - (l + 2));
	memcpy(dat, data + l + 1, size - (l + 2));
	free(data);
	*size_ret = size - (l + 2);
	return dat;
     }
   return NULL;
}*/

void
_ecore_config_db_write(Ecore_Config_DB_File *db, Ecore_Config_Prop *e)
{
   char *prev_locale= NULL;
   char *val = NULL;
   char *r = NULL;
   int num;
   
   prev_locale = setlocale(LC_NUMERIC, "C");

   switch (e->type) 
     {
	case ECORE_CONFIG_INT:
	   esprintf(&val, "%i", _ecore_config_int_get(e));
	   break;
	case ECORE_CONFIG_BLN:
	   esprintf(&val, "%i", _ecore_config_boolean_get(e));
	   break;
	case ECORE_CONFIG_FLT:
	   esprintf(&val, "%16.16f", _ecore_config_float_get(e));
	   break;
	case ECORE_CONFIG_STR: 
	   val = _ecore_config_string_get(e);
	   break;
	case ECORE_CONFIG_THM:
	   val = _ecore_config_theme_get(e);
	   break;
	case ECORE_CONFIG_RGB:
	   val = _ecore_config_argbstr_get(e);
	   break;
	default:
	   E(0, "Type %d not handled\n", e->type);
     }

   if (prev_locale)
     {
	setlocale(LC_NUMERIC, prev_locale);
     }
   
   if(val)
     {
	num = esprintf(&r, "%c%c%s%c", (char) e->type, 0, val, 0);
	if(num)
	  eet_write(db->ef, e->key, r, num, 1);
	free(r);
     }

   free(val);
}
/*
void
_ecore_config_db_key_data_set(Ecore_Config_DB_File *db, const char *key, void *data, int data_size)
{
   char *buf;
   int num;
   
   num = 1 + 1 + data_size + 1;
   buf = malloc(num);
   if (!buf) return;
   buf[0] = (char) ECORE_CONFIG_BIN;
   buf[1] = 0;
   memcpy(buf + 2, data, data_size);
   buf[num - 1] = 0;
   eet_write(db->ef, (char*)key, buf, num, 1);
   free(buf);
}*/
