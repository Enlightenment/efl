#include "Ecore_Config.h"
#include "Ecore.h"

#include "config.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

char               *__ecore_config_app_description;

extern int          ecore_config_bound(Ecore_Config_Prop * e);

/* shorthand prop setup code to make client apps a little smaller ;) */

/**
 * Creates a new property, if it does not already exist, and sets its
 * attributes to those given.
 *
 * The type of the property is guessed from the key and the value
 * given.
 *
 * @param   key       The property key.
 * @param   val       Pointer to default value of key.
 * @param   short_opt Short option used to set the property from command
 *                    line.
 * @param   long_opt  Long option used to set the property from command line.
 * @param   desc      String description of property.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Create_Group
 */
int
ecore_config_create(const char *key, void *val, char short_opt, char *long_opt,
		    char *desc)
{
   int                 type = ecore_config_type_guess(key, val);

   return ecore_config_typed_create(key, val, type, short_opt, long_opt, desc);
}

/**
 * Creates a new property, if it does not already exist, and sets its
 * attributes to those given.
 * @param   key       The property key.
 * @param   val       Pointer to default value of key.
 * @param   type      Type of the property.
 * @param   short_opt Short option used to set the property from
 *                    command line.
 * @param   long_opt  Long option used to set the property from command line.
 * @param   desc      String description of property.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Create_Group
 */
int
ecore_config_typed_create(const char *key, void *val, int type, char short_opt,
			  char *long_opt, char *desc)
{
   int                 ret;

   if ((ret =
	ecore_config_typed_default(key, val, type)) != ECORE_CONFIG_ERR_SUCC)
      return ret;
   if ((ret =
	ecore_config_short_opt_set(key, short_opt)) != ECORE_CONFIG_ERR_SUCC)
      return ret;
   if ((ret =
	ecore_config_long_opt_set(key, long_opt)) != ECORE_CONFIG_ERR_SUCC)
      return ret;
   ret = ecore_config_describe(key, desc);
   return ret;
}

/**
 * Creates a new integer property, if it does not already exist, and sets its
 * attributes to those given.
 * @param   key       The property key.
 * @param   val       Default integer value of key.
 * @param   short_opt Short option used to set the property from command
 *                    line.
 * @param   long_opt  Long option used to set the property from command line.
 * @param   desc      String description of property.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Create_Group
 */
int
ecore_config_int_create(const char *key, int val, char short_opt,
			char *long_opt, char *desc)
{
   return
      ecore_config_typed_create(key, (void *)&val, PT_INT, short_opt, long_opt,
				desc);
}

/**
 * Creates a new integer property, if it does not already exist, and sets its
 * attributes to those given.
 * @param   key       The property key.
 * @param   val       Default integer value of key.
 * @param   low       Lowest valid integer value for the property.
 * @param   high      Highest valid integer value for the property.
 * @param   step      Increment value for the property.
 * @param   short_opt Short option used to set the property from command
 *                    line.
 * @param   long_opt  Long option used to set the property from command line.
 * @param   desc      String description of property.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Create_Group
 */
int
ecore_config_int_create_bound(const char *key, int val, int low, int high,
			      int step, char short_opt, char *long_opt,
			      char *desc)
{
   Ecore_Config_Prop  *e;
   int                 ret;

   ret =
      ecore_config_typed_create(key, (void *)&val, PT_INT, short_opt, long_opt,
				desc);
   if (ret != ECORE_CONFIG_ERR_SUCC)
      return ret;
   e = ecore_config_get(key);
   if (e)
     {
	e->step = step;
	e->flags |= PF_BOUNDS;
	e->lo = low;
	e->hi = high;
	ecore_config_bound(e);
     }
   return ret;
}

/**
 * Creates a new string property, if it does not already exist, and sets its
 * attributes to those given.
 * @param   key       The property key.
 * @param   val       Default value of key.
 * @param   short_opt Short option used to set the property from command
 *                    line.
 * @param   long_opt  Long option used to set the property from command line.
 * @param   desc      String description of property.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Create_Group
 */
int
ecore_config_string_create(const char *key, char *val, char short_opt,
			   char *long_opt, char *desc)
{
   return
      ecore_config_typed_create(key, (void *)val, PT_STR, short_opt, long_opt,
				desc);
}

/**
 * Creates a new float property, if it does not already exist, and sets its
 * attributes to those given.
 * @param   key       The property key.
 * @param   val       Default float value of key.
 * @param   short_opt Short option used to set the property from command
 *                    line.
 * @param   long_opt  Long option used to set the property from command line.
 * @param   desc      String description of property.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Create_Group
 */
int
ecore_config_float_create(const char *key, float val, char short_opt,
			  char *long_opt, char *desc)
{
   return
      ecore_config_typed_create(key, (void *)&val, PT_FLT, short_opt, long_opt,
				desc);
}

/**
 * Creates a new float property, if it does not already exist, and sets its
 * attributes to those given.
 * @param   key       The property key.
 * @param   val       Default float value of key.
 * @param   low       Lowest valid float value for the property.
 * @param   high      Highest valid float value for the property.
 * @param   step      Increment value for the property.
 * @param   short_opt Short option used to set the property from command
 *                    line.
 * @param   long_opt  Long option used to set the property from command line.
 * @param   desc      String description of property.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Create_Group
 */
int
ecore_config_float_create_bound(const char *key, float val, float low,
				float high, float step, char short_opt,
				char *long_opt, char *desc)
{
   Ecore_Config_Prop  *e;
   int                 ret;

   ret =
      ecore_config_typed_create(key, (void *)&val, PT_FLT, short_opt, long_opt,
				desc);
   e = ecore_config_get(key);
   if (e)
     {
	e->step = (int)(step * ECORE_CONFIG_FLOAT_PRECISION);
	e->flags |= PF_BOUNDS;
	e->lo = (int)(low * ECORE_CONFIG_FLOAT_PRECISION);
	e->hi = (int)(high * ECORE_CONFIG_FLOAT_PRECISION);
	ecore_config_bound(e);
     }
   return ret;
}

/**
 * Creates a new color property, if it does not already exist, and sets its
 * attributes to those given.
 * @param   key       The property key.
 * @param   val       Default color value of key, as a hexadecimal string.
 * @param   short_opt Short option used to set the property from command
 *                    line.
 * @param   long_opt  Long option used to set the property from command line.
 * @param   desc      String description of property.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Create_Group
 */
int
ecore_config_rgb_create(const char *key, char *val, char short_opt,
			char *long_opt, char *desc)
{
   return
      ecore_config_typed_create(key, (void *)val, PT_RGB, short_opt, long_opt,
				desc);
}

/**
 * Creates a new theme property, if it does not already exist, and sets its
 * attributes to those given.
 * @param   key       The property key.
 * @param   val       Default theme name for the property.
 * @param   short_opt Short option used to set the property from command
 *                    line.
 * @param   long_opt  Long option used to set the property from command line.
 * @param   desc      String description of property.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Create_Group
 */
int
ecore_config_theme_create(const char *key, char *val, char short_opt,
			  char *long_opt, char *desc)
{
   return
      ecore_config_typed_create(key, (void *)val, PT_THM, short_opt, long_opt,
				desc);
}

/* this should only be built if evas is present */

/**
 * Calls evas_font_path_append on @p evas for each of the font names stored
 * in the property "/e/font/path".
 * @param  evas Evas object to append the font names to.
 * @return @c ECORE_CONFIG_ERR_SUCC on success.  @c ECORE_CONFIG_ERR_NODATA
 *         is returned if the property has not been set.
 */
int
ecore_config_evas_font_path_apply(Evas * evas)
{
   char               *font_path, *font_path_tmp, *ptr, *end;

   font_path = ecore_config_string_get("/e/font/path");

   if (!font_path)
      return ECORE_CONFIG_ERR_NODATA;
   ptr = font_path;
   end = font_path + strlen(font_path);
   font_path_tmp = font_path;
   while (ptr && ptr < end)
     {
	while (*ptr != '|' && ptr < end)
	   ptr++;
	if (ptr < end)
	   *ptr = '\0';

	evas_font_path_append(evas, font_path_tmp);
	ptr++;
	font_path_tmp = ptr;
     }

   free(font_path);

   return ECORE_CONFIG_ERR_SUCC;
}

char               *
ecore_config_theme_default_path_get(void)
{
   char               *path, *home;
   int                 len;

   home = getenv("HOME");
   len = strlen(PACKAGE_DATA_DIR "/../") + strlen(__ecore_config_app_name) +
            strlen("/themes/") + 1;
   if (home)
      len += strlen(home) + strlen("/.e/apps/") +
		strlen(__ecore_config_app_name) +
		strlen("/themes/|"); /* no \0, as that is above */

   if (!(path = malloc(len)))
      return NULL;

   *path = '\0';
   if (home)
     {
        strcat(path, home);
        strcat(path, "/.e/apps/");
        strcat(path, __ecore_config_app_name);
        strcat(path, "/themes/|");
     }
   strcat(path, PACKAGE_DATA_DIR "/../");
   strcat(path, __ecore_config_app_name);
   strcat(path, "/themes/");

   return path;
}

/**
 * Returns the search path used to find themes. This is specified by a user in the
 * property "/e/themes/search_path". If the property is not set the path defaults to
 * "/usr/local/share/<app_name>/themes/|~/.e/apps/<app_name>/themes".
 * Note: This should be called after ecore_config_load() to allow a users overriding
 * search path to be read.
 * @return The search path, or NULL if there is no memory left.
 */
char               *
ecore_config_theme_search_path_get(void)
{
   char               *search_path;
   search_path = ecore_config_string_get("/e/themes/search_path");

   /* this should no longer be the case, as it is defaulted in init */
   if (!search_path)
     {
	search_path = ecore_config_theme_default_path_get();
	if (search_path)
	  {
	     ecore_config_string_default("/e/themes/search_path", search_path);
	     free(search_path);
	  }
     }
   return search_path;
}

/**
 * Get a theme files full path, as it is found according to the search path.
 * The theme searched for is @name (e.g. "winter").
 * The search path is defined by ecore_config_theme_search_path_get().
 * @param  name The theme name to search for.
 * @return A full path to the theme on sucess, or NULL on failure (no key specified or
 *         no theme matching that name could be found).
 */
char               *
ecore_config_theme_with_path_from_name_get(char *name)
{
   char               *search_path, *search_path_tmp, *ptr, *end, *file;
   struct stat         st;

   if (!name)
      return NULL; /* no theme specified (nor a default) */

   search_path = ecore_config_theme_search_path_get();
   ptr = search_path;
   end = search_path + strlen(search_path);
   search_path_tmp = search_path;
   while (ptr && ptr < end)
     {
        while (*ptr != '|' && ptr < end)
           ptr++;
        if (ptr < end)
           *ptr = '\0';

        file = malloc(strlen(search_path_tmp) + strlen(name) + 6);
           /* 6 = / + .eet + \0 */
        snprintf(file, strlen(search_path_tmp) + strlen(name) + 6, 
			"%s/%s.eet", search_path_tmp, name);
	
        if (stat(file, &st) == 0)
          {
              free(search_path);
              return file;
          }
        free(file);
        ptr++;
        search_path_tmp = ptr;
     }

   free(search_path);

   return NULL; /* we could not find the theme with that name in search path */
}

/**
 * Get a theme files full path, as it is found according to the search path.
 * The theme searched for is stored in the property @key.
 * The search path is defined by ecore_config_theme_search_path_get().
 * @param  key The property containing the theme name to search for.
 * @return A full path to the theme on sucess, or NULL on failure (no key specified or
 *         no theme matching that name could be found).
 */
char               *
ecore_config_theme_with_path_get(const char *key)
{
   return
      ecore_config_theme_with_path_from_name_get(ecore_config_theme_get(key));
}

static char        *_ecore_config_short_types[] =
   { "nil", "int", "flt", "str", "rgb", "str" };

/**
 * Prints the property list of the local configuration bundle to output.
 */
void
ecore_config_args_display(void)
{
   Ecore_Config_Prop  *props;

   if (__ecore_config_app_description)
      printf("%s\n\n", __ecore_config_app_description);
   printf("Supported Options:\n");
   printf(" -h, --help\t      Print this text\n");
   if (!__ecore_config_bundle_local)
      return;
   props = __ecore_config_bundle_local->data;
   while (props)
     {
	if (props->flags & PF_SYSTEM)
	  {
	     props = props->next;
	     continue;
	  }
	printf(" %c%c%c --%s\t<%s> %s\n", props->short_opt ? '-' : ' ',
	       props->short_opt ? props->short_opt : ' ',
	       props->short_opt ? ',' : ' ',
	       props->long_opt ? props->long_opt : props->key,
	       _ecore_config_short_types[props->type],
	       props->description ? props->
	       description : "(no description available)");

	props = props->next;
     }
}

static int
ecore_config_parse_set(Ecore_Config_Prop * prop, char *arg, char *opt,
		       char opt2)
{
   if (!arg)
     {
	if (opt)
	   printf("Missing expected argument for option --%s\n", opt);
	else
	   printf("Missing expected argument for option -%c\n", opt2);
	return ECORE_CONFIG_PARSE_EXIT;
     }
   else
      ecore_config_set(prop->key, arg);
   return ECORE_CONFIG_PARSE_CONTINUE;
}

/**
 * Parse the arguments set by @ref ecore_app_args_set and set properties
 * accordingly.
 *
 * @return ECORE_CONFIG_PARSE_CONTINUE if successful.
 *         ECORE_CONFIG_PARSE_EXIT is returned if an unrecognised option
 *         is found.  ECORE_CONFIG_PARSE_HELP is returned if help was
 *         displayed.
 */
int
ecore_config_args_parse(void)
{
   int                 argc;
   char              **argv;
   int                 nextarg, next_short_opt, found, ret;
   char               *arg;
   char               *long_opt, short_opt;
   Ecore_Config_Prop  *prop;

   ecore_app_args_get(&argc, &argv);
   nextarg = 1;
   while (nextarg < argc)
     {
	arg = argv[nextarg];

	if (*arg != '-')
	  {
	     printf("Unexpected attribute \"%s\"\n", arg);
	     nextarg++;
	     continue;
	  }

	next_short_opt = 1;
	short_opt = *(arg + next_short_opt);

	if (short_opt == '-')
	  {
	     long_opt = arg + 2;

	     if (!strcmp(long_opt, "help"))
	       {
		  ecore_config_args_display();
		  return ECORE_CONFIG_PARSE_HELP;
	       }

	     found = 0;
	     prop = __ecore_config_bundle_local->data;
	     while (prop)
	       {
		  if ((prop->long_opt && !strcmp(long_opt, prop->long_opt))
		      || !strcmp(long_opt, prop->key))
		    {
		       found = 1;
		       if ((ret =
			    ecore_config_parse_set(prop, argv[++nextarg],
						   long_opt,
						   '\0')) !=
			   ECORE_CONFIG_PARSE_CONTINUE);
		       return ret;
		       break;
		    }
		  prop = prop->next;
	       }
	     if (!found)
	       {
		  printf("Unrecognised option \"%s\"\n", long_opt);
		  printf("Try using -h or --help for more information.\n\n");
		  return ECORE_CONFIG_PARSE_EXIT;
	       }
	  }
	else
	  {
	     while (short_opt)
	       {
		  if (short_opt == 'h')
		    {
		       ecore_config_args_display();
		       return ECORE_CONFIG_PARSE_HELP;
		    }
		  else
		    {
		       found = 0;
		       prop = __ecore_config_bundle_local->data;
		       while (prop)
			 {
			    if (short_opt == prop->short_opt)
			      {
				 found = 1;
				 if ((ret =
				      ecore_config_parse_set(prop,
							     argv[++nextarg],
							     NULL,
							     short_opt)) !=
				     ECORE_CONFIG_PARSE_CONTINUE)
				    return ret;
				 break;
			      }
			    prop = prop->next;
			 }

		       if (!found)
			 {
			    printf("Unrecognised option '%c'\n", short_opt);
			    printf
			       ("Try using -h or --help for more information.\n\n");
			    return ECORE_CONFIG_PARSE_EXIT;
			 }
		    }
		  short_opt = *(arg + ++next_short_opt);
	       }
	  }
	nextarg++;
     }

   return ECORE_CONFIG_PARSE_CONTINUE;
}

/**
 * Sets the description string used by @ref ecore_config_args_display .
 * @param description Description of application.
 */
void
ecore_config_app_describe(char *description)
{
   if (__ecore_config_app_description)
      free(__ecore_config_app_description);
   __ecore_config_app_description = strdup(description);
}
