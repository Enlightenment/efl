#ifndef _ECORE_CONFIG_PRIVATE_H
# define _ECORE_CONFIG_PRIVATE_H

/* debug */
extern int               DEBUG;
# define D(fmt,args...) do { if(DEBUG>=0) fprintf(stderr,fmt,## args); } while(0);
# define E(lvl,args...) do { if(DEBUG>=(lvl)) fprintf(stderr,## args); } while(0)

typedef struct _Ecore_Config_DB_File Ecore_Config_DB_File;

int                   _ecore_config_mod_init(const char *pipe_name, void **data);
int                   _ecore_config_mod_exit(void **data);
int                   _ecore_config_mod_poll(void **data);

Ecore_Config_DB_File *_ecore_config_db_open_read(const char *file);
Ecore_Config_DB_File *_ecore_config_db_open_write(const char *file);
void                  _ecore_config_db_close(Ecore_Config_DB_File *db);
char                **_ecore_config_db_keys_get(Ecore_Config_DB_File *db, int *num_ret);
char                 *_ecore_config_db_key_type_get(Ecore_Config_DB_File *db, const char *key);
int                   _ecore_config_db_key_int_get(Ecore_Config_DB_File *db, const char *key, int *dest);
int                   _ecore_config_db_key_float_get(Ecore_Config_DB_File *db, const char *key, double *dest);
char                 *_ecore_config_db_key_str_get(Ecore_Config_DB_File *db, const char *key);
void                 *_ecore_config_db_key_data_get(Ecore_Config_DB_File *db, const char *key, int *size_ret);
void                  _ecore_config_db_key_int_set(Ecore_Config_DB_File *db, const char *key, int val);
void                  _ecore_config_db_key_float_set(Ecore_Config_DB_File *db, const char *key, double val);
void                  _ecore_config_db_key_str_set(Ecore_Config_DB_File *db, const char *key, char *str);
void                  _ecore_config_db_key_data_set(Ecore_Config_DB_File *db, const char *key, void *data, int data_size);

#endif
