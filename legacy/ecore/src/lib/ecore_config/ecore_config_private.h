#ifndef _ECORE_CONFIG_PRIVATE_H
# define _ECORE_CONFIG_PRIVATE_H

/* debug */
extern int               DEBUG;
#ifdef __sgi
# define D
# define E
#else
# define D(fmt,args...) do { if(DEBUG>=0) fprintf(stderr,fmt,## args); } while(0);
# define E(lvl,args...) do { if(DEBUG>=(lvl)) fprintf(stderr,## args); } while(0)
#endif

typedef struct _Ecore_Config_DB_File Ecore_Config_DB_File;

int                   _ecore_config_mod_init(const char *pipe_name, void **data);
int                   _ecore_config_mod_exit(void **data);
int                   _ecore_config_mod_poll(void **data);

Ecore_Config_DB_File *_ecore_config_db_open_read(const char *file);
Ecore_Config_DB_File *_ecore_config_db_open_write(const char *file);
void                  _ecore_config_db_close(Ecore_Config_DB_File *db);
char                **_ecore_config_db_keys_get(Ecore_Config_DB_File *db, int *num_ret);
Ecore_Config_Type     _ecore_config_db_key_type_get(Ecore_Config_DB_File *db, const char *key);
int                   _ecore_config_db_read(Ecore_Config_DB_File *db, const char *key);
void                  _ecore_config_db_write(Ecore_Config_DB_File *db, Ecore_Config_Prop *e);

int                   _ecore_config_boolean_get(Ecore_Config_Prop *e);
char                 *_ecore_config_string_get(Ecore_Config_Prop *e);
long                  _ecore_config_int_get(Ecore_Config_Prop *e);
int                   _ecore_config_argb_get(Ecore_Config_Prop *e, int *a, int *r,
					  int *g, int *b);
char                 *_ecore_config_argbstr_get(Ecore_Config_Prop *e);
long                  _ecore_config_argbint_get(Ecore_Config_Prop *e);
float                 _ecore_config_float_get(Ecore_Config_Prop *e);
char                 *_ecore_config_theme_get(Ecore_Config_Prop *e);

int                   _ecore_config_ipc_ecore_init(const char *pipe_name, void **data);
int                   _ecore_config_ipc_ecore_exit(void **data);
int                   _ecore_config_ipc_ecore_poll(void **data);

#include "ecore_private.h"

#endif
