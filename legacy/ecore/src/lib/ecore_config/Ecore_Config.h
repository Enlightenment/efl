#ifndef _ECORE_CONFIG_H
#define _ECORE_CONFIG_H

/**
 * @file Ecore_Config.h
 * @brief The file that any project using ecore_config will want to include.
 *
 * This file provies all headers and structs for use with Ecore_Config.
 * Using individual header files should not be necessary.
 */

/**
 * @mainpage Enlightened Configuration Library Documentation
 *
 * @image html ecore_config_mini.png
 *
 * @section intro Introduction
 *
 * The Enlightened Property Library (Ecore_Config) is an adbstraction from the 
 * complexities of writing your own configuration. It provides many features
 * using the Enlightenment 17 development libraries.
 */

#include "config.h"

#define DIR_DELIMITER      '/'
#define ECORE_CONFIG_FLOAT_PRECISION 1000

#ifndef TRUE
#  define FALSE (0)
#  define TRUE  (!FALSE)
#endif

#include <Ecore_Ipc.h>

/* debug */
#define DEBUG 999

#ifdef ECORE_CONFIG_DEBUG
#  define D(fmt,args...) do { if(DEBUG>=0) fprintf(stderr,fmt,## args); } while(0);
#  define E(lvl,args...) do { if(DEBUG>=(lvl)) fprintf(stderr,## args); } while(0)
#else
#  define D(msg,args...)
#  define E(lvl,args...) do { } while(0)
#endif


/* structures */

typedef enum Ecore_Config_Type {
  PT_NIL=0,
  PT_INT=1,
  PT_FLT=2,
  PT_STR=3,
  PT_RGB=4
} Ecore_Config_Type;



typedef enum Ecore_Config_Flag {
  PF_NONE=0,
  PF_BOUNDS=1,
  PF_MODIFIED=2
} Ecore_Config_Flag;



typedef int (*Ecore_Config_Listener)(const char *key,const Ecore_Config_Type type,const int tag,void *data,void *bundle);


typedef struct Ecore_Config_Listener_List {
  Ecore_Config_Listener              listener;
  const char                  *name;
  void                        *data;
  int                          tag;
  struct Ecore_Config_Listener_List *next; } Ecore_Config_Listener_List;

/**
 * The actual property for storing a key-value pair.
 */
typedef struct Ecore_Config_Prop {
  char                  *key;
  char                  *description;
  char                  *ptr;
  Ecore_Config_Type            type;
  long                   val,lo,hi,step;
  Ecore_Config_Flag            flags;
  Ecore_Config_Listener_List  *listeners;
  struct Ecore_Config_Prop    *next; /**< pointer to the next property in the list */
  } Ecore_Config_Prop;

/**
 * A container for a list of properties. Provided so that an application can
 * use different set of properties at any time. This is useful for multiple
 * window support.
 */
typedef struct Ecore_Config_Bundle {
  char *identifier; /**< identifier for this set of properties (window ID for example) */
  char *owner; /**< this is used to store the application name related to the bundle */
  long serial; /**< unique identifier to identify bundle */
  Ecore_Config_Prop *data; /**< pointer to root of property list */
  void *user_data; /**< app specific pointer to "other data" */
  struct Ecore_Config_Bundle *next; /**< pointer to next bundle in this application */
  } Ecore_Config_Bundle;

typedef struct Ecore_Config_Server {
  void                  *server;
  char                  *name;
  Ecore_Config_Bundle        *bundles;          /* data anchor */
  struct Ecore_Config_Server *next; } Ecore_Config_Server;


Ecore_Config_Prop   *ecore_config_get(const Ecore_Config_Bundle *t,const char *key);
const char    *ecore_config_get_type(const Ecore_Config_Prop *e);
void          *ecore_config_get_data(const Ecore_Config_Bundle *t,const char *key);
char          *ecore_config_get_string(const Ecore_Config_Bundle *t,const char *key);
long           ecore_config_get_int(const Ecore_Config_Bundle *t,const char *key);
int            ecore_config_get_rgb(const Ecore_Config_Bundle *t,const char *key,int *r, int *g, int *b);
float          ecore_config_get_float(const Ecore_Config_Bundle *t,const char *key);
char          *ecore_config_get_as_string(const Ecore_Config_Bundle *t,const char *key);
char          *ecore_config_canonize_key(char *,int modify);
int            ecore_config_set(Ecore_Config_Bundle *t,const char *key,char *val);
int            ecore_config_set_string(Ecore_Config_Bundle *t,const char *key,char *val);
int            ecore_config_set_int(Ecore_Config_Bundle *t,const char *key,int val);
int            ecore_config_set_rgb(Ecore_Config_Bundle *t,const char *key,char *val);
char          *ecore_config_get_rgbstr(const Ecore_Config_Bundle *t,const char *key);
int            ecore_config_set_float(Ecore_Config_Bundle *t,const char *key,float val);
int            ecore_config_set_as_string(Ecore_Config_Bundle *t,const char *key,char *val);

int            ecore_config_default(Ecore_Config_Bundle *t,const char *key,char *val,float lo,float hi,float step);
int            ecore_config_default_int(Ecore_Config_Bundle *t,const char *key,int val);
int            ecore_config_default_int_bound(Ecore_Config_Bundle *t,const char *key,int val,int lo,int hi,int step);
int            ecore_config_default_string(Ecore_Config_Bundle *t,const char *key,char *val);
int            ecore_config_default_float(Ecore_Config_Bundle *t,const char *key,float val);
int            ecore_config_default_float_bound(Ecore_Config_Bundle *t,const char *key,float val,float lo,float hi,float step);
int            ecore_config_default_rgb(Ecore_Config_Bundle *t,const char *key,char *val);

int            ecore_config_listen(Ecore_Config_Bundle *t,const char *name,const char *key,Ecore_Config_Listener listener,int tag,void *data);
int            ecore_config_deaf(Ecore_Config_Bundle *t,const char *name,const char *key,Ecore_Config_Listener listener);
Ecore_Config_Prop   *ecore_config_dst(Ecore_Config_Bundle *t,Ecore_Config_Prop *e);
int ecore_config_guess_type(char *val);

Ecore_Config_Bundle *ecore_config_bundle_new(Ecore_Config_Server *srv, const char *id);
Ecore_Config_Bundle *ecore_config_bundle_get_1st(Ecore_Config_Server *srv);
Ecore_Config_Bundle *ecore_config_bundle_get_next(Ecore_Config_Bundle *ns);
Ecore_Config_Bundle *ecore_config_bundle_get_by_serial(Ecore_Config_Server *srv, long serial);
Ecore_Config_Bundle *ecore_config_bundle_get_by_label(Ecore_Config_Server *srv, const char *label);
long           ecore_config_bundle_get_serial(Ecore_Config_Bundle *ns);
char          *ecore_config_bundle_get_label(Ecore_Config_Bundle *ns);
                      

Ecore_Config_Server *ecore_config_init(char *name);
Ecore_Config_Server *ecore_config_init_global(char *name);
int            ecore_config_exit(void);

int            ecore_config_load(Ecore_Config_Bundle *b);
int            ecore_config_load_file(Ecore_Config_Bundle *b, char *file);
int            ecore_config_save(Ecore_Config_Bundle *b);
int            ecore_config_save_file(Ecore_Config_Bundle *b, char *file);

/* error codes */
#  define ECORE_CONFIG_ERR_NOTSUPP     (-16)
#  define ECORE_CONFIG_ERR_NOFILE      (-15)
#  define ECORE_CONFIG_ERR_META_DLFAIL (-14)
#  define ECORE_CONFIG_ERR_META_FILE   (-13)
#  define ECORE_CONFIG_ERR_META_FORMAT (-12)
#  define ECORE_CONFIG_ERR_MONMIS      (-11)
#  define ECORE_CONFIG_ERR_NOEXEC      (-10)
#  define ECORE_CONFIG_ERR_PARTIAL      (-9)
#  define ECORE_CONFIG_ERR_PATHEX       (-8)
#  define ECORE_CONFIG_ERR_TYPEMISMATCH (-7)
#  define ECORE_CONFIG_ERR_MUTEX        (-6)
#  define ECORE_CONFIG_ERR_NOTFOUND     (-5)
#  define ECORE_CONFIG_ERR_OOM          (-4)
#  define ECORE_CONFIG_ERR_IGNORED      (-3)
#  define ECORE_CONFIG_ERR_NODATA       (-2)
#  define ECORE_CONFIG_ERR_FAIL         (-1)
#  define ECORE_CONFIG_ERR_SUCC          (0)

#endif
