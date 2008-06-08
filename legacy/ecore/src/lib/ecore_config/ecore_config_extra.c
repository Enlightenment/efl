#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "Ecore_Config.h"
#include "Ecore.h"

#include "config.h"

typedef struct __Ecore_Config_Arg_Callback _Ecore_Config_Arg_Callback;
struct __Ecore_Config_Arg_Callback
{
   char		  short_opt;
   char		 *long_opt;
   char		 *description;
   void		 *data;
   void		(*func)(char *val, void *data);
   Ecore_Config_Type type;
   _Ecore_Config_Arg_Callback *next;
};

char               *__ecore_config_app_description;
_Ecore_Config_Arg_Callback *_ecore_config_arg_callbacks;

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
 * Creates a new boolean property, if it does not already exist, and sets its
 * attributes to those given.
 * @param   key       The property key.
 * @param   val       Default boolean value of key.
 * @param   short_opt Short option used to set the property from command
 *                    line.
 * @param   long_opt  Long option used to set the property from command line.
 * @param   desc      String description of property.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Create_Group
 */
int
ecore_config_boolean_create(const char *key, int val, char short_opt,
			char *long_opt, char *desc)
{
   return
      ecore_config_typed_create(key, (void *)&val, ECORE_CONFIG_BLN, short_opt, long_opt,
				desc);
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
      ecore_config_typed_create(key, (void *)&val, ECORE_CONFIG_INT, short_opt, long_opt,
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
      ecore_config_typed_create(key, (void *)&val, ECORE_CONFIG_INT, short_opt, long_opt,
				desc);
   if (ret != ECORE_CONFIG_ERR_SUCC)
      return ret;
   e = ecore_config_get(key);
   if (e)
     {
	e->step = step;
	e->flags |= ECORE_CONFIG_FLAG_BOUNDS;
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
      ecore_config_typed_create(key, (void *)val, ECORE_CONFIG_STR, short_opt, long_opt,
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
      ecore_config_typed_create(key, (void *)&val, ECORE_CONFIG_FLT, short_opt, long_opt,
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
      ecore_config_typed_create(key, (void *)&val, ECORE_CONFIG_FLT, short_opt, long_opt,
				desc);
   e = ecore_config_get(key);
   if (e)
     {
	e->step = (int)(step * ECORE_CONFIG_FLOAT_PRECISION);
	e->flags |= ECORE_CONFIG_FLAG_BOUNDS;
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
ecore_config_argb_create(const char *key, char *val, char short_opt,
			char *long_opt, char *desc)
{
   return
      ecore_config_typed_create(key, (void *)val, ECORE_CONFIG_RGB, short_opt, long_opt,
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
      ecore_config_typed_create(key, (void *)val, ECORE_CONFIG_THM, short_opt, long_opt,
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

/**
 * Retrieves the default theme search path.
 *
 * @return The default theme search path.
 */
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
 * Retrieves the search path used to find themes.
 *
 * The search path is stored in the property "/e/themes/search_path".  If
 * the property has not been set, the default path used is
 * "/usr/local/share/<app_name>/themes|~/.e/apps/<app_name>/themes".
 * See @ref ecore_config_theme_default_path_get for more information about
 * the default path.
 *
 * @return The search path.  @c NULL is returned if there is no memory left.
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
 * Adds the given path to the search path used to find themes.
 *
 * If the search path is successfully, the new search path will be saved
 * into the property "/e/themes/search_path".  Therefore, this function
 * should be called @b after @ref ecore_config_load to allow a user to
 * override the default search path.
 *
 * @param  path The given 
 * @return @c ECORE_CONFIG_ERR_SUCC on success.  @c ECORE_CONFIG_ERR_FAIL
 *         will be returned if @p path already exists in the search path.
 *         @c ECORE_CONFIG_ERR_FAIL is returned if @p path is @c NULL.
 */
int
ecore_config_theme_search_path_append(const char *path)
{
   char               *search_path, *loc, *new_search_path;
   int                 len, search_len;
   Ecore_Config_Prop  *prop;

   if (!path)
     return ECORE_CONFIG_ERR_NODATA;
   search_path = ecore_config_theme_search_path_get();

   loc = strstr(search_path, path);
   len = strlen(path);
   search_len = strlen(search_path);
   
   if (loc == NULL || (loc != search_path && *(loc - 1) != '|') || 
       (loc != (search_path + search_len - len) && *(loc + len - 1) != '|'))
     {
	new_search_path = malloc(search_len + len + 2); /* 2 = \0 + | */
	strcpy(new_search_path, search_path);
	strncat(new_search_path, "|", 1);
	strncat(new_search_path, path, len);

	ecore_config_string_set("/e/themes/search_path", new_search_path);
	prop = ecore_config_get("/e/themes/search_path");
	if (prop)
	  prop->flags &= ~ECORE_CONFIG_FLAG_MODIFIED;

	free(new_search_path);

	return ECORE_CONFIG_ERR_SUCC;
     }
   return ECORE_CONFIG_ERR_FAIL;
}

/**
 * Retrieve a theme file's full path.
 *
 * The search path for theme files is given by @ref
 * ecore_config_theme_search_path_get .
 *
 * @param  name The name of the theme.
 * @return A full path to the theme on success.  @c NULL will be returned
 *         if @p name is @c NULL or no theme matching the given name could
 *         be found.
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
           /* 6 = / + .edj + \0 */

        snprintf(file, strlen(search_path_tmp) + strlen(name) + 6, 
                      "%s/%s.edj", search_path_tmp, name);
	
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
 * Retrieves the full path to the theme file of the theme stored in the
 * given property.
 *
 * The search path for themes is given by @ref
 * ecore_config_theme_search_path_get .
 *
 * @param  key The given property.
 * @return A full path to the theme on success, or @c NULL on failure.
 *         This function will fail if no key is specified or not theme
 *         matching that given by the property @p key could be found.
 */
char               *
ecore_config_theme_with_path_get(const char *key)
{
   return
      ecore_config_theme_with_path_from_name_get(ecore_config_theme_get(key));
}

static const char  *_ecore_config_short_types[] =
   { "      ", "<int> ", "<flt> ", "<str> ", "<rgb> ", "<str> ", "<bool>" };

/**
 * Prints the property list of the local configuration bundle to output.
 */
void
ecore_config_args_display(void)
{
   Ecore_Config_Prop  *props;
   _Ecore_Config_Arg_Callback *callbacks;

   if (__ecore_config_app_description)
      printf("%s\n\n", __ecore_config_app_description);
   printf("Supported Options:\n");
   printf(" -h, --help\t       Print this text\n");
   if (!__ecore_config_bundle_local)
      return;
   props = __ecore_config_bundle_local->data;
   while (props)
     {
	/* if it is a system prop, or cannot be set on command line hide it */
	if (props->flags & ECORE_CONFIG_FLAG_SYSTEM || (!props->short_opt && !props->long_opt))
	  {
	     props = props->next;
	     continue;
	  }
	printf(" %c%c%c --%s\t%s %s\n", props->short_opt ? '-' : ' ',
	       props->short_opt ? props->short_opt : ' ',
	       props->short_opt ? ',' : ' ',
	       props->long_opt ? props->long_opt : props->key,
	       _ecore_config_short_types[props->type],
	       props->description ? props->description :
	       "(no description available)");

	props = props->next;
     }
   callbacks = _ecore_config_arg_callbacks;
   while (callbacks)
     {
        printf(" %c%c%c --%s\t%s %s\n", callbacks->short_opt ? '-' : ' ',
	       callbacks->short_opt ? callbacks->short_opt : ' ',
	       callbacks->short_opt ? ',' : ' ',
	       callbacks->long_opt ? callbacks->long_opt : "",
               _ecore_config_short_types[callbacks->type],
	       callbacks->description ? callbacks->description :
	       "(no description available)");

	callbacks = callbacks->next;
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
     {
	ecore_config_set(prop->key, arg);
	prop->flags |= ECORE_CONFIG_FLAG_CMDLN;
     }   
   return ECORE_CONFIG_PARSE_CONTINUE;
}

static void
ecore_config_args_callback_add(char short_opt, char *long_opt, char *desc,
			       void (*func)(char *val, void *data),
			       void *data, Ecore_Config_Type type) {
   _Ecore_Config_Arg_Callback *new_cb;

   new_cb = malloc(sizeof(_Ecore_Config_Arg_Callback));
   new_cb->short_opt = short_opt;
   if (long_opt)
      new_cb->long_opt = strdup(long_opt);
   if (desc)
      new_cb->description = strdup(desc);
   new_cb->data = data;
   new_cb->func = func;
   new_cb->type = type;

   new_cb->next = _ecore_config_arg_callbacks;
   _ecore_config_arg_callbacks = new_cb;
}

void
ecore_config_args_callback_str_add(char short_opt, char *long_opt, char *desc,
			           void (*func)(char *val, void *data),
			           void *data) {
   ecore_config_args_callback_add(short_opt, long_opt, desc, func, data, ECORE_CONFIG_STR);
}

void
ecore_config_args_callback_noarg_add(char short_opt, char *long_opt, char *desc,
			             void (*func)(char *val, void *data),
			             void *data) {
   ecore_config_args_callback_add(short_opt, long_opt, desc, func, data, ECORE_CONFIG_NIL);
}

/**
 * Parse the arguments set by @ref ecore_app_args_set and set properties
 * accordingly.
 *
 * @return @c ECORE_CONFIG_PARSE_CONTINUE if successful.
 *         @c ECORE_CONFIG_PARSE_EXIT is returned if an unrecognised option
 *         is found.  @c ECORE_CONFIG_PARSE_HELP is returned if help was
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
   _Ecore_Config_Arg_Callback *callback;

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
			   ECORE_CONFIG_PARSE_CONTINUE)
		          return ret;
		       break;
		    }
		  prop = prop->next;
	       }
	     if (!found)
	       {
		  callback = _ecore_config_arg_callbacks;
		  while (callback)
		    {
		       if ((callback->long_opt && 
			    !strcmp(long_opt, callback->long_opt)))
			 {
			    found = 1;
			    if (callback->type == ECORE_CONFIG_NIL)
			      {
				 callback->func(NULL, callback->data);
			      }
			    else 
			      {
				 if (!argv[++nextarg])
				   {
				      printf("Missing expected argument for option --%s\n", long_opt);
				      return ECORE_CONFIG_PARSE_EXIT;
				   }
				   callback->func(argv[nextarg], callback->data);
			      }
			    break;
			 }
		       callback = callback->next;
		    }
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
		 	    callback = _ecore_config_arg_callbacks;
			    while (callback)
			      {
				 if (short_opt == callback->short_opt)
				   {
				      found = 1;
				      if (callback->type == ECORE_CONFIG_NIL)
					{
					   callback->func(NULL, callback->data);
					}
				      else
					{
					   if (!argv[++nextarg])
					     {
						printf("Missing expected argument for option -%c\n", short_opt);
						return ECORE_CONFIG_PARSE_EXIT;
					     }
					   callback->func(argv[nextarg], callback->data);
					}
				      break;
				   }
				 callback = callback->next;
			      }
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
