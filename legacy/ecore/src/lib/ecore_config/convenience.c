#include "Ecore_Config.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char               *__ecore_config_app_description;

extern int          ecore_config_bound(Ecore_Config_Prop * e);

/* shorthand prop setup code to make client apps a little smaller ;) */

int
ecore_config_create(const char *key, void *val, char short_opt, char *long_opt,
		    char *desc)
{
   int                 type = ecore_config_type_guess(key, val);

   return ecore_config_typed_create(key, val, type, short_opt, long_opt, desc);
}

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

int
ecore_config_int_create(const char *key, int val, char short_opt,
			char *long_opt, char *desc)
{
   return
      ecore_config_typed_create(key, (void *)&val, PT_INT, short_opt, long_opt,
				desc);
}

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

int
ecore_config_string_create(const char *key, char *val, char short_opt,
			   char *long_opt, char *desc)
{
   return
      ecore_config_typed_create(key, (void *)val, PT_STR, short_opt, long_opt,
				desc);
}

int
ecore_config_float_create(const char *key, float val, char short_opt,
			  char *long_opt, char *desc)
{
   return
      ecore_config_typed_create(key, (void *)&val, PT_FLT, short_opt, long_opt,
				desc);
}

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

int
ecore_config_rgb_create(const char *key, char *val, char short_opt,
			char *long_opt, char *desc)
{
   return
      ecore_config_typed_create(key, (void *)val, PT_RGB, short_opt, long_opt,
				desc);
}

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
 * Calls evas_font_path_append on @evas for each of the font names stored
 * in the property "/e/font/path".
 * @param  evas Evas object to append the font names to.
 * @return ECORE_CONFIG_ERR_SUCC on success.  ECORE_CONFIG_ERR_NODATA
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

int
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

void
ecore_config_app_describe(char *description)
{
   if (__ecore_config_app_description)
      free(__ecore_config_app_description);
   __ecore_config_app_description = strdup(description);
}
