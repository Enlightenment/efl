#include "Ecore_Config.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* this should only be built if evas is present */

int
ecore_config_evas_font_path_apply (Evas * evas)
{
  char *font_path, *font_path_tmp, *ptr, *end;

  font_path = ecore_config_get_string ("/e/font/path");

  if (!font_path)
    return ECORE_CONFIG_ERR_NODATA;
  ptr = font_path;
  end = font_path + strlen (font_path);
  font_path_tmp = font_path;
  while (ptr && ptr < end)
    {
      while (*ptr != '|' && ptr < end)
        ptr++;
      if (ptr < end)
        *ptr = '\0';

      evas_font_path_append (evas, font_path_tmp);
      ptr++;
      font_path_tmp = ptr;
    }

  free (font_path);

  return ECORE_CONFIG_ERR_SUCC;
}

static char *_ecore_config_short_types[]={ "nil", "int", "flt", "str", "rgb", "str"};

void
ecore_config_args_display(void)
{
  Ecore_Config_Prop    *props;
  
  printf("Application information to be added here!\n\n"); //FIXME
  printf("Supported Options:\n");
  printf(" -h, --help\t\t       Print this text\n");
  if (!__ecore_config_bundle_local) return;
  props = __ecore_config_bundle_local->data;
  while (props)
    {
      if (props->flags&PF_SYSTEM) {
        props = props->next;
        continue;
      }
      printf(" %c%c%c --%s\t <%s> %s\n", props->short_opt?'-':' ',
             props->short_opt?props->short_opt:' ', props->short_opt?',':' ',
             props->long_opt?props->long_opt:props->key,
             _ecore_config_short_types[props->type], props->description);
      
      props = props->next;
    }
}

void
ecore_config_parse_set(Ecore_Config_Prop *prop, char *arg) {
  if (!arg)
    printf("Missing expected attribute for option --%s\n", prop->key);
  else
    ecore_config_set(prop->key, arg);
}

int
ecore_config_args_parse(int argc, char **argv)
{
  int nextarg, next_short_opt, found;
  char *arg;
  char *long_opt, short_opt;
  Ecore_Config_Prop *prop;
  
  nextarg = 1;
  while (nextarg < argc) {
    arg = argv[nextarg];

    if (*arg != '-') {
      printf("Unexpected attribute \"%s\"\n", arg);
      nextarg++;
      continue;
    }
    
    next_short_opt = 1;
    short_opt = *(arg + next_short_opt);

    if (short_opt == '-') {
      long_opt = arg + 2;
        
      if (!strcmp(long_opt, "help")) {
        ecore_config_args_display();
        return ECORE_CONFIG_PARSE_HELP;
      }

      found = 0;
      prop = __ecore_config_bundle_local->data;
      while (prop) {
        if ((prop->long_opt && !strcmp(long_opt, prop->long_opt))
             || !strcmp(long_opt, prop->key)) {
          found = 1;
          ecore_config_parse_set(prop, argv[++nextarg]);
          break;
        }
        prop = prop->next;
      }
      if (!found) {
        printf("Unrecognised option \"%s\"\n\n", long_opt);
        ecore_config_args_display();
        return ECORE_CONFIG_PARSE_EXIT;
      }
    } else {
      while (short_opt) {
        if (short_opt == 'h') {
          ecore_config_args_display();
          return ECORE_CONFIG_PARSE_HELP;
        } else {
          found = 0;
          prop = __ecore_config_bundle_local->data;
          while (prop) {
            if (short_opt == prop->short_opt) {
              found = 1;
              ecore_config_parse_set(prop, argv[++nextarg]);
              break;
            }
            prop = prop->next;
          }
                  
          if (!found) {
            printf("Unrecognised option '%c'\n\n", short_opt);
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

