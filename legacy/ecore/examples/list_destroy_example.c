#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Ecore_Data.h>

void alloc_data (Ecore_List *list) {
  ecore_list_append(list, strdup("first"));
  ecore_list_append(list, strdup("second"));
  ecore_list_append(list, strdup("third"));
}

void destroy_cb (void *data) {
  printf("Destroying \"%s\".\n", (char *)data);
  free(data);
}

int main (int argc, char *argv[]) {
  Ecore_List *list;

  list = ecore_list_new();
  alloc_data(list);
  ecore_list_set_free_cb(list, destroy_cb);
  ecore_list_destroy(list);

  return 0;
}
