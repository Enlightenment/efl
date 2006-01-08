/* Shows how to handle the Enlightened Properties Library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Ecore_Config.h>

#define INT_VAL_KEY "/example/integer"
#define FLT_VAL_KEY "/example/float"
#define STR_VAL_KEY "/example/string"
#define RGB_VAL_KEY "/example/colour"
#define THM_VAL_KEY "/example/theme"

long   int_val;
float  flt_val;
int    a_val;
int    r_val;
int    g_val;
int    b_val;
char  *str_val;
char  *thm_val;

void set_defaults (void) {
  ecore_config_int_default(INT_VAL_KEY, 0);
  ecore_config_float_default(FLT_VAL_KEY, 0.0);
  ecore_config_string_default(STR_VAL_KEY, "test1");
  ecore_config_argb_default(RGB_VAL_KEY, 255, 0, 0, 0);
  ecore_config_theme_default(THM_VAL_KEY, "default");
}

void get_settings (void) {
  /* Note the order here.  Defaults are to be set before load, so that the
   * types are definitely right.
   */
  set_defaults();
  ecore_config_load();
  int_val = ecore_config_int_get(INT_VAL_KEY);
  flt_val = ecore_config_float_get(FLT_VAL_KEY);
  str_val = ecore_config_string_get(STR_VAL_KEY);
  ecore_config_argb_get(RGB_VAL_KEY, &a_val, &r_val, &g_val, &b_val);
  thm_val = ecore_config_theme_get(THM_VAL_KEY);
}

void change_settings(void) {
  int_val += 1;
  flt_val += 0.1;
  if('9' == str_val[4]) {
    str_val[4] = '0';
  } else {
    str_val[4] += 1;
  }

  a_val = (a_val + 1) % 256;
  r_val = (r_val + 1) % 256;
  g_val = (g_val + 1) % 256;
  b_val = (b_val + 1) % 256;

  if(!strcmp(thm_val, "default")) {
    if(thm_val) free(thm_val);
    thm_val = strdup("winter");
  } else {
    if(thm_val) free(thm_val);
    thm_val = strdup("default");
  }
}

void save_settings (void) {
  ecore_config_int_set(INT_VAL_KEY, int_val);
  ecore_config_float_set(FLT_VAL_KEY, flt_val);
  ecore_config_string_set(STR_VAL_KEY, str_val);
  ecore_config_argb_set(RGB_VAL_KEY, a_val, r_val, g_val, b_val);
  ecore_config_theme_set(THM_VAL_KEY, thm_val);
  ecore_config_save();
}

void dump_settings (void) {
  printf("  Int Value:    %li\n", int_val);
  printf("  Float Value:  %f\n",  flt_val);
  printf("  String Value: %s\n",  str_val);
  printf("  ARGB Value:   %i %i %i %i\n",  a_val, r_val, g_val, b_val);
  printf("  Theme Value:  %s\n",  thm_val);
}

int main (int argc, char **argv) {
  ecore_config_init("config_basic_example");
  get_settings();
  printf("--- Original Values ---\n");
  dump_settings();
  change_settings();
  printf("--- Values to be Saved ---\n");
  dump_settings();
  save_settings();
  if(str_val) free(str_val);
  if(thm_val) free(thm_val);
  ecore_config_shutdown();
  return 0;
}
