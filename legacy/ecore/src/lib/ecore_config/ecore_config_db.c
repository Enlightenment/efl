#include "Ecore_Config.h"
#include "ecore_config_private.h"
#include <Eet.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct _Ecore_Config_DB_File
{
   Eet_File *ef;
};

Ecore_Config_DB_File *
_ecore_config_db_open_read(char *file)
{
   Eet_File *ef;
   Ecore_Config_DB_File *db;
   
   eet_init();
   db = malloc(sizeof(Ecore_Config_DB_File));
   if (!db) return NULL;
   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
	free(db);
	return NULL;
     }
   db->ef = ef;
   return db;
}

Ecore_Config_DB_File *
_ecore_config_db_open_write(char *file)
{
   Eet_File *ef;
   Ecore_Config_DB_File *db;
   
   eet_init();
   db = malloc(sizeof(Ecore_Config_DB_File));
   if (!db) return NULL;
   ef = eet_open(file, EET_FILE_MODE_WRITE);
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
   
   keys = eet_list(db->ef, "*", &key_count);
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

char *
_ecore_config_db_key_type_get(Ecore_Config_DB_File *db, char *key)
{
   char *data;
   int size;
   
   data = eet_read(db->ef, key, &size);
   if (data)
     {
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
	return data;
     }
   return NULL;
}

int
_ecore_config_db_key_int_get(Ecore_Config_DB_File *db, char *key, int *dest)
{
   char *data;
   int size;
   
   data = eet_read(db->ef, key, &size);
   if (data)
     {
	int l;

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
	*dest = atoi(data + l + 1);
	free(data);
     }
   return 0;
}

int
_ecore_config_db_key_float_get(Ecore_Config_DB_File *db, char *key, double *dest)
{
   char *data;
   int size;
   
   data = eet_read(db->ef, key, &size);
   if (data)
     {
	int l;
	
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
	/* "type" NIL "1242.4234" NIL */
	l = strlen(data);
	if (l >= (size - 1))
	  {
	     free(data);
	     return 0;
	  }
	*dest = atof(data + l + 1);
	free(data);
     }
   return 0;
}

char *
_ecore_config_db_key_str_get(Ecore_Config_DB_File *db, char *key)
{
   char *data;
   int size;
   
   data = eet_read(db->ef, key, &size);
   if (data)
     {
	int l;
	char *s;
	
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
	/* "type" NIL "string goes here" NIL */
	l = strlen(data);
	if (l >= (size - 1))
	  {
	     free(data);
	     return NULL;
	  }
	s  = strdup(data + l + 1);
	free(data);
	return s;
     }
   return NULL;
}

void *
_ecore_config_db_key_data_get(Ecore_Config_DB_File *db, char *key, int *size_ret)
{
   char *data;
   int size;
   
   data = eet_read(db->ef, key, &size);
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
	/* "type" NIL data_goes_here NIL */
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
}

void
_ecore_config_db_key_int_set(Ecore_Config_DB_File *db, char *key, int val)
{
   char buf[256];
   int num;
   
   num = snprintf(buf, sizeof(buf), "%s %i ", "int", val);
   buf[3] = 0;
   buf[num - 1] = 0;
   eet_write(db->ef, key, buf, num, 1);
}

void
_ecore_config_db_key_float_set(Ecore_Config_DB_File *db, char *key, double val)
{
   char buf[256];
   int num;
   
   num = snprintf(buf, sizeof(buf), "%s %f ", "float", val);
   buf[5] = 0;
   buf[num - 1] = 0;
   eet_write(db->ef, key, buf, num, 1);
}

void
_ecore_config_db_key_str_set(Ecore_Config_DB_File *db, char *key, char *str)
{
   char *buf;
   int num;
   
   num = 3 + 1 + strlen(str) + 1;
   buf = malloc(num);
   if (!buf) return;
   strcpy(buf, "str");
   strcpy(buf + 4, str);
   eet_write(db->ef, key, buf, num, 1);
   free(buf);
}

void
_ecore_config_db_key_data_set(Ecore_Config_DB_File *db, char *key, void *data, int data_size)
{
   char *buf;
   int num;
   
   num = 3 + 1 + data_size + 1;
   buf = malloc(num);
   if (!buf) return;
   strcpy(buf, "data");
   memcpy(buf + 5, data, data_size);
   buf[num - 1] = 0;
   eet_write(db->ef, key, buf, num, 1);
   free(buf);
}
