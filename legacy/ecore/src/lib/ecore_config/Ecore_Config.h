#ifndef _ECORE_CONFIG_H
# define _ECORE_CONFIG_H

#ifdef EAPI
#undef EAPI
#endif
#ifdef _MSC_VER
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

/**
 * @file 
 * @brief Provides the Enlightened Property Library.
 *
 * This file provies all headers and structs for use with Ecore_Config.
 * Using individual header files should not be necessary.
 */

# define DIR_DELIMITER      '/'
# define ECORE_CONFIG_FLOAT_PRECISION 1000

/* FIXME: this should only be included if evas is present */
# include <Evas.h>

# define ECORE_CONFIG_GLOBAL_ID "_system"

/* structures */

/**
 * Valid configuration property types.
 */
typedef enum Ecore_Config_Type
{
   ECORE_CONFIG_NIL = 0,			/**< Property with no value. */
   ECORE_CONFIG_INT = 1,			/**< Integer property type. */
   ECORE_CONFIG_FLT = 2,			/**< Float property type. */
   ECORE_CONFIG_STR = 3,			/**< String property type. */
   ECORE_CONFIG_RGB = 4,			/**< Colour property type. */
   ECORE_CONFIG_THM = 5,			/**< Theme property type. */
   ECORE_CONFIG_BLN = 6,			/**< Boolean property type. */
   ECORE_CONFIG_SCT = 7,      /**< Structure property type */
} Ecore_Config_Type;

typedef enum Ecore_Config_Flag
{
   ECORE_CONFIG_FLAG_NONE = 0,
   ECORE_CONFIG_FLAG_BOUNDS = 1,
   ECORE_CONFIG_FLAG_MODIFIED = 2,
   ECORE_CONFIG_FLAG_SYSTEM = 4,
   ECORE_CONFIG_FLAG_CMDLN = 8
} Ecore_Config_Flag;

/**
 * Property change callback function prototype.
 */
typedef int         (*Ecore_Config_Listener) (const char *key,
					      const Ecore_Config_Type type,
					      const int tag, void *data);

typedef struct Ecore_Config_Listener_List
{
   Ecore_Config_Listener listener;
   const char         *name;
   void               *data;
   int                 tag;
   struct Ecore_Config_Listener_List *next;
} Ecore_Config_Listener_List;

/**
 * The actual property for storing a key-value pair.
 */
typedef struct Ecore_Config_Prop
{
   char               *key;	/* Property key. */
   char               *description;	/* Description set by ecore_config_descibe. */
   char                short_opt;	/* short identifier on command line (-f) */
   char               *long_opt;	/* long identifier on command line (--foo) */
   char               *ptr;	/* Used as the value when the property is a string or theme. */
   Ecore_Config_Type   type;	/* Property type. */
   long                val;	/* Used as the value when the property is an integer, float or colour. */
   long                lo;	/* Lower bound for the value when the property is an integer or float. */
   long                hi;	/* Higher bound for the value when the property is an integer or float. */
   long                step;	/* Increment for the value when the property is an integer or float. */
   Ecore_Config_Flag   flags;	/// < Configuration flags.
   Ecore_Config_Listener_List *listeners;	/* List of change listeners. */
   void               *data;	/// < Stores extra data for the property.
   struct Ecore_Config_Prop *parent; /* if we are in a struct we have a parent to notify of changes etc */
   struct Ecore_Config_Prop *next;	/* Pointer to the next property in the list. */
} Ecore_Config_Prop;

/*
 * A container for a list of properties.  Provided so that an
 * application can use different set of properties at any time. This
 * is useful for multiple window support.
 */
typedef struct Ecore_Config_Bundle
{
   char               *identifier;	/* Identifier for this set of properties (window ID for example) */
   char               *owner;	/* This is used to store the application name related to the bundle */
   long                serial;	/* Unique identifier to identify bundle */
   Ecore_Config_Prop  *data;	/* Pointer to root of property list */
   void               *user_data;	/* App specific pointer to "other data" */
   struct Ecore_Config_Bundle *next;	/* Pointer to next bundle in this application */
} Ecore_Config_Bundle;

typedef struct Ecore_Config_Server
{
   void               *server;
   char               *name;
   Ecore_Config_Bundle *bundles;	/* data anchor */
   struct Ecore_Config_Server *next;
} Ecore_Config_Server;

# ifdef __cplusplus
extern              "C"
{
# endif

/* global ptrs to save passing them through the API */
   EAPI extern Ecore_Config_Server *__ecore_config_server_global;
   EAPI extern Ecore_Config_Server *__ecore_config_server_local;
   EAPI extern Ecore_Config_Bundle *__ecore_config_bundle_local;
   EAPI extern char        *__ecore_config_app_name;

   EAPI Ecore_Config_Prop  *ecore_config_get(const char *key);
   EAPI const char         *ecore_config_type_get(const Ecore_Config_Prop *e);
   EAPI int                 ecore_config_boolean_get(const char *key);
   EAPI char               *ecore_config_string_get(const char *key);
   EAPI long                ecore_config_int_get(const char *key);
   EAPI int                 ecore_config_argb_get(const char *key, int *a, int *r,
						  int *g, int *b);
   EAPI long                ecore_config_argbint_get(const char *key);
   EAPI char               *ecore_config_argbstr_get(const char *key);
   EAPI float               ecore_config_float_get(const char *key);
   EAPI char               *ecore_config_theme_get(const char *key);
   EAPI char               *ecore_config_as_string_get(const char *key);
   EAPI int                 ecore_config_bound(Ecore_Config_Prop *e);
   EAPI int                 ecore_config_describe(const char *key, const char *desc);
   EAPI int                 ecore_config_short_opt_set(const char *key,
						       char short_opt);
   EAPI int                 ecore_config_long_opt_set(const char *key,
						      const char *long_opt);
   EAPI int                 ecore_config_set(const char *key, const char *val);
   EAPI int                 ecore_config_typed_set(const char *key, const void *val,
						   int type);
   EAPI int                 ecore_config_boolean_set(const char *key, int val);
   EAPI int                 ecore_config_string_set(const char *key, const char *val);
   EAPI int                 ecore_config_int_set(const char *key, int val);
   EAPI int                 ecore_config_argb_set(const char *key, int a, int r, int g, int b);
   EAPI int                 ecore_config_argbint_set(const char *key, long argb);
   EAPI int                 ecore_config_argbstr_set(const char *key, const char *val);
   EAPI int                 ecore_config_float_set(const char *key, float val);
   EAPI int                 ecore_config_theme_set(const char *key, const char *val);
   EAPI int                 ecore_config_theme_preview_group_set(const char *key,
								 const char *group);
   EAPI int                 ecore_config_as_string_set(const char *key, const char *val);

   EAPI int                 ecore_config_default(const char *key, const char *val,
						 float lo, float hi, float step);
   EAPI int                 ecore_config_typed_default(const char *key, const void *val,
						       int type);
   EAPI int                 ecore_config_boolean_default(const char *key, int val);
   EAPI int                 ecore_config_int_default(const char *key, int val);
   EAPI int                 ecore_config_int_default_bound(const char *key, int val,
							   int lo, int hi, int step);
   EAPI int                 ecore_config_string_default(const char *key, const char *val);
   EAPI int                 ecore_config_float_default(const char *key, float val);
   EAPI int                 ecore_config_float_default_bound(const char *key,
							     float val, float lo,
							     float hi, float step);
   EAPI int                 ecore_config_argb_default(const char *key, int a, int r, int g, int b);
   EAPI int                 ecore_config_argbint_default(const char *key, long argb);
   EAPI int                 ecore_config_argbstr_default(const char *key, const char *val);
   EAPI int                 ecore_config_theme_default(const char *key, const char *val);
   EAPI int                 ecore_config_struct_default(const char *key);
   EAPI int                 ecore_config_struct_int_add(const char *key, const char *name, int val);
   EAPI int                 ecore_config_struct_float_add(const char *key, const char *name, float val);
   EAPI int                 ecore_config_struct_create(const char *key);
   EAPI int                 ecore_config_struct_string_add(const char *key, const char *name, const char* val);
   EAPI int                 ecore_config_struct_theme_add(const char *key, const char *name, const char* val);
   EAPI int                 ecore_config_struct_argb_add(const char *key, const char *name, int a, int r, int g, int b);
   EAPI int                 ecore_config_struct_boolean_add(const char *key, const char *name, int val);
   EAPI int                 ecore_config_struct_get(const char *key, void *data);

   EAPI int                 ecore_config_listen(const char *name, const char *key,
						Ecore_Config_Listener listener,
						int tag, void *data);
   EAPI int                 ecore_config_deaf(const char *name, const char *key,
					      Ecore_Config_Listener listener);
   EAPI Ecore_Config_Prop  *ecore_config_dst(Ecore_Config_Prop *e);
   EAPI int                 ecore_config_type_guess(const char *key, const char *val);

   EAPI Ecore_Config_Bundle *ecore_config_bundle_new(Ecore_Config_Server *srv,
						     const char *id);
   EAPI Ecore_Config_Bundle *ecore_config_bundle_1st_get(Ecore_Config_Server *srv);
   EAPI Ecore_Config_Bundle *ecore_config_bundle_next_get(Ecore_Config_Bundle *ns);
   EAPI Ecore_Config_Bundle *ecore_config_bundle_by_serial_get(Ecore_Config_Server *srv,
							       long serial);
   EAPI Ecore_Config_Bundle *ecore_config_bundle_by_label_get(Ecore_Config_Server *srv,
							      const char *label);
   EAPI long                ecore_config_bundle_serial_get(Ecore_Config_Bundle *ns);
   EAPI char               *ecore_config_bundle_label_get(Ecore_Config_Bundle *ns);

   EAPI int                 ecore_config_init(const char *name);
   EAPI int                 ecore_config_shutdown(void);

   EAPI int                 ecore_config_system_init(void);
   EAPI int                 ecore_config_system_shutdown(void);

   EAPI int                 ecore_config_load(void);
   EAPI int                 ecore_config_file_load(const char *file);
   EAPI int                 ecore_config_save(void);
   EAPI int                 ecore_config_file_save(const char *file);

/* error codes */
# define ECORE_CONFIG_ERR_NOTSUPP     (-16)
# define ECORE_CONFIG_ERR_NOFILE      (-15)
# define ECORE_CONFIG_ERR_META_DLFAIL (-14)
# define ECORE_CONFIG_ERR_META_FILE   (-13)
# define ECORE_CONFIG_ERR_META_FORMAT (-12)
# define ECORE_CONFIG_ERR_MONMIS      (-11)
# define ECORE_CONFIG_ERR_NOEXEC      (-10)
# define ECORE_CONFIG_ERR_PARTIAL      (-9)
# define ECORE_CONFIG_ERR_PATHEX       (-8)
# define ECORE_CONFIG_ERR_TYPEMISMATCH (-7)
# define ECORE_CONFIG_ERR_MUTEX        (-6)
# define ECORE_CONFIG_ERR_NOTFOUND     (-5)	/* Error indicating that the item searched for could not be found. */
# define ECORE_CONFIG_ERR_OOM          (-4)	/* Error given when the program runs out of memory. */
# define ECORE_CONFIG_ERR_IGNORED      (-3)	/* Error occurred, but was ignored. */
# define ECORE_CONFIG_ERR_NODATA       (-2)	/* Error given when necessary data is not provided. */
# define ECORE_CONFIG_ERR_FAIL         (-1)	/* Failure result. */
# define ECORE_CONFIG_ERR_SUCC          (0)	/* Success result. */

# define ECORE_CONFIG_PARSE_HELP       (-2)	/* Help was displayed */
# define ECORE_CONFIG_PARSE_EXIT       (-1)	/* An error occurred */
# define ECORE_CONFIG_PARSE_CONTINUE    (0)	/* Arguments parsed successfully */

/* convenience mathods in convenience.c */
   /* FIXME: this should only be included if evas is present */
   EAPI int                 ecore_config_evas_font_path_apply(Evas *evas);
   EAPI char               *ecore_config_theme_search_path_get(void);
   EAPI int                 ecore_config_theme_search_path_append(const char *append);
     
   EAPI char               *ecore_config_theme_default_path_get(void);
   EAPI char               *ecore_config_theme_with_path_from_name_get(char *name);
   EAPI char               *ecore_config_theme_with_path_get(const char *key);
   EAPI void                ecore_config_args_display(void);
   EAPI int                 ecore_config_args_parse(void);
   EAPI void                ecore_config_args_callback_str_add(char short_opt,
							       char *long_opt, char *desc,
							       void (*func)(char *val, void *data),
							       void *data);
   EAPI void                ecore_config_args_callback_noarg_add(char short_opt,
								 char *long_opt, char *desc,
								 void (*func)(char *val, void *data),
								 void *data);
   EAPI void                ecore_config_app_describe(char *description);

   EAPI int                 ecore_config_create(const char *key, void *val,
						char short_opt, char *long_opt,
						char *desc);
   EAPI int                 ecore_config_typed_create(const char *key, void *val,
						      int type, char short_opt,
						      char *long_opt, char *desc);
   EAPI int                 ecore_config_boolean_create(const char *key, int val,
							char short_opt, char *long_opt,
							char *desc);
   EAPI int                 ecore_config_int_create(const char *key, int val,
						    char short_opt, char *long_opt,
						    char *desc);
   EAPI int                 ecore_config_int_create_bound(const char *key, int val,
							  int low, int high,
							  int step, char short_opt,
							  char *long_opt,
							  char *desc);
   EAPI int                 ecore_config_string_create(const char *key, char *val,
						       char short_opt,
						       char *long_opt, char *desc);
   EAPI int                 ecore_config_float_create(const char *key, float val,
						      char short_opt, char *long_opt,
						      char *desc);
   EAPI int                 ecore_config_float_create_bound(const char *key,
							    float val, float low,
							    float high, float step,
							    char short_opt,
							    char *long_opt,
							    char *desc);
   EAPI int                 ecore_config_argb_create(const char *key, char *val,
						     char short_opt, char *long_opt,
						     char *desc);
   EAPI int                 ecore_config_theme_create(const char *key, char *val,
						      char short_opt, char *long_opt,
						      char *desc);

# ifdef __cplusplus
}
# endif
#endif
