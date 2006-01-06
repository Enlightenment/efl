/* Demonstrates how to use listeners.
 */

#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Config.h>

#define INT_PROPERTY_KEY  "/example/integer"
#define CALLBACK_NAME     "change listener"

int change_listener (const char *key, const Ecore_Config_Type type,
		     const int tag, void *data) {
  printf("Callback called when property '%s' is %li\n", key, ecore_config_int_get(key));
  return 0;
}

int main (int argc, char *argv[]) {
  /* We'll use the same config as config_basic_example.c */
  ecore_config_init("config_basic_example");
  ecore_config_int_default (INT_PROPERTY_KEY, -1);
  ecore_config_listen(CALLBACK_NAME, INT_PROPERTY_KEY, change_listener, 1, NULL);
  ecore_config_load();
  ecore_config_int_set(INT_PROPERTY_KEY, ecore_config_int_get(INT_PROPERTY_KEY) + 1);
  ecore_config_shutdown();
  return 0;
}
